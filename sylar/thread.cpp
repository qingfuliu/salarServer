#include"thread.h"
#include<pthread.h>
#include"defineLogger.h"
#include<assert.h>
namespace sylar{
//--------------------信号量---------------------------//
   Semaphore::Semaphore(int count){
       if(sem_init(&sem_,0,count)){
           
           LOG_ERROR<<"sem_init error";
           throw std::logic_error("sem_init error");
       }
   }
    Semaphore::~Semaphore(){
        sem_destroy(&sem_);
    }
    void Semaphore::wait(){
            if(sem_wait(&sem_)){
           LOG_ERROR<<"sem_wait error";
            throw std::logic_error("sem_wait error");
            }
    }
    void Semaphore::notify(){
            if(sem_post(&sem_)){
            LOG_ERROR<<"sem_post error";
            throw std::logic_error("sem_post error");
            }
    }

//--------------------线程----------------------------//
static thread_local Thread* CurrentThread=nullptr;
static thread_local std::string CurrentThreadName="UNKNOW";
    Thread* Thread::GetCurrentTherad(){
        return CurrentThread;
    }
    std::string Thread::GeCurrentName(){
        return CurrentThreadName;
    }
     void Thread::SetCurrentName(const std::string &name){
         if(CurrentThread){
             CurrentThread->setName(name);
         }
         CurrentThreadName=name;
    }
    Thread::~Thread(){
        if(threadId_){
            pthread_detach(threadId_);
        }
    }
    Thread::Thread(std::function<void()>cb,const std::string &name,void*arg){
        this->arg=arg;
        CurrentThread=this;
        if(!name.empty())
        {
            CurrentThreadName=name;
            name_=name;
        }
        cb_=cb;
        int rt=pthread_create(&threadId_,NULL,&Thread::run,this);
        if(rt){
            LOG_ERROR<<"Thread create error:  name="<<name;
        }
       semaphore_.wait();
    }
    void Thread::join(){
        if(threadId_){
            int rt=pthread_join(threadId_,NULL);
            if(rt){
                LOG_ERROR<<"Thread join error:  name="<<GeCurrentName()<<" id="<<name_;
            }
        }
        threadId_=0;
    }
    void * Thread::run(void*arg){
            Thread* thread_=static_cast<Thread*>(arg);
            //CurrentThread=thread_;
            Thread::SetCurrentName(thread_->name_);
           // LOG_DEBUG<<thread_->GetThreadId()<<Thread::GeCurrentName()<<"  running....";
            pthread_setname_np(thread_->threadId_,GeCurrentName().substr(0,15).c_str());
            std::function<void()>cb;
            cb.swap(thread_->cb_);
            thread_->semaphore_.notify();
            cb();
            //LOG_DEBUG<<thread_->GetThreadId()<<"exit....";
            return 0;
    }
    //---------------------互斥锁-------------------------//
    MutexLock::MutexLock(){
        pthread_mutex_init(&mutex_,NULL);
    }
    MutexLock::~MutexLock(){
        assert(holder_==0);
        pthread_mutex_destroy(&mutex_);
    }
    void MutexLock::lock(){
        pthread_mutex_lock(&mutex_);
        holder_=pthread_self();
    }
    void MutexLock::unlock(){
        holder_=0;
        pthread_mutex_unlock(&mutex_);
    }
    
    MutexLockGuard::MutexLockGuard(MutexLock&mutexLock)
    :mutexLock_(mutexLock)
    {
        mutexLock_.lock();
    }
    MutexLockGuard::~MutexLockGuard(){
        mutexLock_.unlock();
    }

    RWMutex::RWMutex(){
        pthread_rwlock_init(&rwMutex_,NULL);
    }
    RWMutex::~RWMutex(){
        pthread_rwlock_destroy(&rwMutex_);
    }
    void RWMutex::wrlock(){
        pthread_rwlock_wrlock(&rwMutex_);
    }
    void RWMutex::rdlock(){
        pthread_rwlock_rdlock(&rwMutex_);
    }
    void RWMutex::unlock(){
        pthread_rwlock_unlock(&rwMutex_);
    }
}