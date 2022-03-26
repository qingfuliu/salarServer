#include"util.h"
#include<execinfo.h>
#include"defineLogger.h"
#include"fiber.h"
#include<sys/time.h>
namespace sylar
{
    pid_t GetThreadId(){
        return syscall(SYS_gettid);
    }

    uint64_t GetFiberId(){
        return  sylar::Fiber::GetFiberId();
    }

    std::string GetThreadName(){
        return sylar::Thread::GeCurrentName();
    }

    void BackTrace(std::vector<std::string>&strs_,int size,int skip){
        void** info=(void**)malloc(sizeof(void*)*size);
        size_t s=backtrace(info,size);
        char**infoSring=backtrace_symbols(info,s);
        if(infoSring==NULL){
            LOG_ERROR<<"BackTrace_backtrace_symbols Error...";
            return;
        }
        for(size_t i=skip;i<s;++i){
            strs_.push_back(infoSring[i]);
        }
        free(infoSring);
    }

    std::string BackTraceToString(int size,int skip,std::string prefix){
        std::vector<std::string>strs_;
        BackTrace(strs_,size,skip);
        std::stringstream ss;
        for(size_t i=0;i<strs_.size();++i){
            ss<<prefix<<strs_[i]<<std::endl;
        }
        return ss.str();
    }

    uint64_t GetCurrentMs(){
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000ul  + tv.tv_usec / 1000;
    }
} // namespace name
