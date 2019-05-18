# 系统说明书

[一份GitHub的使用教程](./GitHub使用提示.md)

# 数据库课程设计系统说明

我们要实现一个键值存储系统，系统的实现参考leveldb以及FPTree。

leveldb是google的一个键值存储系统，github仓库见https://github.com/google/leveldb

FPTree是Oukid提出的一种适用于SCM(Storage Class Memory)的一类新型的B树，详见前期工作与论文阅读总结文件夹下的Oukid_FPTree.pdf论文。

另外，我们需要实现NVM文件管理的主要对象PAllocator，对应着p_allocator.cpp，其负责分配LeafNode在NVM中的空间，映射数据文件并返回虚拟地址给LeafNode使用。其管理的叶子文件的粒度是一个LeafGroup，一个LeafGroup由多个叶子以及一个数据头组成，数据头由一个8字节的当前LeafGroup使用叶子数和叶子槽的bitmap，bitmap为了简单使用1个byte指明一个槽位。

## FPTreeDB键值存储系统

本次课程设计基于针对NVM优化的数据结构FPTree，实现一个简单的键值存储引擎FPTreeDB。我们通过将其包装成一个调用库，供用户程序使用并管理其数据存储，与LevelDB的使用方式类似。
其对外可用的对数据的基本操作就增删改查：

1. Insert增
2. Remove删
3. Update改
4. Find查

对于系统恢复，我们采取课本介绍的BulkLoading方式。

我们的基本目标是实现上述5大基本操作，使系统能正常运行。系统架构如下：
![FPTreeDB架构](./asset/FPTreeDB.png)

我们预期完成：

1. 单线程版本的FPTree
2. NVM相关编程

## 项目文件说明

```
|__gtest: 为Google Test项目目录，不用管
|__include: 里包含所有用到的头文件
   |__fptree: fptree的头文件所在文件夹
      |__fptree.h: fptree地头文件
   |__utility: fptree所用工具的头文件所在文件夹
      |__utility.h: 指纹计算等工具函数所在头文件
      |__clhash.h: 指纹计算所用哈希函数头文件
      |__p_allocator.h: NVM内存分配器头文件
|__src: 为项目源码所在地，完成里面所有的实现
   |__bin: 可执行文件所在文件夹
      |__main: main.cpp的可执行文件
      |__lycsb: lycsb.cpp的可执行文件
      |__ycsb: ycsb.cpp的可执行文件
   |__fptree.cpp: fptree的源文件，项目核心文件(TODO)
   |__clhash.c: 指纹计算的哈希函数源文件
   |__p_allocator.cpp: NVM内存分配器源文件(TODO)
   |__lycsb.cpp: LevelDB的YCSB测试代码(TODO)
   |__ycsb.cpp: FPTreeDB和LevelDB的YCSB对比测试代码(TODO)
   |__makefile: src下项目的编译文件
|__workloads: 为YCSB测试负载文件，用于YCSB Benchmark测试
   |__数据量-rw-读比例-写比例-load.txt: YCSB测试数据库装载文件
   |__数据量-rw-读比例-写比例-run.txt: YCSB测试运行文件
|__test: 为Google Test用户测试代码所在，请完成编译并通过所有测试
   |__bin: 单元测试可执行文件所在文件夹
      |__fptree_test: fptree_test.cpp的可执行文件
      |__utility_test: utility_test.cpp的可执行文件
   |__fptree_test.cpp: fptree相关测试
   |__utility_test.cpp: PAllocator等相关测试
   |__makefile: gtest单元测试的编译文件
```

## PAllocator

PAllocator中管理三种比较重要的文件，分别为LeafGroup,catelog,freeList，它们的结构如下：

1. LeafGroup结构：| usedNum(8 bytes) | bitmap(n bytes) | Leaf1 | ... | leafN |

2. catelog：| maxFileId(8 bytes) | freeNum(8 bytes) | treeStartLeaf(PPointer) |

3. freeList：| (fId, offset)1, ..., (fId)N |



LeafGroup是数据文件，其文件名用整数表示，从1递增分配即可，规定0为非法标号。PAllocator需要记录分配文件的最大标号，即catalog文件的maxFileId。catalog文件中freeNum为当前可用叶子数，treeStartLeaf为第一个叶子的持久化指针，用于重载树时从其开始，通过链表形式重载。freeList文件每个条目为空叶子的持久化指针，用于启动即可知道可用叶子。

### 各个函数说明：

##### PAllocator::PAllocator() ；

这个函数需要完成的工作是：

- 判断是否创建了catelog,freeList文件
  - 如果创建了上述文件，则从文件中读入数据给变量赋值
  - 如果没有创建上述文件，则创建文件，同时对变量进行初始化
- 执行initFilePmemAddr()

##### void PAllocator::initFilePmemAddr() ;

