#include <leveldb/db.h>
#include <string>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <time.h>

#define KEY_LEN 8
#define VALUE_LEN 8
using namespace std;

const string workload = "../workloads/";

const string load = workload + "220w-rw-50-50-load.txt"; // TODO: the workload_load filename
const string run  = workload + "220w-rw-50-50-run.txt"; // TODO: the workload_run filename

const string filePath = "/tmp/testdb";

const int READ_WRITE_NUM = 350000; // TODO: how many operations

int main()
{        
    leveldb::DB* db;
    leveldb::Options options;
    leveldb::WriteOptions write_options;
    // TODO: open and initial the levelDB

    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, filePath, &db);
    if (!status.ok()) {
        cerr << status.ToString() << endl;
        exit(1);
    }

    uint64_t inserted = 0, queried = 0, t = 0;
    uint64_t* key = new uint64_t[2200000]; // the key and value are same
    bool* ifInsert = new bool[2200000]; // the operation is insertion or not
    FILE *ycsb_load, *ycsb_run; // the files that store the ycsb operations
    char *buf = NULL;
    size_t len = 0;
    struct timespec start, finish; // use to caculate the time
    double single_time; // single operation time

    printf("Load phase begins \n");
    // TODO: read the ycsb_load and store
    ycsb_load = fopen(load.c_str(), "r");
    if (ycsb_load == NULL) {
        printf("read load file failed\n");
        exit(1);
    } else {
        printf("read load file success\n");
    }
    char op[7];  
    uint64_t k;
    while (fscanf(ycsb_load, "%s %lu\n", op, &k) != EOF) {
        key[t] = k;
        if (strcmp("INSERT", op) == 0) ifInsert[t] = true;
        else ifInsert[t] = false;
        t++;
    }
    fclose(ycsb_load);

    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: load the workload in LevelDB
    len = 8;
    for (int i = 0; i < t; ++ i) {
        status = db->Put(write_options, leveldb::Slice((char*) &key[i], len), leveldb::Slice((char*) &key[i], len));
        if (!status.ok()) cout << key[t] << " insert failed" << endl;
        else inserted++;
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);
    single_time = (finish.tv_sec - start.tv_sec) * 1000000000.0 + (finish.tv_nsec - start.tv_nsec);

    printf("Load phase finishes: %d items are inserted \n", inserted);
    printf("Load phase used time: %fs\n", single_time / 1000000000.0);
    printf("Load phase single insert time: %fns\n", single_time / inserted);

    // leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    // for (it->SeekToFirst(); it->Valid(); it->Next()) {
    //     printf("key: %lld, value: %lld\n", *((uint64_t*)(it->key().data())),*((uint64_t*)(it->value().data())));
    // }

    int operation_num = 0;
    inserted = 0;       

    // TODO:read the ycsb_run and store
    t = 0;
    ycsb_run = fopen(run.c_str(), "r");
    if (ycsb_run == NULL) {
        printf("read run file failed\n");
        exit(1);
    } else {
        printf("read run file success\n");
    }
    while (fscanf(ycsb_run, "%s %lu\n", op, &k) != EOF) {
        key[t] = k;
        if (strcmp("INSERT", op) == 0) ifInsert[t] = true;
        else ifInsert[t] = false;
        t++;
    }
    fclose(ycsb_run);

    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: operate the levelDB
    len = 8;
    string val;
    for (int i = 0; i < t; ++ i) {
        operation_num++;
        if (ifInsert[i]) {
            status = db->Put(write_options, leveldb::Slice((char*) &key[i], len), leveldb::Slice((char*) &key[i], len));
            if (!status.ok()) cout << key[t] << " insert failed" << endl;
            else inserted++;
        } else {
            status = db->Get(leveldb::ReadOptions(), leveldb::Slice((char*) &key[i], len), &val);
            if (!status.ok()) cout << key[t] << " read failed" << endl;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);
    single_time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    // printf("Run phase finishes: %d/%d items are inserted/searched\n", operation_num - inserted, inserted);
    // printf("Run phase throughput: %f operations per second \n", READ_WRITE_NUM/single_time);

    printf("Run phase finishes: %d/%d items are inserted/searched\n", inserted, operation_num - inserted);
    printf("Run phase throughput: %f operations per second \n", operation_num/single_time);    
    return 0;
}
