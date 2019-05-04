#include"utility/p_allocator.h"
#include<iostream>
#include <string>
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

PAllocator::PAllocator() 
{
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    string freeListPath         = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    ifstream allocatorCatalog(allocatorCatalogPath, ios::in|ios::binary);
    ifstream freeListFile(freeListPath, ios::in|ios::binary);
    // judge if the catalog exists
    // cout << allocatorCatalog.is_open() << " " << freeListFile.is_open() << "   here is open test\n";
    if (allocatorCatalog.is_open() && freeListFile.is_open()) 
    {

        //cout << "have input \n";
        //freeList.clear();
        /*while(!allocatorCatalog.eof())
        {
            char * pmemaddr;
            size_t mapped_len;
            int is_pmem;

            pmemaddr = (char*)pmem_map_file(allocatorCatalogPath.c_str(), LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);

            uint64_t *t = (uint64_t*) pmemaddr;
            maxFileId  = *t;
            t += 1;
            freeNum = *t;
            t += 1;
            PPointer* t1 = (PPointer *)t;
            startLeaf = *t1;
*/
        allocatorCatalog.read((char *)&maxFileId,sizeof(uint64_t));
        allocatorCatalog.read((char *)&freeNum,sizeof(uint64_t));
        allocatorCatalog.read((char *)&startLeaf,sizeof(uint64_t));
        // cout << "maxFILeid : "<< maxFileId << endl;
        // cout << "freeNum :" << freeNum << endl;
           // cout << "startLeaf :" << startLeaf << endl;
        // }
        allocatorCatalog.close();
        /*while(!freeListFile.eof())
        {

            char * pmemaddr;
            size_t mapped_len;
            int is_pmem;
            pmemaddr = (char*)pmem_map_file(freeListPath.c_str(), LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);

            uint64_t *t = (uint64_t*)pmemaddr;
            
            PPointer t1;
            t1.fileId = *(t++);
            t1.offset = *t;
            freeList.push_back(t1);
*/

        PPointer t;
        for(int i = 0;i < freeNum;i ++){
            freeListFile.read((char *)&t.fileId,sizeof(uint64_t));
            freeListFile.read((char *)&t.offset,sizeof(uint64_t));
            freeList.push_back(t);
            //cout << t.fileId << endl;
            //cout << t.offset << endl;
        }
            
            //fId2PmAddr.insert(pair<uint64_t,char*>(t.fileId,pmem_map(t.data)));
       // }
        freeListFile.close();
    } 
    else 
    {
        //Needing to be added
        maxFileId = 1;
        //cout << "not input disk\n";
        freeNum = 0;
        //startLeaf = NULL;
        
	
	/*this->newLeafGroup();
        getLeaf(treeStartLeaf,fId2PmAddr[maxFileId - 1]);
        */
        ofstream allocatorCatalogOut(allocatorCatalogPath, ios::out|ios::binary);
        if(allocatorCatalogOut.is_open())
        {
            allocatorCatalogOut.close();
        }

        char * pmemaddr;
        size_t mapped_len;
        int is_pmem;

        pmemaddr = (char*)pmem_map_file(allocatorCatalogPath.c_str(), sizeof(uint64_t) * 2 + sizeof(PPointer), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);

        uint64_t * t = (uint64_t*) pmemaddr;
        *t = maxFileId;
        //cout << "maxFileId : " <<  *t << endl;
        t += 1;
        *t = freeNum;
        if (is_pmem){
            //cout << "is_pmem\n";
            pmem_persist(pmemaddr, mapped_len);
        }  
        else{
            //cout << "not pmem\n";
            pmem_msync(pmemaddr, mapped_len);
        }
        ofstream freeListFileOut(freeListPath,ios::out|ios::binary);
        if(freeListFileOut.is_open())
        {
            freeListFileOut.close();
        }
        // pmemaddr = (char*)pmem_map_file(freeListPath.c_str(), sizeof(PPointer)*freeNum, PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);
        // t = (uint64_t*) pmemaddr;
        // *t = maxFileId;
        // cout << "maxFileId : " <<  *t << endl;
        // t += 1;
        // *t = freeNum;
        // if (is_pmem){
        //     // cout << "is_pmem\n";
        //     pmem_persist(pmemaddr, mapped_len);
        // }   
        // else{
        //     pmem_msync(pmemaddr, mapped_len);
        // }
    }
    this->initFilePmemAddr();
    //cout << "freenum : " << freeNum << endl;
}

