#include"fptree/fptree.h"

using namespace std;

// Initial the new InnerNode
InnerNode::InnerNode(const int& d, FPTree* const& t, bool _isRoot) {
    this->degree = d;
    this->tree = t;
    this->isLeaf = false;
    this->isRoot = _isRoot;
    this->nKeys = 0;
    this->nChild = 0;
    this->keys = new Key[2*d + 1];
    this->childrens = new Node*[2*d + 2];
}

// delete the InnerNode
InnerNode::~InnerNode() {
    delete[] keys;
    delete[] childrens;
}

// binary search the first key in the innernode larger than input key
int InnerNode::findIndex(const Key& k) {
    int left = 0, right = this->nKeys - 1, mid;
    while (left <= right) {
        mid = (left + right) / 2;
        if (this->keys[mid] > k) right = mid - 1;
        else left = mid + 1;
    }
    return left;
}

// insert the node that is assumed not full
// insert format:
// ======================
// | key | node pointer |
// ======================
// WARNING: can not insert when it has no entry
void InnerNode::insertNonFull(const Key& k, Node* const& node) {
    if(node == NULL || ((InnerNode*)node)->nKeys == 0) {
        printf("can not insert when it has no entry\n");
        return;
    }
    int pos = findIndex(k);
    for (int i = nKeys; i > pos; i--) keys[i] = keys[i - 1];
    keys[pos] = k;
    nKeys++;
    for (int i = nChild; i > pos + 1; i--) childrens[i] = childrens[i - 1];
    childrens[pos + 1] = node;
    nChild++;
}

// insert func
// return value is not NULL if split, returning the new child and a key to insert
KeyNode* InnerNode::insert(const Key& k, const Value& v) {
    KeyNode* newChild = NULL;

    // 1.insertion to the first leaf(only one leaf)
    if (this->isRoot && this->nKeys == 0) {
        if (this->nChild == 0) {
            LeafNode* leaf_node = new LeafNode(this->tree);
            leaf_node->insert(k, v);
            this->childrens[nChild++] = leaf_node;
        } else {
            newChild = this->childrens[0]->insert(k, v);
            if (newChild != NULL) {
                this->keys[nKeys++] = newChild->key;
                this->childrens[nChild++] = newChild->node;
                delete newChild;
                newChild = NULL;
            }
        }
        return newChild;
    }
    
    // 2.recursive insertion
    int pos = findIndex(k);
    newChild = this->childrens[pos]->insert(k, v);
    if (newChild != NULL) {
        this->insertNonFull(newChild->key, newChild->node);
        delete newChild;
        newChild = NULL;
        if (this->nKeys > 2 * this->degree) {
            newChild = this->split();
        }
    }

    if (this->isRoot && newChild != NULL) {
        InnerNode* newRoot = new InnerNode(this->degree, this->tree, true);
        this->isRoot = false;
        newRoot->keys[newRoot->nKeys++] = newChild->key;
        newRoot->childrens[newRoot->nChild++] = this;
        newRoot->childrens[newRoot->nChild++] = newChild->node;
        tree->changeRoot(newRoot);
        delete newChild;
        newChild =  NULL;
    }

    return newChild;
}

// ensure that the leaves inserted are ordered
// used by the bulkLoading func
// inserted data: | minKey of leaf | LeafNode* |
KeyNode* InnerNode::insertLeaf(const KeyNode& leaf) {
    // if(this->isRoot) tree->printTree();
    KeyNode* newChild = NULL;
    // first and second leaf insertion into the tree
    if (this->isRoot && this->nKeys == 0) {
        if (this->nChild == 0) {
            this->childrens[nChild++] = leaf.node;
        } else {
            this->keys[nKeys++] = leaf.key;
            this->childrens[nChild++] = leaf.node;
        }
        return newChild;
    }
    
    // recursive insert
    // Tip: please judge whether this InnerNode is full
    // next level is not leaf, just insertLeaf
    int pos = findIndex(leaf.key);
    if (!this->childrens[pos]->ifLeaf()) {
        newChild = ((InnerNode*)(this->childrens[pos]))->insertLeaf(leaf);
        if (newChild != NULL) {
            this->insertNonFull(newChild->key, newChild->node);
            delete newChild;
            newChild = NULL;
            if (this->nKeys > 2 * this->degree) {
                newChild = this->split();
            }
        }
    }

    // next level is leaf, insert to childrens array
    else {
        this->keys[nKeys++] = leaf.key;
        this->childrens[nChild++] = leaf.node;
        if (nKeys > 2 * this->degree) {
            newChild = this->split();
        }
    }

    if (this->isRoot && newChild != NULL) {
        InnerNode* newRoot = new InnerNode(this->degree, this->tree, true);
        this->isRoot = false;
        newRoot->keys[newRoot->nKeys++] = newChild->key;
        newRoot->childrens[newRoot->nChild++] = this;
        newRoot->childrens[newRoot->nChild++] = newChild->node;
        tree->changeRoot(newRoot);
        delete newChild;
        newChild =  NULL;
    }

    return newChild;
}

