#include "utility/microlog.h"
#include <fstream>


MicroLog::MicroLog(const string path) {
    init(path);

    char* pmem_addr;
    size_t mapped_len;
    int is_pmem;

    pmem_addr = (char*)pmem_map_file(path.c_str(), sizeof(PPointer) * 2,
                            PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem);
    if (pmem_addr == NULL) {
        printf("error: open log %s fail\n", path.c_str());
        exit(1);
    }
    this->is_pmem = is_pmem;
    this->first = (PPointer*) pmem_addr;
    this->second = this->first + 1;
}

void MicroLog::init(const string path) {
    ifstream fin(path, ios::in|binary);
    if (fin) {
        fin.close();
        return;
    }
    ofstream fout(path, ios::out|ios::binary);
    PPointer invalidp;
    invalidp.fileId = 0;
    invalidp.offfset = 0;
    fout.write((char*)&invalidp, sizeof(PPointer));
    fout.write((char*)&invalidp, sizeof(PPointer));
    fout.close();
}

bool MicroLog::isUsed() {
    PPointer invalidp;
    invalidp.fileId = 0;
    invalidp.offfset = 0;
    return *first == invalidp;
}

bool MicroLog::isFree() {
    return !isUsed();
}

bool MicroLog::logCurPointer(PPointer p) {
    *first = p;
    persist((void*) first, sizeof(PPointer));
    return true;
}

bool MicroLog::logChangePointer(PPointer p) {
    *second = p;
    persist((void*) second, sizeof(PPointer));
    return true;
}

PPointer MicroLog::getCurPointer() {
    return *first;
}

PPointer MicroLog::getChangePointer() {
    return *second;
}

void MicroLog::reset() {
    PPointer invalidp;
    invalidp.fileId = 0;
    invalidp.offfset = 0;
    *first = invalidp;
    *second = invalidp;
    persist((void*)first, sizeof(PPointer) * 2);
}

void MicroLog::persist(void* pointer, int len) {
    if (is_pmem) {
        pmem_persist(pointer, len);
    } else {
        pmem_msync(pointer, len);
    }
}

MicroLog::~MicroLog() {
    pmem_unmap((void*) first, sizeof(PPointer) * 2);
}