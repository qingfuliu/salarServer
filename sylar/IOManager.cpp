#include"IOManager.h"
#include"defineLogger.h"
#include<sys/epoll.h>
#include<sys/fcntl.h>
#include<unistd.h>
#include<string.h>
#include"timer.h"
namespace sylar{
        IOManager::FdContex::FdEvent& IOManager::FdContex::getEventConetx(Event event){
            ASSERT(event&_event);
            switch(event){
                case  Event::READD:
                return _read;
                case Event::WRITE:
                return _write;
                default:
                 ASSERT1(false,"getEventConetx fault");
            }
            throw std::invalid_argument("event is a invaild argument");
        }
         void IOManager::FdContex::triggerEvent(Event event){
            ASSERT(event&_event);
            FdEvent& fd_event=getEventConetx(event);
            _event=(Event)(_event&~event);

            if(fd_event._cb){
                fd_event.scheduler->scheduler(fd_event._cb);
            }else if(fd_event._fiber){
                fd_event.scheduler->scheduler(fd_event._fiber);
            }
            fd_event.scheduler=nullptr;
            return;
      }
     void IOManager::FdContex::resetContex(FdEvent&fd_event){
            fd_event.scheduler=nullptr;
            fd_event._cb=nullptr;
            fd_event._fiber.reset();
        }
    IOManager::IOManager(size_t threadPool_size_,bool use_caller,const std::string &name):
    Scheduler(threadPool_size_,use_caller,name){
        ep_fd_=epoll_create(5000);
        if(ep_fd_<0){
            LOG_DEBUG<<"epoll_create error int IOManager...";
            abort();
            return;
        }
        epoll_event event;
        memset(&event,0,sizeof(event));
        event.events=EPOLLIN | EPOLLET;
        event.data.fd=tickle_fd_[0];

        int rt=pipe(tickle_fd_);
        ASSERT1(rt!=-1,"pipe in IOManager");

        rt=fcntl(tickle_fd_[0],F_SETFD,O_NONBLOCK);
        ASSERT1(rt!=-1,"fcntl in IOManager");

        rt=epoll_ctl(ep_fd_,EPOLL_CTL_ADD,tickle_fd_[0],&event);

        start();
    }
    IOManager::~IOManager(){
        LOG_DEBUG<<"IOManager stop...";
        stop();
        close(ep_fd_);
        close(tickle_fd_[0]);
        close(tickle_fd_[1]);

        for(size_t i=0;i<fd_Contexs_.size();++i){
            if(fd_Contexs_[i]){
                delete fd_Contexs_[i];
            }
        }
    }

