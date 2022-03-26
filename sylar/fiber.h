#ifndef _FIBER_H_
#define  _FIBER_H_
#include<functional>
#include<ucontext.h>
#include<memory>
#include<atomic>
namespace sylar{
    class Scheduler;
    class Fiber : public std::enable_shared_from_this<Fiber>{
    public:
        typedef std::shared_ptr<Fiber> ptr;
        enum State{
            INIT,
            READY,
            HOLD,
            EXEC,
            TERM,
            EXCET
        };
        Fiber(std::function<void()>cb,size_t stack_size,bool use_coller=false);
        ~Fiber();
        void swapIn();
        void swapOut();
        void back();
        void reset(std::function<void()>cb=nullptr);
        void call();
    private:
        Fiber();
    public:
        uint64_t _id_;
        State state_=INIT;
        size_t stack_size_;
        void* stack_=nullptr;
        ucontext_t contex_;
        std::function<void()>cb_;
    public:
        static Fiber::ptr GetCurrentFiber();
        static uint64_t FiberCount();
        static void MainFunc();
        static void BackExternFunc();
        static void setCurrentFiber(Fiber*);
        static uint64_t GetFiberId();
        static void YieldToReady();
        static void YieldToHold();
    };
}
#endif