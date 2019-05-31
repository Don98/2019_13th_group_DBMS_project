#include "fptree/fptree.h"
#include "gtest/gtest.h"
#include <iostream>
#include <thread>

using namespace std;

const string catalogPath = DATA_DIR + "p_allocator_catalog";
const string freePath = DATA_DIR + "free_list";
const string file1 = DATA_DIR + "1";
const string file2 = DATA_DIR + "2";
const string file3 = DATA_DIR + "3";
const string file4 = DATA_DIR + "4";

void removeFile() {
    PAllocator::getAllocator()->~PAllocator();
    remove(catalogPath.c_str());
    remove(file1.c_str());
    remove(freePath.c_str());
}
void find_one(FPTree* tree , int start){
    for (int i = start * 100 ; i < start * 100 + 100 ; i++) {
        EXPECT_EQ(tree->find(i),i * 10);
    }
}

TEST(CFPTreeTest, MultipFind) {
    FPTree* tree = new FPTree(2);
    const int thread_num = 5;

    for (int i = 0 ; i < thread_num * 100 ; i++)
        tree->insert(i, i * 10);

    thread all[thread_num];
    for(int i = 0;i < thread_num; i++)
    {
        all[i] = thread(find_one,tree,i);
    }
    for(int i = 0;i < thread_num;i++)
        all[i].join();
    delete tree;
    removeFile();
}
void update_one(FPTree* tree , int start){
    for (int i = start * 100 ; i < start * 100 + 100 ; i++) {
        tree->update(i,i * 10 + 1);
    }
}

TEST(CFPTreeTest, MultipUpdate) {
    FPTree* tree = new FPTree(2);
    const int thread_num = 5;

    for (int i = 0 ; i < thread_num * 100 ; i++)
        tree->insert(i, i * 10);

    thread all[thread_num];
    for(int i = 0;i < thread_num; i++)
    {
        all[i] = thread(update_one,tree,i);
    }
    for(int i = 0;i < thread_num;i++)
        all[i].join();
    for(int i = 0;i < thread_num * 10;i++){
        // cout << tree->find(i) << " " << i * 10 << endl;
        EXPECT_EQ(tree->find(i),i * 10 + 1);
    } 
    delete tree;
    removeFile();
}

void insert_one(FPTree* tree , int start){
    for (int i = start * 100 ; i < start * 100 + 100 ; i++) {
        // cout << i << "\n";
        tree->insert(i, i * 10);
    }
}
TEST(CFPTreeTest, MultipInsert) {
    FPTree* tree = new FPTree(2);

    const int thread_num = 2;

	thread all[thread_num];
	for(int i = 0;i < thread_num; i++)
	{
		all[i] = thread(insert_one,tree,i);
	}
    for(int i = 0;i < thread_num;i++){
		// cout << i << endl;
        all[i].join();
    }
    for(int i = 0;i < thread_num * 10;i++){
        // cout << tree->find(i) << " " << i * 10 << endl;
        EXPECT_EQ(tree->find(i),i * 10);
    } 
    delete tree;
    removeFile();    
}


void remove_one(FPTree* tree , int start){
    int i = start * LEAF_DEGREE;
    for (int j = 0;  j < LEAF_DEGREE; i++, j++) {
        // cout << i << endl;
        tree->remove(i);
    }
    tree->remove(i);
}
TEST(CFPTreeTest, MultipRemove) {
    FPTree* tree = new FPTree(2);

    const int thread_num = 2;

    for (int i = 0 ; i < thread_num * LEAF_DEGREE ; i++)
        tree->insert(i, i * 10);

    thread all[thread_num];
    for(int i = 0;i < thread_num; i++)
    {
        all[i] = thread(remove_one,tree,i);
    }
    for(int i = 0;i < thread_num;i++){
        all[i].join();
    }
    // tree->printTree();
    PPointer p;
    p.fileId = 1;
    for(int i = 1;i <= 5; i++){
        p.offset = LEAF_GROUP_HEAD + calLeafSize() * (LEAF_GROUP_AMOUNT - i);
        PAllocator* pa = PAllocator::getAllocator();
        EXPECT_EQ(pa->ifLeafUsed(p), false);
        EXPECT_EQ(pa->ifLeafFree(p), true);
    }
    delete tree;    
    removeFile();    
}