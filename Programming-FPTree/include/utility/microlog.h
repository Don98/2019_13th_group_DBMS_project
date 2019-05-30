#ifndef MICROLOG_H
#define MICROLOG_H
#include <string>
#include <map>
#include <libpmem.h>
#include <fstream>
#include "utility/utility.h"

const string SPLIT_LOG_PATH = DATA_DIR + "split.log";
const string REMOVE_LOG_PATH = DATA_DIR + "remove.log";

using std::string;

class MicroLog {
private:
    PPointer* first;
    PPointer* second;
    bool is_pmem;
public:
    MicroLog(const string path);
    void init(const string path);
    bool isFree();
    bool isUsed();
    bool logCurPointer(PPointer p);
    bool logChangePointer(PPointer p);
    PPointer getCurPointer();
    PPointer getChangePointer();
    void reset();
    void persist(void* pointer, int len);
    ~MicroLog();
};

#endif