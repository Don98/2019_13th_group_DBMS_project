#include"fptree/fptree.h"
#include <algorithm>
using namespace std;

// Initial the new InnerNode
// the keys num nKeys is constrainted to d <= nKeys <= 2d except root,
// thus the child num nChild is constrainted to d + 1 <= nChile <= 2d + 1,
// also expect root,
// root is constrainted to 0 <= nKeys <= 2d, and 0 <= nChild <= 2d + 1,
// but we add one buffer to keys array and childs array for some Convenience
// when implement the insert function
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
    // when it has no entry, just return
    if(node == NULL || ((InnerNode*)node)->nKeys == 0) {
        printf("error: InnerNode can not insert when it has no entry\n");
        return;
    }
    // find the position of the key to insert in key array and the position
    // of the node to insert in child array
    int pos = findIndex(k);

    // the posision of the insert key is pos, need to move the keys behind the
    // position back one by one and the set keys[pos] as insert key, and
    // increase the keys num nKeys
    for (int i = nKeys; i > pos; i--) keys[i] = keys[i - 1];
    keys[pos] = k;
    nKeys++;

    // the posision of the insert node is pos + 1, need to move the childs
    // behind the position back one by one and the set child[pos + 1] as
    // insert node, and then increase the child num nChilds
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
        // if nChild is 0, there is no leaf node, create one and insert the key
        // value to the leaf node, and set the new leaf node as child
        if (this->nChild == 0) {
            LeafNode* leaf_node = new LeafNode(this->tree);
            leaf_node->insert(k, v);
            this->childrens[nChild++] = leaf_node;
        } else {
            // if nChile is not 0, then nChile is 1, the Child must be a leaf,
            // insert the kv to leaf, and check the return value which
            // imdicate whether the leaf split, if split, then add the new leaf
            // to Child array and the new key to key array
            newChild = this->childrens[0]->insert(k, v);
            if (newChild != NULL) {
                this->keys[nKeys++] = newChild->key;
                this->childrens[nChild++] = newChild->node;
                // note that the newChild is a keynode and be new by child, it
                // MUST DELETE, or it will lead to MEMORY LEAK
                delete newChild;
                newChild = NULL;
            }
        }
        return newChild;
    }

    // 2.recursive insertion
    // find the position to insert kv, and call the child insert it
    int pos = findIndex(k);
    newChild = this->childrens[pos]->insert(k, v);
    // if the newChild return by child is not NULL, need to insert
    // newChild's key and node to this
    if (newChild != NULL) {
        this->insertNonFull(newChild->key, newChild->node);
        // after insert the newChild to this, delete the newChild and set
        // the newChild as NULL
        delete newChild;
        newChild = NULL;
        // if this's key num if greater than 2d, need to split, and return
        // the newChild to this's parent
        if (this->nKeys > 2 * this->degree) {
            newChild = this->split();
        }
    }

    // if this is root, there is no parent to handle the newChlid, so need
    // to handle the newChild by itself
    // need to new a InnerNode and set it to root, the only one key is
    // newChild's key, the first child is this, the second child is newChild's
    // node
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
    // if needn't to split, return NULL
    if(this->nKeys <= 2*this->degree ){
        printf("error: InnerNode split innernode when not full\n");
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
    if (this->isRoot && this->nKeys == 0) {
        if (this->nChild > 0) {
            ifRemove = this->childrens[0]->remove(k, 0, this, ifDelete);
            if (ifDelete) {
                delete this->childrens[0];
                this->nChild--;
            }
        }
        return ifRemove;
    }

    // recursive remove
    int pos = this->findIndex(k);
    ifRemove = this->childrens[pos]->remove(k, pos, this, ifDelete);

    // if the needn't to delete the child, return the result whether
    // remove key is success
    if (!ifDelete) return ifRemove;

    // this occurs when the root has 2 leaf and the root and its two child
    // has merge, when merge the root and two child, the root has 2d keys and
    // 2d + 1 childs, but we put the nChild as 0 to indicate that the root has
    // been merge, and the child in position nChild is the pointer of the node
    // that call merge, delete it and return
    if (this->isRoot && this->nChild == 0) {
        this->nChild = this->nKeys + 1;
        delete this->childrens[this->nChild];
        return ifRemove;
    }

    // otherwise delete the child and key that need to delete
    // the key index is alway the same as the child index except the child to
    // delete is at the last position, in this case, the key position is also
    // the last position
    delete this->childrens[pos];
    int keyIdx = pos < nKeys ? pos : pos - 1;
    this->removeChild(keyIdx, pos);

    // if the key num is greater or equal to degree, it needn't to go further
    // set ifDelete to false
    if (this->nKeys >= this->degree) {
        ifDelete = false;
    }

    else {
        // if is root, it is allow that the keys is less than degree, return
        if (this->isRoot) return ifRemove;
        InnerNode* leftBro = NULL;
        InnerNode* rightBro = NULL;
        // get the brother of this
        getBrother(index, parent, leftBro, rightBro);

        // if this can redistribute of it's right brother, redistribute and set
        // ifDelete to false
        if (rightBro != NULL && rightBro->nKeys + this->nKeys >= 2 * this->degree) {
            redistributeRight(index, rightBro, parent);
            ifDelete = false;
        }
        // else if this can redistribute of it's left brother, redistribute and set
        // ifDelete to false
        else if (leftBro != NULL && leftBro->nKeys + this->nKeys >= 2 * this->degree) {
            redistributeLeft(index, leftBro, parent);
            ifDelete = false;
        }
        // else need to delete at lease one node
        else {
            ifDelete = true;
            // is this is root and has only 2 leaf, it means that the key num
            // of root is one, the key num of its two child is one d - 1 and
            // the other d (or it the two child has already redistribute, won't
            // go into this block), merge the root and its two child
            if (parent->isRoot && parent->nChild == 2) {
                if (leftBro != NULL) mergeParentLeft(parent, leftBro);
                else mergeParentRight(parent, rightBro);
            }
            // else merge the node and it's brother, prior is right
            else {
                if (rightBro != NULL) mergeRight(rightBro, parent->keys[index]);
                else mergeLeft(leftBro, parent->keys[index - 1]);
            }
        }
    }

    return ifRemove;
}

