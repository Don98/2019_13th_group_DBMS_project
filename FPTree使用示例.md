## 使用示例说明

#### void FPTree::insert(Key k, Value v)

```c++
#include <libpmem.h>
#include <fptree/fptree.h>

FPtree::FPTree *tree = new FPTree(32); //创建一个新的degree为32的树
tree->insert(10,100);                  //插入(10,100)键值对
FPTree::tree->printTree();             //打印整个树
```

#### bool FPTree::remove(Key k) 

```c++
#include <libpmem.h>
#include <fptree/fptree.h>

...
FPtree::FPTree *tree = new FPTree(32); //创建一个新的degree为32的树
tree->insert(10,100);                  //插入(10,100)键值对
tree->remove(10);                      //删除键为10的键值对
...
```

#### bool FPTree::update(Key k, Value v) 

```c++
#include <libpmem.h>
#include <fptree/fptree.h>

FPtree::FPTree *tree = new FPTree(32); //创建一个新的degree为32的树
tree->insert(10,100);                  //插入(10,100)键值对
tree->update(10,200);                  //将key 10对应的值修改为200
```

#### Value FPTree::find(Key k)

```c++
#include <libpmem.h>
#include <fptree/fptree.h>

FPtree::FPTree *tree = new FPTree(32); //创建一个新的degree为32的树
tree->insert(10,100);                  //插入(10,100)键值对
int num = tree->find(10);              //查找key 10对应的值
```
