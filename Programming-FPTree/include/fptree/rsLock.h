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

    bool ifWlock;
    bool ifRlock;
    bool ifTlock;
    bool ifGiveUp;
public:
    RSLock(){
        this->writerCount = 0;
        this->readerCount = 0;
        this->ifWlock = false;
        this->ifRlock = false;
        this->ifTlock = false;
        this->ifGiveUp = true;
    }
    void top_lock(){
        top.lock();
        this->ifTlock = true;
    }
    void top_rele(){
        top.unlock();
        this->ifTlock = false;
    }
    void read_lock(){
        this->top_lock();
        this->read.lock();
        this->readerCount++;
        if(this->readerCount == 1)
            this->mut.lock();
        this->read.unlock();
        this->top_rele();
        this->ifRlock = true;
    }
    void read_rele(){        
        this->read.lock();
        this->readerCount--;
        if(readerCount == 0)
            this->mut.unlock();
        this->read.unlock();
        this->ifRlock = false;
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
        this->ifWlock = true;
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
        this->ifWlock = false;
    }
    bool ifWLock(){
        return this->ifWlock;
    }
    bool ifRLock(){
        return this->ifRlock;
    }
    bool ifTLock(){
        return this->ifTlock;
    }
    int get_write(){
        return this->writerCount;
    }
    int get_read(){
        return this->readerCount;
    }
    void setGiveUp(bool value){
        this->ifGiveUp = value;
    }
    bool getGiveUp(){
        return this->ifGiveUp;
    }

};