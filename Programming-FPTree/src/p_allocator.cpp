#include"utility/p_allocator.h"
#include<iostream>
#include<cstring>
using namespace std;

// the file that store the information of allocator
const string P_ALLOCATOR_CATALOG_NAME = "p_allocator_catalog";
// a list storing the free leaves
const string P_ALLOCATOR_FREE_LIST    = "free_list";

// Singleton
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

        // after create the catalog and free_list file, they need to
        // store, freeList now is empty, so create an empty file
        ofstream freeListFileOut(freeListPath, ios::out|ios::binary);
        if (freeListFileOut.is_open()) {
            freeListFileOut.close();
        }
        // this function is used for store the catalog file
        persistCatalog();
    }
    this->initFilePmemAddr();
}

/******************************************************************************
when destruct, store the data, and then unmap the pmem address, at last set
the pointer pAllocator to NULL

TODO
derectly set the pAllocator to NULL without delete may cause MEMERY LEAK,
consider user samrt pointer or other method to fixed
******************************************************************************/
PAllocator::~PAllocator() {
    persistCatalog();
    for (uint64_t i = 1; i < maxFileId; ++ i) {
        if (ifPmemAddr(i)) {
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
        fId2IsPmAddr.insert(pair<uint64_t,int>(i, is_pmem));
    }
}

// get the pmem address of the target PPointer from the map fId2PmAddr
char* PAllocator::getLeafPmemAddr(PPointer p) {
    if (ifLeafExist(p)) {
        return fId2PmAddr[p.fileId] + p.offset;
    }
    return NULL;
}

bool PAllocator::ifPmemAddr(PPointer p) {
    if (fId2IsPmAddr.count(p.fileId) == 0) return false;
    return fId2IsPmAddr[p.fileId];
}

bool PAllocator::ifPmemAddr(uint64_t fileId) {
    if (fId2IsPmAddr.count(fileId) == 0) return false;
    return fId2IsPmAddr[fileId];
}

// get and use a leaf for the fptree leaf allocation
// return true if successs, false otherwise
// the freelist file needn't to be update for we always allocate the last leaf
// in free list, and we only search the first freeNum leaf in freelist file for
// reload the freelist when initialize, we just update freeNum
bool PAllocator::getLeaf(PPointer &p, char* &pmem_addr) {
    // if freenum is 0, it means that there is no free leaf, call function
    // newLeafGroup to create a new leaf group for supply free leaves,
    // if newLeafGroup return false, then create new leaf group failed
    // and there is no usable free leaf, retrun false
    if (freeNum == 0 && !newLeafGroup()) return false;

    // get the last leaf in free list
    p.fileId = freeList.back().fileId;
    p.offset = freeList.back().offset;
    pmem_addr = getLeafPmemAddr(p);

    if (pmem_addr == NULL) {
        printf("error: in getLeaf -> getLeafPmemAddr\n");
        return false;
    }

    // if start leaf is not used, the leaf create now is the start leaf
    if (!ifLeafUsed(this->startLeaf)) {
        this->startLeaf = p;
    }

    freeList.pop_back();
    freeNum--;

    // initial the leaf as 0, maybe can remove this initial step
    memset(pmem_addr, 0, calLeafSize());
    if (ifPmemAddr(p)) {
        pmem_persist(pmem_addr, calLeafSize());
    } else {
        pmem_msync(pmem_addr, calLeafSize());
    }

    // set the corresponding leaf group header,
    // usedNum + 1, and set the corresponding bitmap byte to 1
    // then persist the leaf group header
    char* leaf_group_pt = fId2PmAddr[p.fileId];
    uint64_t* pt = (uint64_t*) leaf_group_pt;
    (*pt)++;
    Byte* bitmap_pt = (Byte*) (pt + 1);
    bitmap_pt[(p.offset-LEAF_GROUP_HEAD)/calLeafSize()] = 1;
    if (ifPmemAddr(p.fileId)) {
        pmem_persist(leaf_group_pt, LEAF_GROUP_HEAD);
    } else {
        pmem_msync(leaf_group_pt, LEAF_GROUP_HEAD);
    }

    return true;
}

// leaf used means the leaf exist and is not free
bool PAllocator::ifLeafUsed(PPointer p) {
    if (ifLeafExist(p) && !ifLeafFree(p)) return true;
    return false;
}

// leaf free means the leaf exist and the corresponding leaf group
// bitmap byte is 0
bool PAllocator::ifLeafFree(PPointer p) {
    if (ifLeafExist(p)) {
        return !*(((Byte*)fId2PmAddr[p.fileId]) + sizeof(uint64_t) +
                (p.offset - LEAF_GROUP_HEAD) / calLeafSize());
    }
    return false;
}

// judge whether the leaf with specific PPointer exists.
// there is 3 illegal case
// case 1: fileid out of bound
// case 2: offset out of bound
// case 3: offset not point to the start of a leaf
bool PAllocator::ifLeafExist(PPointer p) {
    if (p.fileId == ILLEGAL_FILE_ID || p.fileId >= maxFileId) return false;
    if (p.offset >= LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize()) return false;
    if ((p.offset - LEAF_GROUP_HEAD) % calLeafSize() != 0) return false;
    return true;
}

// free and reuse a leaf
bool PAllocator::freeLeaf(PPointer p) {
    // if leaf not used, can't free
    if (!ifLeafUsed(p)) return false;

    // if the ppointer is start leaf, set start leaf to it's pnext,
    // and persist catalog
    if (startLeaf == p) {
        startLeaf = *((PPointer*)(getLeafPmemAddr(p) + (LEAF_DEGREE * 2 + 7) / 8));
        persistCatalog();
    }

    // open the freeList file, and update it
    ofstream freeListFile((DATA_DIR + P_ALLOCATOR_FREE_LIST).c_str(), ios::out|ios::binary);
    if (!freeListFile.is_open()) {
        printf("error in freeLeaf\n");
    }
    freeListFile.seekp(freeNum * sizeof(PPointer), ios::beg);
    freeListFile.write((char*) &p, sizeof(PPointer));
    freeListFile.close();

    // add the free leaf to free list
    freeList.push_back(p);
    freeNum++;

    // update leaf group header and persist
    char* leaf_group_pt = fId2PmAddr[p.fileId];
    uint64_t* pt = (uint64_t*) leaf_group_pt;
    (*pt)--;
    Byte* bitmap_pt = (Byte*) (pt + 1);
    bitmap_pt[(p.offset-LEAF_GROUP_HEAD)/calLeafSize()] = 0;

    if (ifPmemAddr(p.fileId)) {
        pmem_persist(leaf_group_pt, LEAF_GROUP_HEAD);
    } else {
        pmem_msync(leaf_group_pt, LEAF_GROUP_HEAD);
    }
    return true;
}

// persist the catalog file
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

    // get pmem map addr
    pmem_addr = (char*)pmem_map_file(path.c_str(),
                            LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(),
                            PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem);
    if (pmem_addr == NULL) {
        printf("error: newLeafGroup fail\n");
        return false;
    }

    // add the new fileid and pmem addr to fId2PmAddr
    fId2PmAddr[maxFileId] = pmem_addr;
    fId2IsPmAddr[maxFileId] = is_pmem;
    memset(pmem_addr, 0, mapped_len);

    // add the new free leaves to free list and store them in free list file
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

    // update max file id and persist the leaf group
    maxFileId++;

    if (is_pmem) {
        pmem_persist(pmem_addr, mapped_len);
    } else {
        pmem_msync(pmem_addr, mapped_len);
    }

    return true;
}
