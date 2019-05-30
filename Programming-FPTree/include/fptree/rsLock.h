#include<memory.h>
#include<iostream>
#include<stdlib.h>
#include<mutex>

class RSLock {
protected:
        /*
    lock : read wrt mutex wrt_mutex
           read : readerCount
           wrt  : writerCOunt
    */
    std::mutex read;  // 修改readerCount的锁
    int readerCount;  // 记录此结点读者的数量
    std::mutex wrt;   // 修改writerCount的锁
    int writerCount;  // 记录此结点写者的数量
    std::mutex mut;   // 全局锁，抢到这个锁之后直到写者数量为零之后放弃。
    std::mutex wmut;  // 写者互斥锁
    std::mutex top;

public:
    RSLock(){
        this->writerCount = 0;
        this->readerCount = 0;
    }
    void top_lock(){
        top.lock();
    }
    void top_rele(){
        top.unlock();
    }
    void read_lock(){
        this->top_lock();
        this->read.lock();
        this->readerCount++;
        if(this->readerCount == 1)
            this->mut.lock();
        this->read.unlock();
        this->top_rele();
    }
    void read_rele(){        
        this->read.lock();
        this->readerCount--;
        if(readerCount == 0)
            this->mut.unlock();
        this->read.unlock();
    }
    void write_lock(){
        this->wrt.lock();
        this->writerCount++;
        if(this->writerCount == 1){
            this->top_lock();
            this->mut.lock();
        }
        this->wrt.unlock();
        this->wmut.lock();
    }
    void write_rele(){
        this->wmut.unlock();
        this->wrt.lock();
        this->writerCount--;
        if(this->writerCount == 0){
            this->mut.unlock();
            this->top_rele();
        }
        this->wrt.unlock();
    }

};