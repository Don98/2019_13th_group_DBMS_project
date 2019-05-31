# fptree的数据可靠性

fptree提供了micro-log机制来保证数据库故障时叶子的分配与释放的可靠性，通过设计函数的执行顺序来保证数据库故障时内部数据的一致性（即不会出现数据库同一条目部分数据改了而另外一部分数据没改从而导致脏数据现象）。由于数据库的数据只存放于叶子节点，内部节点都是动态生成，所以只需要保证叶子节点不会出现脏数据即可，由于数据只会在写操作时更改，以下只对叶子的写操作进行分析。

## 单线程版本的故障恢复

### micro-log 设计
micro-log中每一个log包含两个数据，更改前的PPointer（称为PCurrentLeaf）和需要更改（新增或删除）的PPointer(称为PChangeLeaf)。
单线程版本只包含两个全局log，即一个split-log用于记录叶子分裂节点时的变化，一个remove-log用于记录叶子删除节点时的变化。

### 数据可靠性说明及故障处理

#### 插入键值对的可靠性
插入键值对的流程如下：
1. 插入键值对
2. 插入fingerprint
3. 持久化键值对和fingerprint
4. 修改bitmap对应的位
5. 持久化bitmap

在第4步（含）前发生故障，由于bitmap还没持久化，数据插入失败，数据库没有脏数据
在第五步时如果发生故障，则持久化bitmap不成功，没有脏数据，若持久化成功，则持久化bitmap成功，此时数据已经完整地插入到数据库里，不需要故障恢复。

#### 更新键值对的可靠性
插入键值对的流程如下：
1. 找到叶子节点中要更新的键值对的位置
2. 更新键值对的值
3. 持久化键值对

在没有持久化键值对的值之前发生故障，则更新不成功，若持久化键值对后发生故障，则更新成功，两种情况都没有脏数据。

#### 删除键值对的可靠性
删除键值对的流程如下：
1. 找到叶子节点中要删除的键值对的bitmap的位置
2. 置bitmap对应的位为0
3. 持久化bitmap

在没有持久化bitmap之前发生故障，则删除不成功，若持久化bitmap后发生故障，则删除成功，两种情况都没有脏数据。

#### 分裂叶子节点时的可靠性
分裂发生在插入键值对后叶子的键值对已经达到等于2 * LEAF_DEGREE的数目（即已满）时。

分裂的流程如下：

1. split-log持久化要分裂的节点的PPointer
2. pallocator分配新节点
3. split-log持久化新节点的PPointer
4. 寻找split-key，并将新叶子应该有的的键值对和fingerprint从旧叶子复制到新叶子节点，同时把新叶子的bitmap置为正确值。
5. 新叶子的pNext等于旧叶子的pNext
6. 持久化新叶子
7. 旧叶子的pNext等于新叶子的PPinter
8. 旧叶子的bitmap等于新叶子的bitmap的反
9. 持久化旧叶子
10. 重置split-log


数据恢复流程：
1. 如果split-log的PCurrentLeaf为空，则没有故障或故障发生在分裂的第1步前，此时新叶子还没分配，无需故障恢复，直接返回。
2. 如果split-log的PNewLeaf为空则在第3步前发生故障，此时直接重置micro-log（注1）
3. 如果PCurrentLeaf的bitmap满了，则是第8步前发生了故障，数据从旧叶子复制到新叶子的过程失败，从分裂的第4步开始恢复
4. 否则是在第8步时发生故障，旧叶子的数据已经正确，从第8步开始恢复

注1：
bug: 如果在p_allocator分配完叶子节点但还没有叶子节点的PPointer还没有被split-log记录时在这里检测不出来，TODO。

#### 删除叶子节点时的可靠性
删除叶子节点发生在删除键值对后叶子节点没有键值对时。
p_alloctor释放叶子时会检测叶子节点是不是第一个叶子节点，如果是则会更新第一个叶子节点为将要删除的叶子节点的下一个节点。
p_allocator会在返回前才持久化对应leaf_group的bitmap。


删除的流程如下：
1. remove-log持久化要删除的节点的PPointer(节点记为PCurrentLeaf)
2. remove-log持久化要删除节点的前一个节点的PPointer(节点记为PPrevLeaf)
3. PPrevLeaf.Next = PCurrent.Next
4. 持久化PrevLeaf.Next
5. 释放PCurrentLeaf
6. 重置remove-log


数据恢复流程：
1. 如果remove-log的PCurrentLeaf为空，则没有故障或故障发生在删除的第1步前，无需故障恢复，直接返回。
2. 如果remove-log的PPrevLeaf不为空则在第3第4步时发生故障，此时重新更改PPrevLeaf的pNext并持久化。
3. 重新释放PCurrentLeaf
4. 重置remove-log


### 单线程的数据可靠性总结
本版本的fptree能够保证当故障发生在fptree的代码行运行时能够进行合理的数据恢复，但当故障发生在p_alloctor的代码行运行时可能会无法记录故障，可靠性有部分保证，待更新。


## 多线程版本的故障恢复
TODO