这个函数完成的工作是：

- 将1~maxFileId-1的LeafGroup文件映射到虚拟地址并将每一组映射存储在fId2PmAddr中

##### PAllocator::~PAllocator() ;

这个函数完成的工作是：

- 退出时将所有的变量作为数据写入文件中
- 将所有变量初始化

##### char* PAllocator::getLeafPmemAddr(PPointer p) ;

这个函数完成的工作是：

- 返回fId2PmAddr对应的虚拟地址加上偏移量

##### bool PAllocator::getLeaf(PPointer &p, char* &pmem_addr)；

这个函数完成的工作是：

- 判断freeNum是否为0
  - 如果为0，调用newLeafGroup()函数
  - 如果不为0，从freeList中分配排在最后面的空闲块并对相应变量进行更改
- 写回磁盘

##### bool PAllocator::ifLeafUsed(PPointer p) ；

这个函数完成的工作是：

- 通过判断bitmap位来判断是否使用过

##### bool PAllocator::ifLeafFree(PPointer p) ；

这个函数完成的工作是：

- return !ifLeafUsed(p);

##### bool PAllocator::ifLeafExist(PPointer p) ；

这个函数完成的工作是：

- 通过判断fId和offset的属性是否超出规则来判断是否合法

##### bool PAllocator::freeLeaf(PPointer p);

这个函数完成的工作是：

- 判断是否p是否存在
  - 如果存在，设置相应的属性并将其加入freeList中，并且设置相应的变量
    - 如果成功，返回true
    - 否则返回false
  - 如果不存在，返回false

##### bool PAllocator::persistCatalog() ;

这个函数完成的工作是：

- 将catelog数据写回到磁盘上

##### bool PAllocator::newLeafGroup() ；

这个函数完成的工作是:

- 分配一个新的LeafGroup并创建一个相应的文件
- 将这个LeafGroup中的所有leaf都加入Freelist中
- 对相关变量进行设置

##### 测试结果如下：

![](./asset/test_result.png)

## fptree

fptree.h中共有四个类，分别是Node,InnerNode,LeafNode,FPtree

其中Node是InnerNode和LeafNode的父类，同时每种Node中都有一个FPtree成员变量，标志着该Node属于哪颗树。

值得注意的是叶子节点的degree是确定的--56，而中间节点的degree是实时赋值的

接下来我将介绍第二阶段完成的任务：插入操作和重载操作

这两个操作需要实现的函数如下：

```c++
// InnerNode构造函数
InnerNode::InnerNode(const int& d, FPTree* const& t, bool _isRoot) {}
// InnerNode析构函数
InnerNode::~InnerNode() {}
// 查找InnerNode中比插入键大的第一个键的位置
int InnerNode::findIndex(const Key& k) {}
// 节点不满直接插入，保持元素有序
void InnerNode::insertNonFull(const Key& k, Node* const& node) {}
// 插入操作
KeyNode* InnerNode::insert(const Key& k, const Value& v) {}
// 插入叶子节点
KeyNode* InnerNode::insertLeaf(const KeyNode& leaf) {}
// 对中间节点进行分裂
KeyNode* InnerNode::split() {}
// LeafNode构造函数
LeafNode::LeafNode(FPTree* t) {}
// LeafNode构造函数
LeafNode::LeafNode(PPointer p, FPTree* t) {}
// LeafNode析构函数
LeafNode::~LeafNode() {}
// 向叶子中插入键值对
KeyNode* LeafNode::insert(const Key& k, const Value& v) {}
// 向判断为未满的叶子节点中插入键值对
void LeafNode::insertNonFull(const Key& k, const Value& v) {}
// 叶子节点分裂(当叶子节点已经满了并且还需要插入时)
KeyNode* LeafNode::split() {}
// 找到叶子节点的中位键并返回
Key LeafNode::findSplitKey() {}
// 获取bitmap中的某一个bit
int LeafNode::getBit(const int& idx) {}
// 数据持久化
void LeafNode::persist() {}
// 重载函数
bool FPTree::bulkLoading() {}
```



这些函数的调用关系如下图所示：

![](./asset/函数依赖.png)

接下来我将介绍几个主要的函数：

**InnerNode::insert()**

这个函数完成的工作是：

- 判断当前节点是否为根节点，以及当前节点键的数量是否为0
  - 如果是查看孩子节点的数量是否为0
    - 如果孩子的数量为0，新开一个叶子节点并对该叶子节点进行未满时插入操作
    - 否则直接进行未满时插入操作。插入后如果该叶子节点满了就进行分裂
- 对孩子节点进行递归调用insert(k,v)操作
- 如果新孩子不为NULL，也即为分裂之后，需要进行相应的设置

**InnerNode::insertNonFull()**

这个函数完成的工作是：

