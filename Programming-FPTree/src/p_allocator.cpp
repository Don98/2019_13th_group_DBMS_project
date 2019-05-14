#include"utility/p_allocator.h"
#include<iostream>
#include<cstring>
using namespace std;

// the file that store the information of allocator
const string P_ALLOCATOR_CATALOG_NAME = "p_allocator_catalog";
// a list storing the free leaves
const string P_ALLOCATOR_FREE_LIST    = "free_list";

PAllocator* PAllocator::pAllocator = new PAllocator();

PAllocator* PAllocator::getAllocator() {
    if (PAllocator::pAllocator == NULL) {
        PAllocator::pAllocator = new PAllocator();
    }
    return PAllocator::pAllocator;
}

/* data storing structure of allocator
   In the catalog file, the data structure is listed below
   | maxFileId(8 bytes) | freeNum = m | treeStartLeaf(the PPointer) |
   In freeList file:
   | freeList{(fId, offset)1,...(fId, offset)m} |
*/
PAllocator::PAllocator() {
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    string freeListPath         = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    ifstream allocatorCatalog(allocatorCatalogPath, ios::in|ios::binary);
    ifstream freeListFile(freeListPath, ios::in|ios::binary);
    // judge if the catalog exists
    if (allocatorCatalog.is_open() && freeListFile.is_open()) {
        allocatorCatalog.read((char*)&maxFileId, sizeof(uint64_t));
        allocatorCatalog.read((char*)&freeNum, sizeof(uint64_t));
        allocatorCatalog.read((char*)&startLeaf.fileId, sizeof(PPointer));
        allocatorCatalog.close();

        PPointer pt;
        for(uint64_t i = 0; i < freeNum; i++){
            freeListFile.read((char*)&pt, sizeof(PPointer));
            freeList.push_back(pt);
        }
        freeListFile.close();
    } else {
        // not exist, create catalog and free_list file, then open.
        maxFileId = 1;
        freeNum = 0;
        startLeaf.fileId = ILLEGAL_FILE_ID;
        startLeaf.offset = 0;
        
        ofstream freeListFileOut(freeListPath, ios::out|ios::binary);
        if (freeListFileOut.is_open()) {
            freeListFileOut.close();
        }

        persistCatalog();
    }
    this->initFilePmemAddr();
}

PAllocator::~PAllocator() {
    persistCatalog();
    for (uint64_t i = 1; i < maxFileId; ++ i) {
        if (pmem_is_pmem(fId2PmAddr[i], LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize())) {
            pmem_persist(fId2PmAddr[i], LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize());
        } else {
            pmem_msync(fId2PmAddr[i], LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize());
        }
        pmem_unmap(fId2PmAddr[i], LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize());
    }
    pAllocator = NULL;
}

// memory map all leaves to pmem address, storing them in the fId2PmAddr
void PAllocator::initFilePmemAddr() {
    char* pmem_addr;
    size_t mapped_len;
    int is_pmem;

    for(uint64_t i = 1; i < maxFileId; i++) {
        string path = DATA_DIR + to_string(i);
        pmem_addr = (char*)pmem_map_file(path.c_str(),
                                LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(),
                                PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem);
        if (pmem_addr == NULL) {
            printf("error: initFilePmemAddr fail\n");
            exit(1);
        }
        fId2PmAddr.insert(pair<uint64_t,char*>(i, pmem_addr));
    }
}

// get the pmem address of the target PPointer from the map fId2PmAddr
char* PAllocator::getLeafPmemAddr(PPointer p) {
    if (ifLeafExist(p)) {
        return fId2PmAddr[p.fileId] + p.offset;
    }
    return NULL;
}

// get and use a leaf for the fptree leaf allocation
// return 
bool PAllocator::getLeaf(PPointer &p, char* &pmem_addr) {
    if (freeNum == 0 && !newLeafGroup()) return false;

    p.fileId = freeList.back().fileId;
    p.offset = freeList.back().offset;
    pmem_addr = getLeafPmemAddr(p);

    if (pmem_addr == NULL) {
        printf("error: in getLeaf -> getLeafPmemAddr\n");
        return false;
    }

    if (!ifLeafExist(this->startLeaf)) {
        this->startLeaf = p;
    }

    freeList.pop_back();
    freeNum--;
    memset(pmem_addr, 0, calLeafSize());
    if (pmem_is_pmem(pmem_addr, calLeafSize())) {
        pmem_persist(pmem_addr, calLeafSize());
    } else {
        pmem_msync(pmem_addr, calLeafSize());
    }

    char* leaf_group_pt = fId2PmAddr[p.fileId];
    uint64_t* pt = (uint64_t*) leaf_group_pt;
    (*pt)++;
    Byte* bitmap_pt = (Byte*) (pt + 1);
    bitmap_pt[(p.offset-LEAF_GROUP_HEAD)/calLeafSize()] = 1;
    if (pmem_is_pmem(leaf_group_pt, LEAF_GROUP_HEAD)) {
        pmem_persist(leaf_group_pt, LEAF_GROUP_HEAD);
    } else {
        pmem_msync(leaf_group_pt, LEAF_GROUP_HEAD);
    }
    
    return true;
}

