#include "fptree/fptree.h"

using namespace std;

/*
    friend class FPTree;

    bool   isRoot;     // judge whether the node is root
    int    nKeys;      // amount of keys
    int    nChild;     // amount of children
    Key*   keys;       // max (2 * d + 1) keys
    Node** childrens;  // max (2 * d + 2) node pointers
*/

/*
    FPTree* tree;     // the tree that the node belongs to
    int     degree;   // the degree of the node
    bool    isLeaf;   // judge whether the node is leaf
    bool    isEmpty;  // judge whether the node has entry     //thanos
*/

// Initial the new InnerNode
InnerNode::InnerNode(const int& d, FPTree* const& t, bool _isRoot) { //thanos
    this->tree = t;
    this->isRoot = _isRoot;
    // if(_isRoot) t->changeRoot(this);
    this->nKeys = 0;
    this->nChild = 0;
    this->keys = new Key [2*d + 1]; 
    this->childrens = new Node * [2*d+2];
    this->degree = d;
    this->isLeaf = false;
    int i;
    for(i = 0;i < 2*d+1;i ++) keys[i] = 0;
    for(i = 0;i < 2*d+2;i ++) childrens[i] = NULL;

    //it's 2*d+1 but not 2*d because that TA want one buffer,whose size is 1
}

// delete the InnerNode
InnerNode::~InnerNode() {
    delete this;
}

// binary search the first key in the innernode larger than input key
int InnerNode::findIndex(const Key& k) {   //thanos
    //typedef uint64_t Key
    //
    //binary search the first key in the innernode larger than input key
    int temp_end = this->getKeyNum()-1;
    int start = 0;
    int end = temp_end;
	int mid = (start+end) / 2;
	
	if(keys[0] > k) return 0;
	if(keys[temp_end] <= k) return -1;
	while(true){
		if(keys[mid] <= k){
			start = mid;
			mid =  (start+end) / 2;
		}else {
			end = mid;
			mid =  (start+end) / 2;
		}
		if(mid == start) break;
	}
    return end;
}

// insert the node that is assumed not full
// insert format:
// ======================
// | key | node pointer |
// ======================
// WARNING: can not insert when it has no entry
void InnerNode::insertNonFull(const Key& k, Node* const& node) { //thanos
    if(node == NULL || ((InnerNode*)node)->nKeys == 0) printf("can not insert when it has no entry\n");
    this->keys[nKeys] = k;
    this->childrens[nChild] = node;
    this->nKeys ++;
    this->nChild++;
}

// insert func
// return value is not NULL if split, returning the new child and a key to insert
KeyNode* InnerNode::insert(const Key& k, const Value& v) {   
    KeyNode* newChild = NULL;

    if (this->isRoot && this->nKeys == 0) { 
        this->keys[nKeys++] = k;
        LeafNode * temp_node = new LeafNode(this->tree);
        this->childrens[nChild++] = temp_node;
        LeafNode * temp_node1 = new LeafNode(this->tree);
        this->childrens[nChild++] = temp_node1;
        ((LeafNode*)(this->childrens[0]))->insertNonFull(k,v);
        return newChild;
    }

    InnerNode *tmp = this;
    int pos = tmp->findIndex(k);
    //Node * tmpChild = tmp->childrens[pos];
    while(true)
    {
        if(tmp->ifLeaf())
        {
            KeyNode * newChild1 = tmp->insert(k,v);
            this->keys[nKeys] = newChild1->key;
            this->childrens[nChild++] = newChild1->node;
            this->nKeys += 1;
            if(this->nKeys  == this->degree * 2)
            {
                newChild = this->split();
            }
            break;
        }
        else
        {
            pos = tmp->findIndex(k);
            tmp = (InnerNode *)(tmp->childrens[pos]);
        }

    }
    return newChild;
}