PAllocator::~PAllocator() 
{
    
    char * pmemaddr;
    size_t mapped_len;
    int is_pmem;

    for(uint64_t i = 1;i < maxFileId;i++)
    {
        string path = DATA_DIR  +to_string(i);
        pmemaddr = (char*)pmem_map_file(path.c_str(), LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);
        if (is_pmem){
            // cout << "is_pmem\n";
            pmem_persist(pmemaddr, mapped_len);
        }
            
        else{
            pmem_msync(pmemaddr, mapped_len);
        }
    }
    //cout << "here4\n";
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    pmemaddr = (char*)pmem_map_file(allocatorCatalogPath.c_str(), sizeof(uint64_t) * 2 + sizeof(PPointer), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);
    uint64_t * t2 = (uint64_t*) pmemaddr;
    *t2 = maxFileId;
     t2 += 1;
    *t2 = freeNum;
    if (is_pmem){
        //cout << "is_pmem\n";
        pmem_persist(pmemaddr, mapped_len);
    }   
    else{
        //cout << "not pmem\n";
        pmem_msync(pmemaddr, mapped_len);
    }
    //cout << "here5\n";
    string freeListPath = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    //cout << "freeNum:" << freeNum << endl;
    
    char * pmemaddr1;
    size_t mapped_len1;
    int is_pmem1;
    if(freeNum > 0){
        pmemaddr1 = (char*)pmem_map_file(freeListPath.c_str(), sizeof(PPointer)*100, PMEM_FILE_CREATE,0666, &mapped_len1, &is_pmem1);
        uint64_t *t = (uint64_t *) pmemaddr1;
        for(int i = 0;i < freeList.size();i++)
        {
            *(t++) = freeList[i].fileId;
            *(t++) = freeList[i].offset;
        }
        //cout << "here6\n";
        if (is_pmem1){
            //cout << "is_pmem\n";
            pmem_persist(pmemaddr1, mapped_len1);
        }   
        else{
            pmem_msync(pmemaddr1, mapped_len1);
        }
    }
    //cout << "here6\n";
    freeNum = 0;
    maxFileId = 1;
    freeList.clear();
    fId2PmAddr.clear();
    //cout << "cut \n";
    //if( PAllocator::pAllocator )
    PAllocator::pAllocator = NULL;
}

// memory map all leaves to pmem address, storing them in the fId2PmAddr
void PAllocator::initFilePmemAddr() 
{
    char * pmemaddr;
    size_t mapped_len;
    int is_pmem;

    for(uint64_t i = 1;i < maxFileId;i++)
    {
        string path = DATA_DIR + to_string(i);
        pmemaddr = (char*)pmem_map_file(path.c_str(), LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);
        fId2PmAddr.insert(pair<uint64_t,char*>(i,pmemaddr));
    }
}

// get the pmem address of the target PPointer from the map fId2PmAddr
char* PAllocator::getLeafPmemAddr(PPointer p) 
{
    return fId2PmAddr[p.fileId] + p.offset;
}

// get and use a leaf for the fptree leaf allocation
// return 
bool PAllocator::getLeaf(PPointer &p, char* &pmem_addr) 
{   
    //cout << "maxFileId : " << maxFileId << " size = : " <<  freeList.size() << endl;
    if(freeList.size() == 0)
    {
        //cout << "size = 0\n";
        this->newLeafGroup();

    //cout << "maxFileId : " << maxFileId << " size = : " <<  freeList.size() << endl;
    }

    //cout << freeList[0].fileId << endl;
    p.fileId = freeList[freeList.size() - 1].fileId;
    p.offset = freeList[freeList.size() - 1].offset;
    //pmem_addr = fId2PmAddr[p.fileId];
    //cout << p.fileId << endl;
    // cout << "freeNum : " << freeNum << endl;
    // cout << "p offset : " << p.offset << endl;
    size_t mapped_len;
    int is_pmem;
    string path = DATA_DIR +to_string(p.fileId);
    pmem_addr = (char *)pmem_map_file(path.c_str(), LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);

    //cout << "freeNUm : " << freeNum << endl;
    freeNum -= 1;
    freeList.pop_back();
    uint64_t *t = (uint64_t*)pmem_addr;
    *t += 1;
    t += 1;

    Byte * t1 = (Byte*)t;
    int length = (p.offset - LEAF_GROUP_HEAD) / calLeafSize();
    //cout<< length << endl;
    
    t1 += length;
    *t1 = 1;
    pmem_msync(pmem_addr, mapped_len);
   // cout << mapped_len << endl;
	if (is_pmem){
//        cout << "is_pmem\n";
        pmem_persist(pmem_addr, mapped_len);
    }
		
	else{
        pmem_msync(pmem_addr, mapped_len);
    }
		

    return true;
    //return false;
}

