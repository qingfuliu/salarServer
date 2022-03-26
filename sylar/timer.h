#ifndef  __TIME__H__
#define __TIME__H__
#include<set>
#include<unistd.h>
#include<functional>
#include<memory>
#include<vector>
#include"thread.h"
namespace sylar{
    class TimerManager;
    class Timer : public std::enable_shared_from_this<Timer>{
        friend class TimerManager;
        public:
        typedef std::shared_ptr<Timer> ptr;
        bool cancel();
        //重新设置时间
        bool reset(uint64_t ms,bool from_now);
        bool refresh();
        Timer(uint64_t ms,std::function<void()>cb,bool recuring,TimerManager*manager);
        Timer(uint64_t next);
        ~Timer();
        private:
        uint64_t ms_=0;
        uint64_t next_ms_=0;
        bool recuring_=false;
        std::function<void()>cb_;
        TimerManager*manager_=nullptr;
        MutexLock mutex_;
        private:
        struct  Comparator
        {
            bool operator()(const Timer::ptr&a,const Timer::ptr& b)const;
        };
        
    };

    class TimerManager{
    friend class Timer;
    public:
        TimerManager();
        virtual ~TimerManager();
        Timer::ptr addTimer(uint64_t ms,std::function<void()>cb,bool recuring=false);
        Timer::ptr addContionTimer(uint64_t ms,std::function<void()>cb,std::weak_ptr<void>ptr,bool recuring=false);
        uint64_t getNextTImer();
        void listExpiredCb(std::vector<std::function<void()>>&cbs_);
        bool detectClockRollover(uint64_t now_ms);
    protected:
        bool hasTimer();
        virtual void onTimerInsertAtFront()=0;
        void addTimer(Timer::ptr timer);
    private:
        MutexLock mutex_;
        std::shared_ptr<std::set<Timer::ptr,Timer::Comparator>>timers_;
        uint64_t previous_timr_;
        bool tickle_=false;
    };
}


#endif