- 向当前中间节点中插入一个keyNode

**InnerNode::split()**

这个函数完成的工作是：

- 判断当前中间节点是否为满的，如果不是报错并返回NULL值
- 将当前节点的右半部分分配到新的节点
- 对相关属性进行设置

**LeafNode::insert()**

这个函数完成的工作是：

- 调用LeafNode::insertNonFull()函数插入键值对
- 判断是否满，如果是进行分裂

**LeafNode::persist()**

这个函数完成的工作是：

- 将当前叶子节点中的数值写回到磁盘中

**LeafNode::insertNonFull()**

这个函数完成的工作是：

- 找到叶子节点中第一个为空的节点，即bitmap相应位置中为0的地址
- 在找到的地址中进行键值对的插入操作

**LeafNode::split()**

这个函数完成的工作是：

- 新开一个叶子节点
- 将键大于中位数的所有键值对移动到新节点上
- 对相应的属性进行设置

**InnerNode::insertLeaf()**

这个函数完成的工作是：

- 对第一个和第二个将要被插入树中的叶子节点进行特殊处理
- 如果下一级并非叶子节点，继续调用insertLeaf(),并对中间节点满的情况进行处理
- 如果下一级是叶子几点，把要插入的叶子节点加入到当前中间节点的孩子中
- 判断根节点是否需要分裂，如果需要则进行相关的操作

insertLeaf()函数将被bulkLoading()函数调用

**FPTree::bulkLoading()**

这个函数完成的工作是：

- 从当前树的第一个叶子节点开始调用inserLeaf()函数，依次将叶子节点插入到树上
- 为每一个要插入到树上的叶子节点设置prev，next指针分别指向前一个叶子节点和后一个叶子节点



第三阶段需要完成的任务是查询和更新操作，对应的函数如下

```c++
// 按照键查找值
Value FPTree::find(Key k) {}
// 按照键查找值（中间节点向下搜索
Value InnerNode::find(const Key& k) {}
// 按照键查找值（在叶子节点中搜索
Value LeafNode::find(const Key& k) {}
// 更新键对应的值
bool FPTree::update(Key k, Value v) {}
// 更新键对应的值（中间节点向下查找并更新
bool InnerNode::update(const Key& k, const Value& v) {}
// 更新键对应的值（在叶子节点中查找并更新
bool LeafNode::update(const Key& k, const Value& v) {}
```

下面我将详细介绍查询操作和更新操作涉及到的函数：

**FPTree::find()**

这个函数完成的工作如下：

- 调用根节点的find()函数

**InnerNode::find()**

这个函数的功能如下：

- 递归向下寻找键所在的叶子节点
- 找到相应叶子节点后调用叶子节点的find()函数

**LeafNode::find()**

这个函数的功能如下：

- 在当前叶子节点中按照键查找bit为1的位置，直到找到键对应的值并返回

**FPTree::update()**

这个函数完成的工作如下：

- 调用根节点的update()函数

**InnerNode::update()**

这个函数的功能如下：

- 递归向下寻找键所在的叶子节点
- 找到相应叶子节点后调用叶子节点的update()函数

**LeafNode::update()**

这个函数功能如下：

- 在叶子节点中按照键查找bit为1的位置，直到找到键对应的位置，并把值修改为新的值



第四阶段需要完成的工作是完成删除操作以及所有剩下TODO，对应的函数如下：

```c++

```

## 完成进度规划

1. 系统说明书，PAllocator实现并通过utility测试，LevelDB的使用以及测试，对应lycsb.cpp，p_allocator.cpp的实现和运行，utility_test.cpp的运行 --- 5/4晚前发布v1版本branch(不会分支的自学)
2. FPTreeDB插入和重载操作并通过相关测试，对应fptree.cpp的实现和fptree_test.cpp部分的运行 --- 5/11晚前发布v2版本branch
3. FPTreeDB查询和更新操作并通过相关测试，对应fptree.cpp的实现和fptree_test.cpp部分的运行 --- 5/18晚前发布v3版本branch
4. FPTreeDB删除操作和所有剩下实现以及测试，对应fptree.cpp的实现和fptree_test.cpp所有的运行 --- 5/31晚前发布final版本branch，作为最后发布版本

以上是基本时间规划，有时间的话会添加多线程版本和micro log的内容。

## 性能测试

关于数据库的性能测试，我们使用类似于ycsb的测试框架来测试FPtree的性能，使用google test来测试数据库实现的正确性。

FPTree的系统结构很类似于leveldb，我们使用ycsb测试leveldb作为例子，后期使用类似的方法用ycsb测试FPTree的性能。ycsb测试leveldb的过程见[lycsb测试leveldb](lycsb测试leveldb.md)。

## 完成进度

阶段一二三四均已经完成，基本实现了fptree的数据结构