// ensure that the leaves inserted are ordered
// used by the bulkLoading func
// inserted data: | minKey of leaf | LeafNode* |
KeyNode* InnerNode::insertLeaf(const KeyNode& leaf) { 
    KeyNode* newChild = NULL;
    // first and second leaf insertion into the tree
    if (this->isRoot && this->nKeys == 0) {
        // TODO
        if (this->nChild == 0) {
            this->childrens[nChild++] = leaf.node;
        } else {
            this->childrens[nChild++] = leaf.node;
            this->keys[nKeys++] = leaf.key;
        }
        return newChild;
    }
    
    // recursive insert
    // Tip: please judge whether this InnerNode is full
    // next level is not leaf, just insertLeaf
    // TODO

    if (!childrens[nChild-1]->ifLeaf())  {
        newChild = insertLeaf(leaf);
        if (newChild != NULL) {
            this->childrens[nChild++] = newChild.node;
            this->keys[nKeys++] = newChild.key;
            if (nKeys > 2 * degree) {
                newChild = split();
                if (isRoot) {
                    tree->changeRoot(newChild.node);
                    delete newChild;
                    return NULL;
                }
                else return newChild;
            }
            delete newChild;
            newChild = NULL;
        }
    }

    // next level is leaf, insert to childrens array
    // TODO

    else {
        childrens[nKeys++] = leaf.key;
        childrens[nChild++] = leaf.node;
        if (nKeys > 2 * degree) {
            newChild = split();
            if (isRoot) {
                tree->changeRoot(newChild.node);
                delete newChild;
                return NULL;
            }
        }
    }


    return newChild;
}

/*
    friend class FPTree;

    bool   isRoot;     // judge whether the node is root
    int    nKeys;      // amount of keys
    int    nChild;     // amount of children
    Key*   keys;       // max (2 * d + 1) keys
    Node** childrens;  // max (2 * d + 2) node pointers
*/

/*
    FPTree* tree;     // the tree that the node belongs to
    int     degree;   // the degree of the node
    bool    isLeaf;   // judge whether the node is leaf
    bool    isEmpty;  // judge whether the node has entry     //thanos
*/
//InnerNode::InnerNode(const int& d, FPTree* const& t, bool _isRoot)
KeyNode* InnerNode::split() {
    if(this->nKeys < 2*this->degree){
        printf("split innernode ERROR,not full\n");
        return NULL;
    }
    // right half entries of old node to the new node, others to the old node. 
    KeyNode* newChild = new KeyNode();
    InnerNode * newNode = new InnerNode(this->degree,this->tree,false);

    newNode->nKeys = nKeys / 2;
    newNode->nChild = nChild / 2 + 1;
    for(int i = nKeys / 2;i < nKeys;i++)
    {
        newNode->childrens[i - nKeys / 2] = this->childrens[i];
        newNode->keys[i - nKeys / 2] = this->keys[i];
    }
    newNode->childrens[nChild / 2] = this->childrens[nChild - 1];

    newChild->key = newNode->keys[0];
    newChild->node = newNode;
    return newChild;
}

// remove the target entry
// return TRUE if the children node is deleted after removement.
// the InnerNode need to be redistributed or merged after deleting one of its children node.
bool InnerNode::remove(const Key& k, const int& index, InnerNode* const& parent, bool &ifDelete) {
    bool ifRemove = false;
    // only have one leaf
    // TODO
    
    // recursive remove
    // TODO
    return ifRemove;
}

// If the leftBro and rightBro exist, the rightBro is prior to be used
void InnerNode::getBrother(const int& index, InnerNode* const& parent, InnerNode* &leftBro, InnerNode* &rightBro) {
    // TODO
}

// merge this node, its parent and left brother(parent is root)
void InnerNode::mergeParentLeft(InnerNode* const& parent, InnerNode* const& leftBro) {
    // TODO
}

// merge this node, its parent and right brother(parent is root)
void InnerNode::mergeParentRight(InnerNode* const& parent, InnerNode* const& rightBro) {
    // TODO
}