bool PAllocator::ifLeafUsed(PPointer p) 
{
    if(ifLeafExist(p))
    {
        uint64_t *t = (uint64_t*)fId2PmAddr[p.fileId];
        t += 1;
        Byte *t1 = (Byte *)t;
        int length = (p.offset - LEAF_GROUP_HEAD) / calLeafSize();
        if(*(t1 + length) == 1) return true;
    }
    return false;
}

bool PAllocator::ifLeafFree(PPointer p) 
{
    return !ifLeafUsed(p);
}

bool PAllocator::ifLeafExist(PPointer p) 
{
    if(p.fileId < 1 && p.fileId >= maxFileId)
        return false;
    uint64_t length = sizeof(uint64_t) + sizeof(Byte) * LEAF_DEGREE + calLeafSize() * LEAF_GROUP_AMOUNT;
    if(p.offset >= length)  return false;
    return true;
}

// free and reuse a leaf
bool PAllocator::freeLeaf(PPointer p) 
{
    if(ifLeafExist(p))
    {
        string freeListPath         = DATA_DIR + P_ALLOCATOR_FREE_LIST;
        char * pmemaddr1;
        size_t mapped_len1;
        int is_pmem1;
        //cout << "here4\n";
        pmemaddr1 = (char*)pmem_map_file(freeListPath.c_str(), sizeof(PPointer)*freeNum, PMEM_FILE_CREATE,0666, &mapped_len1, &is_pmem1);
       //cout << "here5\n";
        uint64_t *t = (uint64_t*)pmemaddr1;
        *t -= 1;
        t += 1;
        
        Byte *t1 = (Byte *)t;
        int length = (p.offset - LEAF_GROUP_HEAD) / calLeafSize();
        *(t1 + length) = 0;
        freeNum += 1;
        //cout << "here6\n";
        //leaf
        Byte *t2 = (Byte *)getLeafPmemAddr(p);
        //cout << "here7\n";
        // for(int i = 0;i < LEAF_DEGREE;i++)
        //     *(t2++) = 0;
        freeList.push_back(p);
        //cout << "here7\n";
        return true;
    }
    else    return true;
}

bool PAllocator::persistCatalog() 
{
    //have question
    string allocatorCatalogPath = DATA_DIR +P_ALLOCATOR_CATALOG_NAME;
    ifstream allocatorCatalog(allocatorCatalogPath, ios::in|ios::binary);
   // int fd = open(allocatorCatalogPath);
    char * pmemaddr;
    size_t mapped_len;
    int is_pmem;

    pmemaddr = (char*)pmem_map_file(allocatorCatalogPath.c_str(), sizeof(uint64_t) * 2 + sizeof(PPointer), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);

    return true;
    //return false;
}

/*
  Leaf group structure: (uncompressed)
  | usedNum(8b) | bitmap(n * byte) | leaf1 |...| leafn |
*/
// create a new leafgroup, one file per leafgroup
bool PAllocator::newLeafGroup() 
{
    char * pmem_addr;
    size_t mapped_len;
    int is_pmem;


    string path = DATA_DIR + to_string(maxFileId);
    pmem_addr = (char*)pmem_map_file(path.c_str(), LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);


    uint64_t *t = (uint64_t*)pmem_addr;
    *t = 0;
    t += 1;
    Byte* t1 = (Byte*) t;
    for(int i = 0;i < LEAF_GROUP_AMOUNT;i++)
        *(t1++) = 0;
    PPointer t3;
    t3.fileId = maxFileId;
    
    for(int i = 0;i < LEAF_GROUP_AMOUNT;i++)
    {
        t3.offset = LEAF_GROUP_HEAD + calLeafSize() * i;
        freeList.push_back(t3);
        freeNum++;
    }

    //fId2PmAddr.insert(pair<uint64_t,char*>(maxFileId,pmem_addr));

    maxFileId += 1;

    return true;
    //return false;
}
