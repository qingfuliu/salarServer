#ifndef _THREAD_H_
#define _THREAD_H_
#include<functional>
#include<string.h>
#include<pthread.h>
#include<memory>
#include<linux/sem.h>
#include<semaphore.h>
#include<stdint.h>
namespace sylar{

class Semaphore{
public:
    Semaphore(int count=1);
    ~Semaphore();
    void wait();
    void notify();
    
private:
    sem_t sem_;
};

class Thread{
public:
    typedef std::shared_ptr<sylar::Thread> ptr;
    Thread();
    Thread( std::function<void()>cb,const std::string &name,void*arg=NULL);
    ~Thread();
    std::string& GetName(){return name_;}
    void setName(const std::string &name){this->name_=name;}
    pthread_t GetThreadId(){return threadId_;}
    static Thread* GetCurrentTherad();
    static  std::string GeCurrentName();
    static void SetCurrentName(const std::string &name);
     void join();
private:
    Thread&operator=(const Thread&)=delete;
    Thread(const Thread&)=delete;
    Thread(const Thread&&)=delete;
    static void*run(void*);
private:
    pid_t id_=-1;
    std::string name_;
    pthread_t threadId_=0;
    std::function<void()>cb_;
    void*arg;    
    Semaphore semaphore_{0};
};

//---------------------互斥锁-------------------------//
class MutexLock{
    public:
    MutexLock();
    ~MutexLock();
    void lock();
    void unlock();
    private:
    pthread_mutex_t mutex_;
    pid_t holder_=0;
}; 
class MutexLockGuard{
public:
    MutexLockGuard(MutexLock&mutexLock);
    ~MutexLockGuard();
private:
    MutexLock&mutexLock_;
};

template<class T>
class RDMutexLockGuard{
public:
    RDMutexLockGuard(T&rwMutex):
    rwMutex_(rwMutex)
    {
    rwMutex_.rdlock();
    }
    ~RDMutexLockGuard(){
    rwMutex_.unlock();
    }
private:
    T&rwMutex_;
};

template<class T>
class WRMutexLockGuard{
public:
    WRMutexLockGuard(T&rwMutex):
    rwMutex_(rwMutex)
    {
    rwMutex_.wrlock();
    }
    ~WRMutexLockGuard(){
    rwMutex_.unlock();
    }
private:
    T&rwMutex_;
};

class RWMutex{
public:
    typedef WRMutexLockGuard<RWMutex> WrLockGuard;
    typedef RDMutexLockGuard<RWMutex> ReLockGuard;
    RWMutex();
    ~RWMutex();
    void wrlock();
    void rdlock();
    void unlock();
private:
    pthread_rwlock_t rwMutex_;
    pid_t holder_;
} ;

}

#endif