// If the leftBro and rightBro exist, the rightBro is prior to be used
void InnerNode::getBrother(const int& index, InnerNode* const& parent, InnerNode* &leftBro, InnerNode* &rightBro) {
    // if the index is out of bound
    if (parent == NULL || index < 0 || index >= parent->nChild) {
        leftBro = NULL;
        rightBro = NULL;
        return;
    }

    if (index == 0) leftBro = NULL;
    else leftBro = (InnerNode*) parent->childrens[index - 1];

    if (index == parent->nChild - 1) rightBro = NULL;
    else rightBro = (InnerNode*) parent->childrens[index + 1];
}

// merge this node, its parent and left brother(parent is root)
// after the function, the keys and childs of its two child be moved to root
void InnerNode::mergeParentLeft(InnerNode* const& parent, InnerNode* const& leftBro) {
    // the new position of root's key is leftBro's keynum
    parent->keys[leftBro->nKeys] = parent->keys[0];
    for (int i = 0; i < leftBro->nKeys; i++) {
        parent->keys[i] = leftBro->keys[i];
    }
    for (int i = 0; i < this->nKeys; i++) {
        parent->keys[i + leftBro->nKeys + 1] = this->keys[i];
    }
    for (int i = 0; i < leftBro->nChild; i++) {
        parent->childrens[i] = leftBro->childrens[i];
    }
    for (int i = 0; i < this->nChild; i++) {
        parent->childrens[i + leftBro->nChild] = this->childrens[i];
    }
    parent->nKeys = leftBro->nKeys + this->nKeys + 1;
    parent->nChild = leftBro->nChild + this->nChild;
    // we need to delete the origin two child, but it can't delete this pointer
    // and root has only one childs buffer now, so we delete the brother, and
    // move this pointer to the back of root's child array, and set root's
    // nChild to 0 to indicate that the root has been merge with its two childs
    // and need to delete on pointer in root's 2d + 2 position
    parent->childrens[parent->nChild] = this;
    parent->nChild = 0;
    delete leftBro;
}

