#include"fiber.h"
#include"thread.h"
#include"defineLogger.h"
#include"scheduler.h"
namespace sylar{

    static thread_local Fiber*Current_Main_Fiber=nullptr;
    static thread_local Scheduler* Current_Scheduler=nullptr;
    Scheduler::Scheduler(size_t threadPool_size,bool use_caller,const std::string &name):
    name_(name)
    {
        ASSERT(threadPool_size>0);
        if(use_caller){
            ASSERT(GetCurrentSch()==nullptr);
            Current_Scheduler=this;
            --threadPool_size;
            Thread::SetCurrentName(name);
            Fiber::GetCurrentFiber();
            root_Fiber_.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));//
            Current_Main_Fiber=root_Fiber_.get();
   
            main_thread_=GetThreadId();
            threadIds_.push_back(main_thread_);
        }else{
            main_thread_=-1;
        }
        threadPool_size_=threadPool_size;
    }
    Scheduler::~Scheduler(){
        ASSERT(stopping_);
        if(Current_Scheduler==this){
            Current_Scheduler=nullptr;
        }
    }

     const std::string&Scheduler::GetCurrentSchName(){
        return Current_Scheduler->getName();
    }
    Scheduler* Scheduler::GetCurrentSch(){
        return Current_Scheduler;
    }
    Fiber* Scheduler::GetMainFiber(){
        return Current_Main_Fiber;
    }
    void Scheduler::setThis(){
        Current_Scheduler=this;
    }

    std::ostream& Scheduler::dump(std::ostream& os){
        os << "[Scheduler name=" << name_
       << " size=" << threadPool_size_
       << " active_count=" << active_threads_count_
       << " idle_count=" << idle_threads_count_
       << " stopping=" << stopping_
       << " ]" << std::endl << "    ";
        for(size_t i = 0; i < threadIds_.size(); ++i) {
            if(i) {
                os << ", ";
            }
            os << threadIds_[i];
        }
        return os;
    }
     void Scheduler::switchTo(int thread){

     }
     void Scheduler::start(){
      {
         MutexLockGuard Lock(mutex_);
         if(!stopping_){
             return;
         }
         ASSERT(threads_.empty());
         stopping_=false;
         threads_.resize(threadPool_size_);
         for(size_t i=0;i<threads_.size();++i){
             threads_[i].reset(new Thread(std::bind(&Scheduler::run,this),
                                    name_+"_"+std::to_string(i),
                                    NULL)
             );
             threadIds_.push_back(threads_[i]->GetThreadId());
         }
      }

     }

    bool Scheduler::stopping(){
       MutexLockGuard lOCK(mutex_);
        return auto_stop_ && stopping_
        &&  fibers_.empty() && active_threads_count_ == 0;
    }

     void  Scheduler::run(){
        setThis();
        Fiber::GetCurrentFiber();
        if(sylar::GetThreadId()!=main_thread_){
            Current_Main_Fiber=Fiber::GetCurrentFiber().get();
        }
        Fiber::ptr idleFiber(new Fiber(
            std::bind(&Scheduler::idle,this),0
        ));
        Fiber::ptr cb_fider;
        Scheduler::FiberAnThread ft;
        bool tickel_me_=false;
        bool is_activate=false;
        while(true){
            ft.reset();
            {
            BackTraceToString(10);
            MutexLockGuard Lock(mutex_);
                auto it=fibers_.begin();
                while(it!=fibers_.end()){
                    if(it->thread!=-1&&it->thread!=sylar::GetThreadId()){
                        ++it;
                        tickel_me_=true;
                        continue;
                    }
                    ASSERT(it->fiber||it->cb);
                    if(it->fiber&&it->fiber->state_==Fiber::EXEC){
                        ++it;
                        continue;
                    }
                    is_activate=true;
                    ft=*it;
                    ++active_threads_count_;
                    fibers_.erase(it);
                    break;        
                }
            }
            if(tickel_me_){
                tickle();
            }
            if(ft.fiber&&ft.fiber->state_!=Fiber::TERM&&ft.fiber->state_!=Fiber::EXCET){
                ft.fiber->swapIn();
                --active_threads_count_;
                if(ft.fiber->state_==Fiber::READY){
                    scheduler(ft.fiber,ft.thread);
                }else if(ft.fiber->state_!=Fiber::TERM&&ft.fiber->state_!=Fiber::EXCET){
                    ft.fiber->state_=Fiber::HOLD;
                }
            }else if(ft.cb){
                  ft.fiber.reset(new Fiber(
                      ft.cb,0
                  ));
                 ft.fiber->swapIn();
                 --active_threads_count_;
                is_activate=false;
                if(ft.fiber->state_==Fiber::READY){
                    scheduler(ft.fiber,ft.thread);
                }else if(ft.fiber->state_!=Fiber::TERM&&ft.fiber->state_!=Fiber::EXCET){
                    ft.fiber->state_=Fiber::HOLD;
                is_activate=false;
                }
            }else{
                if(is_activate){
                    --active_threads_count_;
                    continue;
                }
                if(idleFiber->state_==Fiber::TERM){
                    break;
                }
                ++idle_threads_count_;
                idleFiber->swapIn();
                --idle_threads_count_;
                if(idleFiber->state_!=Fiber::EXEC&&idleFiber->state_!=Fiber::TERM
                ){
                    idleFiber->state_=Fiber::HOLD;
                }
            }
        }
        //LOG_DEBUG<<"Scheduler::run() exit";
     }
    void Scheduler::stop(){
        auto_stop_=true;

        if(main_fiber_&&(main_fiber_->state_==Fiber::TERM||main_fiber_->state_==Fiber::INIT)&&
        threadPool_size_<=0){
            stopping_=true;
            if(stopping())
            return;
        }

        if(main_thread_==-1){
            ASSERT(GetCurrentSch()!=this);
        }else{
            ASSERT(GetCurrentSch()==this);
        }
        stopping_=true;
        for(int i=0;i<threadPool_size_;++i){
            tickle();
        }
        if(main_fiber_){
            tickle();
        }
        if(main_fiber_){
            if(!stopping()){
                main_fiber_->call();
            }
        }
        std::vector<Thread::ptr>thrs;
        {
            MutexLockGuard Lock(mutex_);
            thrs.swap(threads_);
        }
        LOG_DEBUG<<"scheduler stopping...";
        for(auto &val:thrs){
            val->join();
        }
    }
     void Scheduler::tickle(){
         LOG_DEBUG<<"tickle";
     }

     void Scheduler::idle(){
            LOG_DEBUG<<"ldel";
            while(!stopping()){
                Fiber::YieldToHold();
            }
     }
}