    void IOManager::fd_contex_resize(size_t size){
        fd_Contexs_.resize(size);
        for(size_t i=0;i<fd_Contexs_.size();++i){
            fd_Contexs_[i]=new FdContex;
            fd_Contexs_[i]->_fd=i;
        }
    }
    bool IOManager::addEvent(int fd,Event event,std::function<void()>cb){
        FdContex*fd_contex;
        {
             MutexLockGuard Lock(mutex_);
            if((int)fd_Contexs_.size()<=fd){
            fd_contex_resize(fd*1.5);
            }
            fd_contex=fd_Contexs_[fd]; 
        }
        MutexLockGuard Lock(fd_contex->_mutex);
        if(fd_contex->_event&event){
            LOG_DEBUG<<"addEvent filed ! fd="<<fd_contex->_fd<<"   in addEvent";
            return false;
        }

        int op=fd_contex->_event?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
        epoll_event epevent;
        epevent.data.ptr=fd_contex;
        epevent.events=EPOLLET|event|fd_contex->_event;

        int rt=epoll_ctl(ep_fd_,op,fd_contex->_fd,&epevent);
        if(rt<0){
            LOG_DEBUG<<"addEvent filed ! fd="<<fd_contex->_fd<<"   in addEvent  "<<strerror(errno);
            return false;
        }

        ++pending_Event_Size_;
        fd_contex->_event=(Event)(event|fd_contex->_event);

        FdContex::FdEvent& event_ctx=fd_contex->getEventConetx(event);
        ASSERT(!event_ctx.scheduler
                &&!event_ctx._cb
                &&!event_ctx._fiber);
        event_ctx.scheduler=Scheduler::GetCurrentSch();
        if(cb){
            event_ctx._cb.swap(cb);
        }else{
            event_ctx._fiber=Fiber::GetCurrentFiber();
            ASSERT(event_ctx._fiber->state_==Fiber::EXEC);
        }
        return true;            
    }
    bool IOManager::delEvent(int fd,Event event){
        FdContex* fd_contex;
        {
            MutexLockGuard Lock(mutex_);
            if((int)fd_Contexs_.size()<=fd){
                return false;
            }
            fd_contex=fd_Contexs_[fd];
        }
        MutexLockGuard Lock(fd_contex->_mutex);
        if(!(fd_contex->_event&event)){
            return false;
        }
        int new_event=fd_contex->_event|event;
        int op =new_event?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.data.ptr=fd_contex;
        epevent.events=new_event;

        int rt=epoll_ctl(ep_fd_,op,fd_contex->_fd,&epevent);
        if(rt<0){
            LOG_DEBUG<<"Delevent faild: fd="<<fd_contex->_fd;
            return false;
        }
        
        --pending_Event_Size_;
        fd_contex->_event=(Event)new_event;
        FdContex::FdEvent& event_ctx=fd_contex->getEventConetx(event);
        fd_contex->resetContex(event_ctx);
        return true;
     
    }
    bool IOManager::cancelEvent(int fd,Event event){
        FdContex* fd_contex;
        {
            MutexLockGuard Lock(mutex_);
            if((int)fd_Contexs_.size()<=fd){
                return false;
            }
            fd_contex=fd_Contexs_[fd];
        }
        MutexLockGuard Lock(fd_contex->_mutex);
        if(!(fd_contex->_event&event)){
            return false;
        }
        int new_event=fd_contex->_event|event;
        int op =new_event?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.data.ptr=fd_contex;
        epevent.events=new_event;

        int rt=epoll_ctl(ep_fd_,op,fd_contex->_fd,&epevent);
        if(rt<0){
            LOG_DEBUG<<"Delevent faild: fd="<<fd_contex->_fd;
            return false;
        }
        --pending_Event_Size_; 
        fd_contex->triggerEvent(event);
        return true;
    }
    bool IOManager::cancelAll(int fd){
        FdContex * fd_contex;
        {
            MutexLockGuard Lock(mutex_);
            if(fd>=(int)fd_Contexs_.size()){
                return false;
            }
            fd_contex=fd_Contexs_[fd];
        }
        MutexLockGuard Lock(fd_contex->_mutex);
        if(!fd_contex->_event){
            LOG_DEBUG<<"cancelAll faild: fd="<<fd_contex->_fd;
            return false;
        }
        int op=EPOLL_CTL_DEL;
        epoll_event new_event;
        new_event.data.ptr=fd_contex;
        new_event.events=0;

        int rt=epoll_ctl(ep_fd_,op,fd_contex->_fd,&new_event);
        if(rt<0){
            LOG_DEBUG<<"cancelAll faild: fd="<<fd_contex->_fd;
            return false;
        }
        if(fd_contex->_event&&Event::READD){
            fd_contex->triggerEvent(Event::READD);
            --pending_Event_Size_;
        }
        if(fd_contex->_event&&Event::WRITE){
            fd_contex->triggerEvent(Event::WRITE);
            --pending_Event_Size_;
        }
        ASSERT(fd_contex->_event==0);
        return true;
    }
    IOManager* IOManager::GetCurrentIOManager(){
        return dynamic_cast<IOManager*>(Scheduler::GetCurrentSch());
    }
    void IOManager::tickle(){
        if(hasIdleThreads()){
            return;
        }
        int rt=write(tickle_fd_[1],"T",1);
        ASSERT(rt==1);
    }
    void IOManager::idle(){
        epoll_event*ep_events=new epoll_event[64]();
        std::shared_ptr<epoll_event>ptr(ep_events,[](epoll_event*pwtr){
            delete[] pwtr;
        });
        while(true){
            uint64_t next_timeout=0;
            if(stopping(next_timeout)){
                break;
            }
            int rt=0;
            do{
                static const int MAX_EPOLLTIME=3000;
                if(next_timeout!=~0ll){
                    next_timeout=next_timeout>MAX_EPOLLTIME?MAX_EPOLLTIME:next_timeout;
                }else{
                    next_timeout=MAX_EPOLLTIME;
                }
                rt=epoll_wait(ep_fd_,ep_events,64,next_timeout);
                if(rt<0&&errno==EINTR){
                }else{
                    break;
                }
            }while(true);
            std::vector<std::function<void()>> timer_cbs;
            listExpiredCb(timer_cbs);
            if(!timer_cbs.empty()){
               // LOG_DEBUG<<"timer_cbs size() is "<<timer_cbs.size();
                scheduler(timer_cbs.begin(),timer_cbs.end());
            }
            for(int i=0;i<rt;++i){
                epoll_event ep_event=ep_events[i];
                if(ep_event.data.fd==tickle_fd_[0]){
                    uint64_t deam[64];
                    while(read(tickle_fd_[0],deam,sizeof(deam))>0 );
                    continue;
                }
                FdContex*fd_contex=(FdContex*)ep_event.data.ptr;
                if(ep_event.events&(EPOLLHUP|EPOLLERR)){
                    ep_event.events|=(EPOLLIN|EPOLLOUT)&fd_contex->_event;
                }
                int real_event=NONE;
                if(ep_event.events&EPOLLIN){
                    real_event|=READD;
                }
                if(ep_event.events&EPOLLOUT){
                    real_event|=WRITE;
                }
                if((fd_contex->_event&real_event)==NONE){
                    continue;
                }
                int new_event=(~real_event&fd_contex->_event);
                int op=new_event?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
                ep_event.events|=EPOLLET|new_event;

                int rt=epoll_ctl(ep_fd_,op,fd_contex->_fd,&ep_event);
                if(rt<0){
                    LOG_DEBUG<< "epoll_ctl(" << ep_fd_ << ", "
                    << op << ", " << fd_contex->_fd << ", " << (EPOLL_EVENTS)ep_event.events << "):"
                    << rt << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
                }

                if(real_event&READD){
                    --pending_Event_Size_;
                    fd_contex->triggerEvent(READD);
                }
                if(real_event&WRITE){
                    --pending_Event_Size_;
                    fd_contex->triggerEvent(WRITE);
                }
            }
            Fiber::ptr cur=Fiber::GetCurrentFiber();
            auto raw_ptr=cur.get();
            cur.reset();
            raw_ptr->swapOut();
        }
    }

 bool IOManager::stopping(){
        return this->Scheduler::stopping()&&pending_Event_Size_==0;
    }

  bool IOManager::stopping(uint64_t&timeout){
        timeout=getNextTImer();
        return this->Scheduler::stopping()&&pending_Event_Size_==0&&timeout==~0ull;
    }

    void IOManager::onTimerInsertAtFront(){
        tickle();
    }
}


