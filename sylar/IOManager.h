#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__
#include"scheduler.h"
#include"timer.h"
namespace sylar{


    class IOManager : public Scheduler,public TimerManager{
    public:
        typedef std::shared_ptr<IOManager> ptr;
        enum  Event
        {
            NONE=0x0,
            READD=0x1,
            WRITE=0x4,
        };
        IOManager(size_t threadPool_size_=10,bool use_caller=true,const std::string &name="");
        ~IOManager();

        void fd_contex_resize(size_t size);
        //0成功  1失败 -1重
        bool addEvent(int fd,Event event,std::function<void()>cb=nullptr);
        bool delEvent(int fd,Event event);
        bool cancelEvent(int fd,Event event);
        bool cancelAll(int fd);
        static IOManager* GetCurrentIOManager();
protected:
        void tickle() override;
        void idle()override;
        bool stopping()override;
        void onTimerInsertAtFront()override;
        bool stopping(uint64_t&timeout);
    private:
        struct FdContex{
            struct FdEvent{
                std::function<void()>_cb;
                Fiber::ptr _fiber;
                Scheduler*scheduler=nullptr;
            };
            FdEvent& getEventConetx(Event event);
            void triggerEvent(Event);
            void resetContex(FdEvent&contex);
            FdEvent  _write;
            FdEvent  _read;
            int _fd;     //事件关联的
            Event _event=NONE;
            MutexLock _mutex;
        };
    private:
        int ep_fd_;
        int tickle_fd_[2];
        std::atomic<size_t> pending_Event_Size_={0};
        MutexLock mutex_;
        std::vector<FdContex*>fd_Contexs_;
    };
}



#endif