// this node and its left brother redistribute
// the left has more entries
void InnerNode::redistributeLeft(const int& index, InnerNode* const& leftBro, InnerNode* const& parent) {
    // TODO
}

// this node and its right brother redistribute
// the right has more entries
void InnerNode::redistributeRight(const int& index, InnerNode* const& rightBro, InnerNode* const& parent) {
    // TODO
}

// merge all entries to its left bro, delete this node after merging.
void InnerNode::mergeLeft(InnerNode* const& leftBro, const Key& k) {
    // TODO
}

// merge all entries to its right bro, delete this node after merging.
void InnerNode::mergeRight(InnerNode* const& rightBro, const Key& k) {
    // TODO
}

// remove a children from the current node, used by remove func
void InnerNode::removeChild(const int& keyIdx, const int& childIdx) {
    // TODO
}

// update the target entry, return true if the update succeed.
bool InnerNode::update(const Key& k, const Value& v) {
    // TODO
    return false;
}

// find the target value with the search key, return MAX_VALUE if it fails.
Value InnerNode::find(const Key& k) {
    // TODO

    return MAX_VALUE;
}

// get the children node of this InnerNode
Node* InnerNode::getChild(const int& idx) {
    
    return this->childrens[idx];
}

// get the key of this InnerNode
Key InnerNode::getKey(const int& idx) {
    if (idx < this->nKeys) {
        return this->keys[idx];
    } else {
        return MAX_KEY;
    }
}

// print the InnerNode
void InnerNode::printNode() {
    cout << "||#|";
    for (int i = 0; i < this->nKeys; i++) {
        cout << " " << this->keys[i] << " |#|";
    }
    cout << "|" << "    ";
}

// print the LeafNode
void LeafNode::printNode() {
    cout << "||";
    for (int i = 0; i < 2 * this->degree; i++) {
        if (this->getBit(i)) {
            cout << " " << this->kv[i].k << " : " << this->kv[i].v << " |";
        }
    }
    cout << "|" << " ====>> ";
}

/*
    // the NVM relative variables
    char*      pmem_addr;      // the pmem address of the leaf node

    // the pointer below are all pmem address based on pmem_addr
    // need to set the pointer pointed to NVM address
    Byte*      bitmap;         // bitmap of the KV slots
    PPointer*  pNext;          // next leafnode
    Byte*      fingerprints;   // the fingerprint of the keys array
    KeyValue*  kv;             // the keyValue pairs array

    // the DRAM relative variables
    int        n;              // amount of entries
    LeafNode*  prev;           // the address of previous leafnode      
    LeafNode*  next;           // the address of next leafnode  
    PPointer   pPointer;        // the persistent pointer pointed to the leaf in NVM
    string     filePath;        // the file path of the leaf
    
    uint64_t   bitmapSize;      // the bitmap size of the leaf(bytes)
*/

/*  FROM SUPER CLASS -- Node
    FPTree* tree;     // the tree that the node belongs to
    int     degree;   // the degree of the node
    bool    isLeaf;   // judge whether the node is leaf
    bool    isEmpty;  // judge whether the node has entry     //thanos
*/
// new a empty leaf and set the valuable of the LeafNode

// Leaf : | bitmap | pNext | fingerprints array | KV array |
LeafNode::LeafNode(FPTree* t) { //thanos
    this->tree = t;
    this ->degree = LEAF_DEGREE;
    this->isLeaf = true;

    //get pmem_addr and PPointer
    PAllocator* pa = PAllocator::getAllocator();
    PPointer pp;
    char * pmem_addr;
    if( !pa->getLeaf(pp,pmem_addr)) printf("get Leaf error\n");

    pa->~PAllocator();

    this->pmem_addr = pmem_addr;
    this->bitmapSize = (LEAF_DEGREE * 2 +7) / 8;   //it's from fun calLeafSize(),but i don't know why    //iron man
    this->bitmap = new Byte [bitmapSize];
    for(int i = 0;i < bitmapSize;i++) bitmap[i] = 0;
    this->pNext = NULL;
    this->fingerprints = new Byte[this->degree * 2];
    this->kv = new KeyValue [this->degree * 2] ;
    // for(int i = 0;i < this->degree * 2 ;i++) bitmap[i] = 0;

    this->n = 0;
    this->prev = NULL;
    this->next = NULL;

    this->pPointer = pp;
    this->filePath = DATA_DIR + to_string(pp.fileId);
    
    // return this;
}

// reload the leaf with the specific Persistent Pointer
// need to call the PAllocator
LeafNode::LeafNode(PPointer p, FPTree* t) {
    this->tree = t;
    this->degree = LEAF_DEGREE;
    this->isLeaf = true;

    PAllocator* pa = PAllocator::getAllocator();

    // printf("777 %d %d \n",p.fileId,p.offset);

    this->pmem_addr = pa->getLeafPmemAddr(p);
    // cout << "pmem_addr " << *((uint64_t *)pmem_addr) << endl;

    this->bitmapSize = (LEAF_DEGREE * 2 +7) / 8; 
    this->bitmap = (Byte *) this->pmem_addr;

    this->pNext  = (PPointer *)(this->bitmap + bitmapSize);

    this->fingerprints = (Byte *)(this->pNext + 1);

    this->n = 0;
    for(int i = 0;i < bitmapSize;i++)
    {
        this->n += countOneBits(*(this->bitmap + i));
    }
    //this-> n = sizeof(fingerprints)/sizeof(Byte);
    cout <<"n initial "<< this->n << endl;
    
    this->kv = (KeyValue*)(this->fingerprints + 2*LEAF_DEGREE); 

    this->prev = NULL;
    this->next = NULL;

    this->pPointer = p;
    this->filePath = DATA_DIR + to_string(p.fileId); 
    
}

LeafNode::~LeafNode() {
    
    // TODO
}

// insert an entry into the leaf, need to split it if it is full
KeyNode* LeafNode::insert(const Key& k, const Value& v) {
    KeyNode* newChild = NULL;
    if(!(this->n < this->degree * 2))
    {
        newChild = this->split();

        if(newChild->key > k)
            ((LeafNode*)(newChild->node))->insertNonFull(k,v);
        else
            this->insertNonFull(k,v);
        return newChild;
    }
    else
    {
        this->insertNonFull(k,v);
        return newChild;
    }
}

// insert into the leaf node that is assumed not full
void LeafNode::insertNonFull(const Key& k, const Value& v) {
    Byte *t;
    Byte *t1 = fingerprints;
    t = bitmap;


    
    printf("value %d\n", v);

    int pos = this->findFirstZero();

    printf("pos is %d\n",pos);

    if(pos != -1 && pos < this->degree * 2){
        t += pos / 8;
        *t |= (1 << (7 - (pos % 8)));

        t1 += pos;
        *t1 = keyHash(k);
        kv[pos].k = k;
        kv[pos].v = v;
        n += 1;
        this->persist();
    }

}

// split the leaf node
KeyNode* LeafNode::split() {
    KeyNode* newChild = new KeyNode();
    LeafNode* tmp = new LeafNode(this->tree);
    Key midKey = findSplitKey();
    
    Byte * t;

    for(int i = 0;i < bitmapSize;i++)
    {
        *(tmp->bitmap+i) = 0;
    }

    for(int i = 0;i < this->degree * 2;i++)
    {
        if(this->kv[i].k >= midKey)
        {
            tmp->kv[i] = this->kv[i];

            t = tmp->bitmap + i / 8;
            *t |= (1 << (7 - (i % 8)));

            t = tmp->fingerprints + i;
            *t = *(fingerprints + i);
            
            t  = bitmap + i / 8;
            *t &= ~(1 << (7 - i % 8));
        }
    }
    tmp->n = n / 2;
    n /= 2;
    tmp->degree = this->degree / 2;
    this->degree /= 2;

    tmp->next = this->next;
    tmp->prev = this;

    this->next = tmp;
    this->persist();
    tmp ->persist();

    newChild->key = midKey;
    newChild->node = tmp;
    return newChild;
}