// merge this node, its parent and right brother(parent is root)
// after the function, the keys and childs of its two child be moved to root
void InnerNode::mergeParentRight(InnerNode* const& parent, InnerNode* const& rightBro) {
    parent->keys[this->nKeys] = parent->keys[0];
    for (int i = 0; i < this->nKeys; i++) {
        parent->keys[i] = this->keys[i];
    }
    for (int i = 0; i < rightBro->nKeys; i++) {
        parent->keys[i + this->nKeys + 1] = rightBro->keys[i];
    }
    for (int i = 0; i < this->nChild; i++) {
        parent->childrens[i] = this->childrens[i];
    }
    for (int i = 0; i < rightBro->nChild; i++) {
        parent->childrens[i + this->nChild] = rightBro->childrens[i];
    }
    parent->nKeys = this->nKeys + rightBro->nKeys + 1;
    parent->nChild = this->nChild + rightBro->nChild;
    parent->childrens[parent->nChild] = this;
    parent->nChild = 0;
    delete rightBro;
}

// this node and its left brother redistribute
// the left has more entries
void InnerNode::redistributeLeft(const int& index, InnerNode* const& leftBro, InnerNode* const& parent) {
    // redistribute the keys of this and its brother, after redistribute, the
    // new key num is half of the sum of this and its brother's key num
    // nedd to move moveKeys from left to right
    // but CAUTIOUS that the key of parent in position index need to update
    // it means that when redistribute, the keys order is
    // [leftBro's keys] [parent key in index] [this' keys]
    // we need to move the keys by order
    int moveKeys = (leftBro->nKeys + this->nKeys) / 2 - leftBro->nKeys;
    for (int i = nKeys - 1; i >= 0; i--) {
        this->keys[i + moveKeys] = this->keys[i];
    }
    for (int i = nChild - 1; i >= 0; i--) {
        this->childrens[i + moveKeys] = this->childrens[i];
    }
    this->keys[moveKeys - 1] = parent->keys[index];
    for (int i = 1; i < moveKeys; i++) {
        this->keys[moveKeys - 1 - i] = leftBro->keys[leftBro->nKeys - i];
    }
    for (int i = 0; i < moveKeys; i++) {
        this->childrens[moveKeys - i - 1] = leftBro->childrens[leftBro->nChild - 1 - i];
    }
    this->nKeys += moveKeys;
    this->nChild += moveKeys;

    parent->keys[index] = leftBro->keys[leftBro->nKeys - moveKeys];

    leftBro->nKeys -= moveKeys;
    leftBro->nChild -= moveKeys;
}

// this node and its right brother redistribute
// the right has more entries
void InnerNode::redistributeRight(const int& index, InnerNode* const& rightBro, InnerNode* const& parent) {
    int moveKeys = (this->nKeys + rightBro->nKeys) / 2 - this->nKeys;
    this->keys[nKeys] = parent->keys[index];
    for (int i = 1; i < moveKeys; i++) {
        this->keys[nKeys + i] = rightBro->keys[i - 1];
    }
    for (int i = 0; i < moveKeys; i++) {
        this->childrens[nChild + i] = rightBro->childrens[i];
    }
    this->nKeys += moveKeys;
    this->nChild += moveKeys;

    parent->keys[index] = rightBro->keys[moveKeys - 1];

    for (int i = 0; i < rightBro->nKeys - moveKeys; i++) {
        rightBro->keys[i] = rightBro->keys[i + moveKeys];
    }
    for (int i = 0; i < rightBro->nChild - moveKeys; i++) {
        rightBro->childrens[i] = rightBro->childrens[i + moveKeys];
    }
    rightBro->nKeys -= moveKeys;
    rightBro->nChild -= moveKeys;
}

// merge all entries to its left bro, delete this node after merging.
void InnerNode::mergeLeft(InnerNode* const& leftBro, const Key& k) {
    leftBro->keys[leftBro->nKeys] = k;
    for (int i = 0; i < this->nKeys; i++) {
        leftBro->keys[leftBro->nKeys + i + 1] = this->keys[i];
    }
    for (int i = 0; i < this->nChild; i++) {
        leftBro->childrens[leftBro->nChild + i] = this->childrens[i];
    }
    leftBro->nKeys += (this->nKeys + 1);
    leftBro->nChild += this->nChild;
}

// merge all entries to its right bro, delete this node after merging.
void InnerNode::mergeRight(InnerNode* const& rightBro, const Key& k) {
    for (int i = rightBro->nKeys - 1; i >= 0; i--) {
        rightBro->keys[i + this->nKeys + 1] = rightBro->keys[i];
    }
    rightBro->keys[this->nKeys] = k;
    for (int i = 0; i < this->nKeys; i++) {
        rightBro->keys[i] = this->keys[i];
    }
    for (int i = rightBro->nChild - 1; i >= 0; i--) {
        rightBro->childrens[i + this->nChild] = rightBro->childrens[i];
    }
    for (int i = 0; i < this->nChild; i++) {
        rightBro->childrens[i] = this->childrens[i];
    }
    rightBro->nKeys += (this->nKeys + 1);
    rightBro->nChild += this->nChild;
}

// remove a children from the current node, used by remove func
void InnerNode::removeChild(const int& keyIdx, const int& childIdx) {
    // index out of bound
    if (keyIdx < 0 || keyIdx >= this->nKeys || childIdx < 0 || childIdx >= nChild) {
        printf("error: InnerNode removeChild with index out of bound\n");
        return;
    }
    for (int i = keyIdx; i < nKeys - 1; i++) {
        this->keys[i] = this->keys[i + 1];
    }
    this->nKeys--;
    for (int i = childIdx; i < nChild - 1; i++) {
        this->childrens[i] = this->childrens[i + 1];
    }
    this->nChild--;
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

    // get a leaf from pAllocator
    // if fail to get a leaf, return
    PAllocator* palloc = PAllocator::getAllocator();
    PPointer ppt;
    char* pmem_addr;
    if (!palloc->getLeaf(ppt, pmem_addr)) {
        printf("error：LeafNode get leaf fail\n");
        return;
    }

    this->bitmapSize = (LEAF_DEGREE * 2 + 7) / 8;
    // set the pointer pointed to NVM address
    this->pmem_addr = pmem_addr;
    this->bitmap = (Byte*) pmem_addr;
    this->pNext = (PPointer*)(this->bitmap + bitmapSize);
    this->fingerprints = (Byte*)(this->pNext + 1);
    this->kv = (KeyValue*)(this->fingerprints + LEAF_DEGREE * 2);

    // the DRAM relative variables
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
        printf("error：LeafNode build leaf fail, ppointer invalid.\n");
    }

    this->bitmapSize = (LEAF_DEGREE * 2 + 7) / 8;
    this->pmem_addr = pmem_addr;
    this->bitmap = (Byte*) pmem_addr;
    this->pNext = (PPointer*)(this->bitmap + bitmapSize);
    this->fingerprints = (Byte*)(this->pNext + 1);
    this->kv = (KeyValue*)(this->fingerprints + LEAF_DEGREE * 2);

    this->n = 0;
    for (uint64_t i = 0; i < bitmapSize; ++ i) {
        this->n += countOneBits(this->bitmap[i]);
    }

    this->prev = NULL;
    this->next = NULL;
    this->pPointer = p;
    this->filePath = DATA_DIR + to_string(p.fileId);

}

// persist the leaf when destruct
LeafNode::~LeafNode() {
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
        printf("error: LeafNode insertNotFull when it is full.\n");
        return;
    }
    // bit operation to set the pos bit of bitmap to 1
    this->bitmap[pos / 8] |= (1 << (7 - pos % 8));

    this->fingerprints[pos] = keyHash(k);
    this->kv[pos].k = k;
    this->kv[pos].v = v;
    this->n++;
    persist();
}