KeyNode* InnerNode::split() {
    if(this->nKeys <= 2*this->degree ){
        printf("error: split innernode when not full\n");
        return NULL;
    }

    KeyNode* newChild = new KeyNode();
    // right half entries of old node to the new node, others to the old node. 

    InnerNode * newNode = new InnerNode(this->degree,this->tree,false);

    newNode->nKeys = degree;
    newNode->nChild = degree + 1;
    for(int i = degree + 1; i <= 2 * degree; i++) {
        newNode->keys[i - degree - 1] = this->keys[i];
        newNode->childrens[i - degree - 1] = this->childrens[i];
    }
    newNode->childrens[degree] = this->childrens[nChild - 1];

    newChild->key = this->keys[degree];
    newChild->node = (Node*)newNode;

    this->nKeys = degree;
    this->nChild = degree + 1;

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
    int pos = findIndex(k);
    if (this->childrens[pos] != NULL) {
        return this->childrens[pos]->update(k, v);
    }
    return false;
}

// find the target value with the search key, return MAX_VALUE if it fails.
Value InnerNode::find(const Key& k) {
    int pos = findIndex(k);
    if (this->childrens[pos] != NULL) {
        return this->childrens[pos]->find(k);
    }
    return MAX_VALUE;
}

// get the children node of this InnerNode
Node* InnerNode::getChild(const int& idx) {
    if (idx >= 0 && idx < nChild) {
        return this->childrens[idx];
    }
    return NULL;
}