int cmp(const void * a , const void * b)
{
    return *(Key*)b - *(Key*)a;
}

// use to find a mediant key and delete entries less then middle
// called by the split func to generate new leaf-node
// qsort first then find
Key LeafNode::findSplitKey() {
    Key midKey = 0;
    Key all[100];
    for(int i = 0;i < this->degree * 2;i++)
        all[i] = (kv++)->k;
    qsort(all,this->degree * 2,sizeof(Key),cmp);
    midKey = all[this->degree];
    return midKey;
}

// get the targte bit in bitmap
// TIPS: bit operation
int LeafNode::getBit(const int& idx) {
    Byte * t = this->bitmap;
    t += idx / 8;
    return (((*t) & (1 << (7 - idx % 8)))) != 0;
}

Key LeafNode::getKey(const int& idx) {
    return this->kv[idx].k;
}

Value LeafNode::getValue(const int& idx) {
    return this->kv[idx].v;
}

PPointer LeafNode::getPPointer() {
    return this->pPointer;
}

// remove an entry from the leaf
// if it has no entry after removement return TRUE to indicate outer func to delete this leaf.
// need to call PAllocator to set this leaf free and reuse it
bool LeafNode::remove(const Key& k, const int& index, InnerNode* const& parent, bool &ifDelete) {
    bool ifRemove = false;
    // TODO
    return ifRemove;
}

// update the target entry
// return TRUE if the update succeed
bool LeafNode::update(const Key& k, const Value& v) {
    bool ifUpdate = false;
    // TODO
    return ifUpdate;
}

// if the entry can not be found, return the max Value
Value LeafNode::find(const Key& k) {
    // for(int i = 0;i < this->bitmapSize;i++)
    // {
    //     Byte * t = bitmap + i;
    //     if(*t == 1)
    //     {
    //         Byte *t1 = this->fingerprints + i;
    //         if(keyHash(k) == *t1)
    //         {
    //             return this->kv[i].v;
    //         }
    //     }
    // }

    cout << "insert it "<< getBit(0) << endl;

    

    for(int i = 0;i < this->n;i ++){
        cout <<"n "<< n << endl;
        if(getBit(i) == 1) {
            cout << "nothing1 \n";
            Byte *t1 = this->fingerprints + i;
            if(keyHash(k) == *t1)
            {
                cout << "nothing2 \n";
                // printf("key == : %d\n",this->kv[i].k);
                cout << "kv -> v " << this->kv[i].v << endl;
                return this->kv[i].v;
            }
        }
    }
    return NULL;
}

// find the first empty slot
int LeafNode::findFirstZero() {
    Byte * bpt = this->bitmap;
    for(int i = 0;i < this->bitmapSize;i++){
        for(int j = 7; j >= 0; j--) {
            // printf(" bpt is : %d\n",*bpt);
            // cout << "abc  "<<((!(*bpt)) & (1 << j)) << endl;
            // cout << "shiff " << (1 << j) << " j is "<< j << endl;
            if((~(*bpt) & (1 << j)))
            {
                return i * 8 + 7 - j ;
            }
        }
        bpt++;
    }
    return -1;
}