// split the leaf node
KeyNode* LeafNode::split() {
    // if this' key num is less than 2d, needn't to split
    if (this->n < 2 * this->degree) {
        return NULL;
    }

    KeyNode* newChild = new KeyNode();
    LeafNode* newNode = new LeafNode(this->tree);
    Key midKey = findSplitKey();

    Byte newBitmap[(LEAF_DEGREE * 2 + 7) / 8];
    memset(newBitmap, 0, bitmapSize);


    // find the keys greater or equal to split key to new leaf and move
    // when move the key vaule, it need to set the same position of new leaf's
    // bitmap bit to 1, and copy the fingerprints and kv of the position to
    // new leaf's same position, at last set this' bitmap bit to 0
    for (int i = 0; i < this->degree * 2; i++) {
        if (this->kv[i].k >= midKey) {
            newBitmap[i / 8] |= (1 << (7 - (i % 8)));
            newNode->fingerprints[i] = this->fingerprints[i];
            newNode->kv[i] = this->kv[i];
        }
    }

    for (int i = 0; i < bitmapSize; ++ i) {
        newNode->bitmap[i] = newBitmap[i];
    }
    *(newNode->pNext) = *(this->pNext);
    newNode->persist();

    for (int i = 0; i < bitmapSize; ++ i) {
        this->bitmap[i] = ~newBitmap[i];
    }
    *(this->pNext) = newNode->pPointer;
    this->persist();

    // after split,
    newNode->n = this->degree;
    this->n = this->degree;

    // set the prev link
    newNode->prev = this;
    if (this->next != NULL) {
        this->next->prev = newNode;
    }

    // set the next link
    newNode->next = this->next;
    this->next = newNode;

    newChild->key = midKey;
    newChild->node = newNode;

    return newChild;
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
    sort(all, all + LEAF_DEGREE * 2);
    midKey = all[LEAF_DEGREE];
    return midKey;
}

// get the targte bit in bitmap
// TIPS: bit operation
// return 0 if bit at idx is 0, otherwise non zero
int LeafNode::getBit(const int& idx) {
    if (idx < 0 || idx >= this->degree * 2) return 0;
    return (this->bitmap[idx / 8] & (1 << (7 - idx % 8))) == 0 ? 0 : 1;
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

    // find the position of the move key
    Byte hash = keyHash(k);
    int pos;
    for (pos = 0; pos < this->degree * 2; ++ pos) {
        if (getBit(pos) && hash == this->fingerprints[pos]) {
            if (k == this->kv[pos].k) break;
        }
    }
    // if the move key is in the leaf, set the pos bit to 0
    if (pos >= 0 && pos < 2 * this->degree) {
        this->n--;
        this->bitmap[pos / 8] &= ~(1 << (7 - pos % 8));
        this->persist();
        ifRemove = true;
        ifDelete = false;
        // if it has no entry after removement set ifDelete to TRUE to indicate
        // outer func to delete this leaf.
        if (this->n == 0) {
            PAllocator* palloc = PAllocator::getAllocator();
            if (!palloc->freeLeaf(this->pPointer)) {
                printf("error: in LeafNode::remove -> PAllocator::freeLeaf\n");
                // TODO throw an error
            }
            if (this->prev != NULL) {
                prev->next = this->next;
                *(prev->pNext) = *(this->pNext);
                prev->persist();
            }
            if (this->next != NULL) {
                next->prev = prev;
            }
            ifDelete = true;
        }
    }
    return ifRemove;
}

// update the target entry
// return TRUE if the update succeed
bool LeafNode::update(const Key& k, const Value& v) {
    bool ifUpdate = false;
    Byte hash = keyHash(k);
    for (int i = 0; i < this->degree * 2; ++ i) {
        if (getBit(i) && hash == this->fingerprints[i]) {
            if (k == this->kv[i].k) {
                this->kv[i].v = v;
                this->persist();
                ifUpdate = true;
                break;
            }
        }
    }
    return ifUpdate;
}

// if the entry can not be found, return the max Value
Value LeafNode::find(const Key& k) {
    Byte hash = keyHash(k);
    for (int i = 0; i < this->degree * 2; ++ i) {
        if (getBit(i) && hash == this->fingerprints[i]) {
            if (k == this->kv[i].k) {
                return this->kv[i].v;
            }
        }
    }
    return MAX_VALUE;
}

// find the first empty slot
int LeafNode::findFirstZero() {
    int i;
    for (i = 0; i < this->degree * 2; ++ i) {
        if (!getBit(i)) break;
    }
    return i;
}

// persist the entire leaf
// use PMDK
void LeafNode::persist() {
    if (PAllocator::getAllocator()->ifPmemAddr(this->pPointer)) {
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
    // level first traverse
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
    // if the start point is not used, the tree is empty
    // return false
    if (!palloc->ifLeafUsed(ppt)) {
        return false;
    }

    LeafNode* old_leaf = NULL;

    // traverse all the leaves by order, and insert the leaf to tree
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

        // set the leaf's prev and next pointer
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