// get the key of this InnerNode
Key InnerNode::getKey(const int& idx) {
    if (idx >= 0 && idx < this->nKeys) {
        return this->keys[idx];
    }
    return MAX_KEY;
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

// new a empty leaf and set the valuable of the LeafNode
LeafNode::LeafNode(FPTree* t) {
    this->tree = t;
    this->degree = LEAF_DEGREE;
    this->isLeaf = true;

    PAllocator* palloc = PAllocator::getAllocator();
    PPointer ppt;
    char* pmem_addr;
    if (!palloc->getLeaf(ppt, pmem_addr)) {
        printf("error： get leaf fail\n");
    }

    this->bitmapSize = (LEAF_DEGREE * 2 + 7) / 8;
    this->pmem_addr = pmem_addr;
    this->bitmap = (Byte*) pmem_addr;
    this->pNext = (PPointer*)(this->bitmap + bitmapSize);
    this->fingerprints = (Byte*)(this->pNext + 1);
    this->kv = (KeyValue*)(this->fingerprints + LEAF_DEGREE * 2);

    this->n = 0;
    this->prev = NULL;
    this->next = NULL;
    this->pPointer = ppt;
    this->filePath = DATA_DIR + to_string(ppt.fileId);
}

// reload the leaf with the specific Persistent Pointer
// need to call the PAllocator
LeafNode::LeafNode(PPointer p, FPTree* t) {
    this->tree = t;
    this->degree = LEAF_DEGREE;
    this->isLeaf = true;

    PAllocator* palloc = PAllocator::getAllocator();

    char* pmem_addr;
    pmem_addr = palloc->getLeafPmemAddr(p);
    if (pmem_addr == NULL) {
        printf("error： build leaf fail, ppointer invalid.\n");
    }

    this->bitmapSize = (LEAF_DEGREE * 2 + 7) / 8;
    this->pmem_addr = pmem_addr;
    this->bitmap = (Byte*) pmem_addr;
    this->pNext = (PPointer*)(this->bitmap + bitmapSize);
    this->fingerprints = (Byte*)(this->pNext + 1);
    this->kv = (KeyValue*)(this->fingerprints + LEAF_DEGREE * 2);

    this->n = 0;
    for (uint64_t i = 0; i < bitmapSize; ++ i) {
        this->n += countOneBits(*(this->bitmap + i));
    }

    this->prev = NULL;
    this->next = NULL;
    this->pPointer = p;
    this->filePath = DATA_DIR + to_string(p.fileId);

}

LeafNode::~LeafNode() {
    // TODO
    persist();
}

// insert an entry into the leaf, need to split it if it is full
KeyNode* LeafNode::insert(const Key& k, const Value& v) {
    KeyNode* newChild = NULL;
    this->insertNonFull(k, v);
    if (this->n >= this->degree * 2) {
        newChild = this->split();
    }
    return newChild;
}

// insert into the leaf node that is assumed not full
void LeafNode::insertNonFull(const Key& k, const Value& v) {
    int pos = this->findFirstZero();
    if (pos < 0 || pos >= this->degree * 2) {
        printf("error: insertNotFull when is full\n.");
    }
    *(this->bitmap + pos / 8) |= (1 << (7 - pos % 8));
    *(this->fingerprints + pos) = keyHash(k);
    this->kv[pos].k = k;
    this->kv[pos].v = v;
    this->n++;
    persist();
}

// split the leaf node
KeyNode* LeafNode::split() {
    KeyNode* newChild = new KeyNode();
    LeafNode* newNode = new LeafNode(this->tree);
    Key midKey = findSplitKey();

    memset(newNode->bitmap, 0, bitmapSize);
    for (int i = 0; i < this->degree * 2; i++) {
        if (this->kv[i].k >= midKey) {
            newNode->bitmap[i / 8] |= (1 << (7 - (i % 8)));
            newNode->fingerprints[i] = this->fingerprints[i];
            newNode->kv[i] = this->kv[i];
            this->bitmap[i / 8] &= ~(1 << (7 - i % 8));
        }
    }

    newNode->n = this->degree;
    this->n = this->degree;

    PPointer pt = *(this->pNext);
    *(this->pNext) = newNode->pPointer;
    *(newNode->pNext) = pt;

    newNode->prev = this;
    if (this->next != NULL) {
        this->next->prev = newNode;
    }

    LeafNode* tmp = this->next;
    this->next = newNode;
    newNode->next = tmp;

    newChild->key = midKey;
    newChild->node = newNode;

    this->persist();
    newNode->persist();

    return newChild;
}

int cmp(const void* a , const void* b) {
    return *(Key*)a - *(Key*)b;
}


// use to find a mediant key and delete entries less then middle
// called by the split func to generate new leaf-node
// qsort first then find
Key LeafNode::findSplitKey() {
    Key midKey = 0;
    Key all[LEAF_DEGREE * 2];
    for(int i = 0; i < LEAF_DEGREE * 2; i++) {
        all[i] = kv[i].k;
    }
    qsort(all, LEAF_DEGREE * 2, sizeof(Key), cmp);
    midKey = all[LEAF_DEGREE];
    return midKey;
}

// get the targte bit in bitmap
// TIPS: bit operation
int LeafNode::getBit(const int& idx) {
    if (idx < 0 || idx >= this->degree * 2);
    Byte byte = this->bitmap[idx / 8];
    return (byte & (1 << 7 - idx % 8)) != 0;
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
    Byte hash = keyHash(k);
    for (int i = 0; i < this->degree * 2; ++ i) {
        if (getBit(i)) {
            if (hash == this->fingerprints[i]) {
                if (k == this->kv[i].k) {
                    this->kv[i].v = v;
                    ifUpdate = true;
                    break;
                }
            }
        }
    }
    return ifUpdate;
}

// if the entry can not be found, return the max Value
Value LeafNode::find(const Key& k) {
    Byte hash = keyHash(k);
    for (int i = 0; i < this->degree * 2; ++ i) {
        if (getBit(i)) {
            if (hash == this->fingerprints[i]) {
                if (k == this->kv[i].k) {
                    return this->kv[i].v;
                }
            }
        }
    }
    return MAX_VALUE;
}

// find the first empty slot
int LeafNode::findFirstZero() {
    Byte byte;
    int pos = -1;
    for (int i = 0; i < this->bitmapSize; ++ i) {
        if (pos != -1) break;
        byte = this->bitmap[i];
        for (int j = 0; j < 8; ++ j) {
            if ((~byte) & (1 << (7 - j))) {
                pos = i * 8 + j;
                break;
            }
        }
    }
    return pos < this->degree * 2 ? pos : -1;
}

// persist the entire leaf
// use PMDK
void LeafNode::persist() {
    if (pmem_is_pmem(this->pmem_addr, calLeafSize())) {
        pmem_persist(this->pmem_addr, calLeafSize());
    } else {
        pmem_msync(this->pmem_addr, calLeafSize());
    }
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
    queue<NodeLevel> q;
    int maxLevel = 0;
    NodeLevel cur, tmp;
    cur.node = root;
    cur.level = 0;
    q.push(cur);
    while (!q.empty()) {
        cur = q.front();
        q.pop();
        if (cur.level > maxLevel) {
            maxLevel++;
            cout << endl;
        }
        cur.node->printNode();
        if (!cur.node->ifLeaf()) {
            InnerNode* inner = (InnerNode*) cur.node;
            for (int i = 0; i < inner->getChildNum(); ++ i) {
                tmp.level = cur.level + 1;
                tmp.node = inner->getChild(i);
                q.push(tmp);
            }
        }
    }
    cout << endl;
}

// bulkLoading the leaf files and reload the tree
// need to traverse leaves chain
// if no tree is reloaded, return FALSE
// need to call the PALlocator
bool FPTree::bulkLoading() {
    PAllocator* palloc = PAllocator::getAllocator();
    PPointer ppt = palloc->getStartPointer();
    if (!palloc->ifLeafUsed(ppt)) {
        return false;
    }

    LeafNode* old_leaf = NULL;

    while (palloc->ifLeafUsed(ppt)) {
        LeafNode* leaf = new LeafNode(ppt, this);
        Key minKey = MAX_KEY;
        Key leafKey;
        for (int i = 0; i < LEAF_DEGREE * 2; i++) {
            if (leaf->getBit(i)) {
                leafKey = leaf->kv[i].k;
                minKey = minKey <= leafKey ? minKey : leafKey;
            }
        }
        KeyNode kn;
        kn.key = minKey;
        kn.node = leaf;

        leaf->prev = old_leaf;
        if (old_leaf != NULL) {
            old_leaf->next = leaf;
        }
        old_leaf = leaf;

        root->insertLeaf(kn);
        ppt = *(leaf->pNext);
    } 
    return false;
}