// persist the entire leaf
// use PMDK
void LeafNode::persist() {
    char * pmemaddr;
    size_t mapped_len;
    int is_pmem;

    string path = DATA_DIR  +to_string(this->pPointer.fileId);
    pmemaddr = (char*)pmem_map_file(path.c_str(), LEAF_GROUP_HEAD + LEAF_GROUP_AMOUNT * calLeafSize(), 
                                    PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem);

    // cout << "pmem_addr " << *((uint64_t *)pmem_addr) << endl;
    // cout << "here\n";
    Byte *t = (Byte *) (pmemaddr + this->pPointer.offset);
    // cout << "here1\n";
    // Byte * bits = (Byte *) this->bitmap;

    for(int i = 0;i < bitmapSize;i++)
    {
        *(t++) = *(this->bitmap + i);
        // cout << "cal num " << countOneBits(*(this->bitmap+i)) << endl;
    }
    // cout << "here2\n";
    PPointer *t1 = (PPointer *)t;
    if(this->pNext == nullptr)
        t1 ++;
    else  *(t1++) = *(this->pNext);
    // cout << "here3\n";

    Byte *t2 = (Byte *)t1;
    // cout << "here3\n";
    for(int i = 0;i < 2*LEAF_DEGREE;i++){
        *(t2++) = *(this->fingerprints + i);
    }
    // cout << "here\n";
    KeyValue * t3 = (KeyValue *)t2;
    for(int i = 0;i < 2*LEAF_DEGREE;i++){
        *(t3++) = this->kv[i];
    }

    if (is_pmem){
        pmem_persist(pmemaddr, mapped_len);
        // pmem_persist(this->pmem_addr,calLeafSize());
    }      
    else{
        pmem_msync(pmemaddr, mapped_len);
        // pmem_msync(this->pmem_addr,calLeafSize());
    }
    // PAllocator* pa = PAllocator::getAllocator();
    // pa->~PAllocator();
}

// call by the ~FPTree(), delete the whole tree
void FPTree::recursiveDelete(Node* n) {
    if (n->isLeaf) {
        delete n;
    } else {
        for (int i = 0; i < ((InnerNode*)n)->nChild; i++) {
            recursiveDelete(((InnerNode*)n)->childrens[i]);
        }
        delete n;
    }
}

FPTree::FPTree(uint64_t t_degree) {
    FPTree* temp = this;
    this->root = new InnerNode(t_degree, temp, true);
    this->degree = t_degree;
    bulkLoading();
}

FPTree::~FPTree() {
    recursiveDelete(this->root);
}

// get the root node of the tree
InnerNode* FPTree::getRoot() {
    return this->root;
}

// change the root of the tree
void FPTree::changeRoot(InnerNode* newRoot) {
    this->root = newRoot;
}

void FPTree::insert(Key k, Value v) {
    if (root != NULL) {
        root->insert(k, v);
    }
}

bool FPTree::remove(Key k) {
    if (root != NULL) {
        bool ifDelete = false;
        InnerNode* temp = NULL;
        return root->remove(k, -1, temp, ifDelete);
    }
    return false;
}

bool FPTree::update(Key k, Value v) {
    if (root != NULL) {
        return root->update(k, v);
    }
    return false;
}

Value FPTree::find(Key k) {
    if (root != NULL) {
        return root->find(k);
    }
}

// call the InnerNode and LeafNode print func to print the whole tree
// TIPS: use Queue
void FPTree::printTree() {
    // TODO
}

// bulkLoading the leaf files and reload the tree
// need to traverse leaves chain
// if no tree is reloaded, return FALSE
// need to call the PALlocator  
bool FPTree::bulkLoading() {
    PALlocator* palloc = getAllocator();
    PPointer ppt = palloc->getStartPointer();
    if (!ifLeafUsed(ppt)) return false;

    while (ifLeafUsed(ppt)) {
        LeafNode* leaf = new LeafNode(ppt, this);
        Key minKey = MAX_KEY;
        Key leafKey;
        for (int i = 0; i < leaf->degree * 2; ++ i) {
            if (getBit(i)) {
                leafKey = leaf->kv[i].k;
                minKey = minKey <= leafKey ? minKey : leafKey;
            }
        }
        KeyNode kn;
        kn.key = minKey;
        kn.node = leaf;
        root->insertLeaf(kn);
        ppt = *(leaf->pNext);
    }
}
