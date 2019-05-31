# 系统说明书

fptree是一个键值存储系统，系统的实现参考leveldb。

leveldb是google的一个键值存储系统，github仓库见https://github.com/google/leveldb

FPTree是Oukid提出的一种适用于SCM(Storage Class Memory)的一类新型的B树，详见前期工作与论文阅读总结文件夹下的Oukid_FPTree.pdf论文。

fptree的实现源码见Programming-FPTree目录下的src和include目录，系统的设计见[Programming-FPTree目录下的READMD](./Programming-FPTree/README.md)，更为详细的代码说明见[Programming-FPTree目录下的函数实现](./Programming-FPTree/函数实现.md)

## 特性
- 键值对均为8字节大小定长
- 非叶节点使用B树结构，键有序存放
- 叶子节点使用fptree结构，键值对无序存放
- 基础操作为find(key), insert(key, value),update(key, value),remove(key)
- 本版本同步于final分支,实现了fptree的所有基本操作并能通过google_test
- 提供编译命令生成静态链接库与动态链接库，可以在外部程序使用静态链接或动态链接调用

## CFPTree版本特性

- 实现了多线程同步的CFPTree
- 可以让多个线程对fptree进行操作，模拟的是多个用户在对远程数据库进行修改、查询等访问过程
- 考虑到数据库需要尽快完成对于数据的更新，我们在这里实现的是使用写者优先方式对资源进行分配、获取。

### 多线程的实现方案

- 详细实现方案[点击](https://github.com/Don98/2019_13th_group_DBMS_project/blob/CFPTree/Programming-FPTree/%E5%A4%9A%E7%BA%BF%E7%A8%8B%E7%9A%84%E5%AE%9E%E7%8E%B0%E6%96%B9%E6%A1%88%E8%A7%A3%E9%87%8A.md)

## 注意事项

- 确保已经安装了pmdk
- **数据存储位置默认为/mnt/mem**，如果需要修改请进入Programming-FPTree/src/utility/utility.h文件中修改DATA_DIR
- 确保数据存储的位置是持久性内存
- Programming-FPTree/src/utility/utility.h的LEAF_DEGREE常量是叶子默认的度的大小，默认是56，叶子可以容纳的键值对数目m为0 < m < LEAF_DEGREE * 2,由于缓存对齐会提高性能，建议将其修改为21，此时叶子的头部大小为（6byte bitmap + 16byte + (21*2)byte fingerprints = 64byte），是一个cache-line的大小

更多详细内容请参考[模拟NVM环境与pmdk安装](./模拟NVM环境与pmdk安装.md)

## FPTree 安装

### 下载

```
git clone https://github.com/Don98/2019_13th_group_DBMS_project.git
```

### 编译安装
安装后会在Programming-FPTree/src/lib生成libfptree.a和libfptree.so文件，并将fptree的头文件和动态链接文件放入用户库中，安装命令为：
```
cd 2019_13th_group_DBMS_project/Programming-FPTree/src
git fetch && git checkout final && git pull 
# 生成动态链接库以及静态链接库
make install                                         
```

### 检测
检测时需要在Programming-FPTree/src目录下
```
# 检测utility.h中定义的DATA_DIR是否为持续性内存
# 检测是否存在需要的libpmem库
# 检测动态链接库是否生成并生效
make check                                            
```

## FPTree测试

### 性能测试

测试前注意设置并挂载DATA_DIR为pmem目录

具体说明见[fptree性能测试](fptree性能测试.md)
先进入Programming-FPTree/src目录，测试命令为

```
sudo make testfptree
```

### google test 测试

测试前注意设置并挂载DATA_DIR为pmem目录

先进入Programming-FPTree/src目录，测试命令为(注意sudo)

```
make
sudo ./bin/utility_test
sudo ./bin/fptree_test
sudo ./bin/CFPTree_test
```

## FPTree使用
详见[FPTree使用示例](FPTree使用示例.md)

## 完成进度

- [x] 系统说明书，PAllocator实现并通过utility测试，LevelDB的使用以及测试(发布v1版本branch)
- [x] FPTreeDB插入和重载操作并通过相关测试(发布v2版本branch)
- [x] FPTreeDB查询和更新操作并通过相关测试(发布v3版本branch)
- [x] FPTreeDB删除操作和所有剩下实现以及测试(发布final版本branch)
- [x] 实现单线程原始FPTree的micro-log机制(发布版本micro-log)
- [x] 实现多线程版本的FPTree(发布版本CFPTree)
- [ ] 实现多线程版本带micro-log的FPTree
