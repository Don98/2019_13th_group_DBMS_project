# 系统说明书

fptree是一个键值存储系统，系统的实现参考leveldb。

leveldb是google的一个键值存储系统，github仓库见https://github.com/google/leveldb

FPTree是Oukid提出的一种适用于SCM(Storage Class Memory)的一类新型的B树，详见前期工作与论文阅读总结文件夹下的Oukid_FPTree.pdf论文。

fptree的实现源码见Programming-FPTree目录下的src和include目录，系统的设计见[Programming-FPTree目录下的READMD](./Programming-FPTree/README.md)，更为详细的代码说明见[Programming-FPTree目录下的函数实现](./Programming-FPTree/函数实现.md)

## 注意事项

- 确保已经安装了pmdk
- **数据存储位置默认为/mnt/mem，如果需要修改请进入utility.h文件中修改DATA_DIR**
- 确保数据存储的位置是持续性内存

更多详细内容请参考[注意事项详细版本](./准备工作.md)

## FPTree 编译与安装

### 下载

```
git clone https://github.com/Don98/2019_13th_group_DBMS_project.git
```

### 编译安装

```
cd 2019_13th_group_DBMS_project/Programming-FPTree/src
git fetch && git checkout final && git pull 
# 生成动态链接库以及静态链接库
make install                                         
```

## 检测

```
# 检测utility.h中定义的DATA_DIR是否为持续性内存
# 检测是否存在需要的libpmem库
# 检测动态链接库是否生成并生效
make check                                            
```

### 性能测试

测试前注意将/mnt/mem目录设为pmem目录路径

具体说明见[fptree性能测试](fptree性能测试.md)
先进入Programming-FPTree/src目录，测试命令为

```
sudo make testfptree
```

### google test 测试

测试前注意将/mnt/mem目录设为pmem目录路径

先进入Programming-FPTree/src目录，测试命令为(注意sudo)

```
make
sudo ./bin/utility_test
sudo ./bin/fptree_test
```

## 完成进度

- [x] 系统说明书，PAllocator实现并通过utility测试，LevelDB的使用以及测试(发布v1版本branch)
- [x] FPTreeDB插入和重载操作并通过相关测试(发布v2版本branch)
- [x] FPTreeDB查询和更新操作并通过相关测试(发布v3版本branch)
- [x] FPTreeDB删除操作和所有剩下实现以及测试(发布final版本branch)
- [ ] 实现原始FPTree的micro-log机制(发布版本micro-log)
- [ ] 实现多线程版本的FPTree(发布版本CFPTree)
