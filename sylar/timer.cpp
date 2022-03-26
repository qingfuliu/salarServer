#include"timer.h"
#include<vector>
#include<functional>
#include"util.h"
#include"defineLogger.h"
namespace sylar{

    Timer::Timer(uint64_t ms,std::function<void()>cb,bool recuring,TimerManager*manager):
    ms_(ms),
    recuring_(recuring),
    cb_(cb),
    manager_(manager)
    {
        next_ms_=GetCurrentMs()+ms_;
    }
    Timer::Timer(uint64_t next){


    }
    Timer::~Timer(){

    }
    bool Timer::cancel(){
        {
            MutexLockGuard Lock(mutex_);
            cb_=nullptr;
            auto it=manager_->timers_->find(shared_from_this());
            if(it!=manager_->timers_->end()){
                manager_->timers_->erase(it);
                return true;
            }
            return false;
        }
    }
    bool Timer::reset(uint64_t ms,bool from_now){
        if(ms==ms_&&!from_now)return false;
        {
            MutexLockGuard Lock(mutex_);
            auto it=manager_->timers_->find(shared_from_this());
            if(cb_==nullptr)return false;
            if(it!=manager_->timers_->end()){
                 {
                    MutexLockGuard Lock2(this->manager_->mutex_);
                    manager_->timers_->erase(it);
                }
                ms_=ms;
                next_ms_=GetCurrentMs()+ms_;
                ASSERT1(cb_!=nullptr,"cb_!=nullptr");
                manager_->addTimer(shared_from_this());
            }
        }
    }

    bool Timer::refresh(){
      {
            MutexLockGuard Lock(mutex_);
            cb_=nullptr;
            auto it=manager_->timers_->find(shared_from_this());
            if(it!=manager_->timers_->end()){
                manager_->timers_->erase(it);
                next_ms_=GetCurrentMs()+ms_;
                manager_->timers_->insert(shared_from_this());
            }
            return false;
        }
    }

    bool Timer::Comparator::operator()(const Timer::ptr&a,const Timer::ptr&b)const{
        if(!a&&!b){
            return false;
        }
        if(!a){
            return true;
        }
        if(!b){
            return false;
        }
        if(a->next_ms_<b->next_ms_){
            return false;
        }
        return a.get()<b.get();
    }


    TimerManager::TimerManager(){
        timers_=std::make_shared<std::set<Timer::ptr,Timer::Comparator>>();
        previous_timr_=GetCurrentMs();
    }
    TimerManager:: ~TimerManager(){

    }
    bool TimerManager::hasTimer(){
        return !timers_->empty();
    }
    Timer::ptr TimerManager::addTimer(uint64_t ms,std::function<void()>cb,bool recuring){
        Timer::ptr timer(new Timer(ms,cb,recuring,this));
         bool at_front;
        {
            MutexLockGuard Lock(mutex_);
            if(!timers_.unique()){
                timers_.reset(new std::set<Timer::ptr,Timer::Comparator>(*timers_));
                LOG_DEBUG<<"copy timers_ in add timers";
            }
            ASSERT1(timers_.unique(),"timers_.unique()");
            auto it=timers_->insert(timer).first;
            at_front=(it==timers_->begin());
        }
        if(at_front){
            onTimerInsertAtFront();
        }
        return timer;
    }

    static void OnTImer(std::weak_ptr<void>weak_pond,std::function<void()>cb){
        std::shared_ptr<void>tep=weak_pond.lock();
        if(tep){
            cb();
        }
    }

    Timer::ptr TimerManager::addContionTimer(uint64_t ms,std::function<void()>cb,std::weak_ptr<void>weak_pond,bool recuring){
        Timer::ptr timer(new Timer(ms,std::bind(OnTImer,weak_pond,cb),recuring,this));

    }

    uint64_t TimerManager::getNextTImer(){
         std::shared_ptr<std::set<Timer::ptr,Timer::Comparator>>it;
        {
            MutexLockGuard lock(mutex_);
            it=timers_;
        }
        if(it->empty()){
            return ~0ull;
        }
        const Timer::ptr &next=*it->begin();
        uint64_t now_ms=GetCurrentMs();
        if(now_ms>next->next_ms_){
            return 0;
        }else{
            return next->next_ms_-now_ms;
        }
    }

    void TimerManager::listExpiredCb(std::vector<std::function<void()>>&cbs_){
        uint64_t  now_ms=GetCurrentMs();
        {
            MutexLockGuard Lock(mutex_);
            std::vector<Timer::ptr>expired;

            if(!timers_.unique())
            timers_.reset(new std::set<Timer::ptr,Timer::Comparator>(*timers_));
            if(timers_->empty())return;

            bool dely=detectClockRollover(now_ms);
            if(!dely&&(*timers_->begin())->next_ms_>now_ms)
                return ;

            Timer::ptr now_timer(new Timer(now_ms));
            std::set<Timer::ptr,Timer::Comparator>::iterator it=
            dely?timers_->end():timers_->upper_bound(now_timer);

            if(it==timers_->begin())return;
            else{
    
                expired.insert(expired.begin(),timers_->begin(),it);

                timers_->erase(timers_->begin(),it);

                cbs_.reserve(expired.size());

                for(auto &val:expired){
                    cbs_.push_back(val->cb_);
                    if(val->recuring_){
                        val->next_ms_=now_ms+val->ms_;
                        timers_->insert(val);
                    }else{
                        val->cb_=nullptr;
                    }
                }
            }
        }
    }
    bool TimerManager::detectClockRollover(uint64_t now_ms){
            bool ans=false;
            if(now_ms<previous_timr_&&
            now_ms<previous_timr_-60*60*1000){
                ans=true;
            }
            previous_timr_=now_ms;
            return ans;
    }
     void TimerManager::onTimerInsertAtFront(){

     }
    void TimerManager::addTimer(Timer::ptr timer){
         bool at_front;
        {
            MutexLockGuard Lock(mutex_);
            if(!timers_.unique()){
                timers_.reset(new std::set<Timer::ptr,Timer::Comparator>(*timers_));
                LOG_DEBUG<<"copy timers_ in addTimer(Timer::ptr timer)";
            }
            ASSERT1(timers_.unique(),"timers_.unique()");
            ASSERT1(timer->cb_!=nullptr,"imer->cb_!=nullptr");
            auto it=timers_->insert(timer).first;
            at_front=(it==timers_->begin());
        }
        if(at_front){
            onTimerInsertAtFront();
        }
    }
}