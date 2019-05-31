#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libpmem.h>
#include"utility/p_allocator.h"

using namespace std;

bool check(){
    string path = DATA_DIR + "test";
    ofstream of;
    of.open(path);
    of << "test";
    of.close();
    void * pmem_addr;
    size_t mapped_len;
    int is_pmem;
    pmem_addr = pmem_map_file(path.c_str(), 4,
                PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem);
    

    if(is_pmem){
        cout << "checking if the DATA_DIR is pmem  \033[32m        [pass] \033[0m \n";
    }else{
        cout << "checking if the DATA_DIR is pmem  \033[31m        [error] \033[0m \n";
    }
    return 0;
}

int main(){
    check();
}