bool PAllocator::ifLeafUsed(PPointer p) {
    if (ifLeafExist(p) && !ifLeafFree(p)) return true;
    return false;
}

bool PAllocator::ifLeafFree(PPointer p) {
    if (ifLeafExist(p)) {
        return !*(((Byte*)fId2PmAddr[p.fileId]) + sizeof(uint64_t) +
                (p.offset - LEAF_GROUP_HEAD) / calLeafSize());
    }
    return false;
}

// judge whether the leaf with specific PPointer exists. 
bool PAllocator::ifLeafExist(PPointer p) {
    if (p.fileId == ILLEGAL_FILE_ID || p.fileId >= maxFileId) return false;
    if (p.offset >= LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize()) return false;
    if ((p.offset - LEAF_GROUP_HEAD) % calLeafSize() != 0) return false;
    return true;
}

// free and reuse a leaf
bool PAllocator::freeLeaf(PPointer p) {
    if (!ifLeafUsed(p)) return false;

    ofstream freeListFile((DATA_DIR + P_ALLOCATOR_FREE_LIST).c_str(), ios::out|ios::binary);
    if (!freeListFile.is_open()) {
        printf("error in freeLeaf\n");
    }
    freeListFile.seekp(freeNum * sizeof(PPointer), ios::beg);
    freeListFile.write((char*) &p, sizeof(PPointer));
    freeListFile.close();

    freeList.push_back(p);
    freeNum++;

    char* leaf_group_pt = fId2PmAddr[p.fileId];
    uint64_t* pt = (uint64_t*) leaf_group_pt;
    (*pt)--;
    Byte* bitmap_pt = (Byte*) (pt + 1);
    bitmap_pt[(p.offset-LEAF_GROUP_HEAD)/calLeafSize()] = 0;

    if (pmem_is_pmem(leaf_group_pt, LEAF_GROUP_HEAD)) {
        pmem_persist(leaf_group_pt, LEAF_GROUP_HEAD);
    } else {
        pmem_msync(leaf_group_pt, LEAF_GROUP_HEAD);
    }
    return true;
}

bool PAllocator::persistCatalog() {
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    ofstream allocatorCatalog(allocatorCatalogPath, ios::out|ios::binary);
    if (allocatorCatalog.is_open()) {
        allocatorCatalog.write((char*)&maxFileId, sizeof(uint64_t));
        allocatorCatalog.write((char*)&freeNum, sizeof(uint64_t));
        allocatorCatalog.write((char*)&startLeaf, sizeof(PPointer));
        allocatorCatalog.close();
        return true;
    }
    return false;
}

/*
  Leaf group structure: (uncompressed)
  | usedNum(8b) | bitmap(n * byte) | leaf1 |...| leafn |
*/
// create a new leafgroup, one file per leafgroup
bool PAllocator::newLeafGroup() {
    char* pmem_addr;
    size_t mapped_len;
    int is_pmem;

    string path = DATA_DIR + to_string(maxFileId);

    pmem_addr = (char*)pmem_map_file(path.c_str(),
                            LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(),
                            PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem);
    if (pmem_addr == NULL) {
        printf("error: newLeafGroup fail\n");
        return false;
    }

    fId2PmAddr[maxFileId] = pmem_addr;
    memset(pmem_addr, 0, mapped_len);

    ofstream freeListFileOut((DATA_DIR + P_ALLOCATOR_FREE_LIST).c_str(), ios::out|ios::binary);
    if (!freeListFileOut.is_open()) {
        printf("error: in newLeafGroup -> open free list file\n");
        return false;
    }
    freeListFileOut.seekp(freeNum * sizeof(PPointer), ios::beg);

    PPointer ppt;
    ppt.fileId = maxFileId;
    for (int i = 0; i < LEAF_GROUP_AMOUNT; ++ i) {
        ppt.offset = LEAF_GROUP_HEAD + i * calLeafSize();
        freeList.push_back(ppt);
        freeListFileOut.write((char*) &ppt, sizeof(PPointer));
        freeNum++;
    }

    freeListFileOut.close();

    maxFileId++;

    if (is_pmem) {
        pmem_persist(pmem_addr, mapped_len);
    } else {
        pmem_msync(pmem_addr, mapped_len);
    }

    return true;
}
