## fptree_test文件使用说明

### 测试insert功能的TEST有：

#### TEST(FPTreeTest, SingleInsert) {} 

这个TEST检测的函数为：

1. LeafNode::find()
2. FPtree::insert()
3. getBit()
4. LeafNode::LeafNode(PPointer,FPtree*)

插入一个键值对(1,100) 检测以下内容是否成立：

1. 根节点的第一个孩子为叶子节点leaf

2. 对上述叶子节点使用函数find(1) ,查找到的值为100
3. 该叶子节点的fileId=1
4. 该叶子节点存储在leafGroupd的最后一个叶节点上
5. 初始化函数使用leaf->PPointer作为参数，得到和上述叶子节点相同的一个叶子节点t_leaf
6. 使用getBit()函数查找bitmap中第0个bit
7. 对叶子节点复制品t_leaf使用函数find(1),查到的值为10

#### TEST(FPTreeTest, InsertOneLeaf) {}

这个TEST检测的函数为：

1. LeafNode::insert()
2. LeafNode::find()
3. LeafNode::LeafNode(FPtree*)
4. LeafNode::LeafNode(PPointer,FPtree*)

声明一个叶子节点l1，向l1中连续插入LEAF_DEGREE个键值对（i，10*i) **1<=i<=LEAF_DEGREE** ，并检查以下内容是否成立：

1. 第一个LeafGroup文件中bitmap的最后一个bit为1
2. 对插入键值对而产生的叶子节点进行查找操作find(1)，返回的值为10

声明一个新的叶节点l2,向l2中插入LEAF_DEGREE个键值对(i，10*i)  **LEAF_DEGREE+1<= i <=2 *LEAF_DEGREE**,并检查以下要求是否成立：

1. 叶子节点l1被分配在LeafGroup文件的最后一个位置
2. 叶子节点l2被分配在LeafGroup文件的倒数第二个位置
3. 在l1中查找键1，即使用find(1)函数，返回的值为10
4. 在l2中查找键 LEAF_DEGREE + 1，即使用find(LEAF_DEGREE + 1)，返回的值为10*(LEAF_DEGREE + 1)

#### TEST(FPTreeTest, PersistLeaf) {}

这个TEST检测的函数为:

1. LeafNode::persist()
2. leafNode::InsertNonFull()
3. LeafNode::find()

声明一个叶子节点leaf，向叶子节点中插入两个键值对(1,100),(2,100),并对叶子进行持久化操作，即leaf->persist()随后将叶子节点leaf删除，并使用LeafNode::LeafNode(PPointer,FPtree*)获取fileId=1的LeafGroup文件中最后一个叶子节点，设为t_leaf，并检查以下要求是否成立：

1. 对t_leaf进行查找操作find(1)，查到的值为100
2. 对t_leaf进行查找操作find(2)，查到的值为200

### 测试BulkLoading的TEST有:

