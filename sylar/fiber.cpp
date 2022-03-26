#include"fiber.h"
#include"config.h"
#include"defineLogger.h"
#include<memory>
#include"scheduler.h"
namespace sylar{
    
    static std::atomic<uint64_t>id_(0);
    static std::atomic<uint64_t>Current_Count_(0);

    static thread_local Fiber* Current_Fiber=nullptr;
    static thread_local Fiber::ptr t_threadFiber=nullptr;

    static ConfigVar<uint64_t>::ptr Config_Stack_Size_=Config::Lookup(
        "system.stack_size_",
        (uint64_t)1024*1024,
        "Config_Stack_Size_"
    );

    uint64_t Fiber::GetFiberId(){
        if(Current_Fiber){
            return Current_Fiber->_id_;
        }
        return  0;
    }

    class MallocStackAllocator{
    public:
        static void* Allocate(size_t size){
            return malloc(size);
        }
        static void Dealloc(void*vp){
            return free(vp);
        }
    };

    using StackAllocator=MallocStackAllocator;

    Fiber::Fiber(){
        _id_=0;
        state_=EXEC;
        setCurrentFiber(this);
        if(getcontext(&contex_)){
            LOG_DEBUG<<"getcontext error!";
            return;
        }
        ++Current_Count_;
    }
    Fiber::Fiber(std::function<void()>cb,uint64_t stack_size,bool use_coller):
        _id_(++id_),
        cb_(cb),
        stack_size_(stack_size)
    {
        //LOG_DEBUG<<"Fiber: "<<_id_;
        ++Current_Count_;
        stack_size_=stack_size_?stack_size_:Config_Stack_Size_->getVal();
        stack_=StackAllocator::Allocate(stack_size_);

        if(getcontext(&contex_)){
            LOG_DEBUG<<"getcontext error!";
            return;
        }
        contex_.uc_link=NULL;
        contex_.uc_stack.ss_size=stack_size_;
        contex_.uc_stack.ss_sp=stack_;
        if(use_coller==false)
            makecontext(&contex_,&Fiber::MainFunc,0);
        else
            makecontext(&contex_,&Fiber::BackExternFunc,0);
        state_=INIT;
    }
    Fiber::~Fiber(){
        --Current_Count_;
        if(stack_){
            ASSERT(
                state_==TERM
                ||state_==INIT
                ||state_==EXCET
            );
            StackAllocator::Dealloc(stack_);
        }else{
            ASSERT(!cb_);
            ASSERT(state_==EXEC);
            Fiber*cur=Current_Fiber;
            if(cur==this){
                setCurrentFiber(nullptr);
            }
        }
        //LOG_DEBUG<<"~Fiber: "<<_id_;
    }
    void Fiber::swapIn(){
        setCurrentFiber(this);
        state_=EXEC;
        if(swapcontext(&(Scheduler::GetMainFiber()->contex_),&contex_)){
                LOG_FATAL<<"swapcontext error at"<<"swapIn";
                return;
        }
    }
    void Fiber::swapOut(){
        setCurrentFiber(Scheduler::GetMainFiber());
        if(swapcontext(&contex_, &Scheduler::GetMainFiber()->contex_)) {
            ASSERT1(false, "swapcontext");
        }
    }
    void Fiber::back(){
        Fiber::setCurrentFiber(t_threadFiber.get());
        if(swapcontext(&contex_,&(t_threadFiber->contex_))){
            LOG_FATAL<<"swapcontext error at"<<"back";
            return;
        }
    }
    void Fiber::reset(std::function<void()>cb){
        assert(stack_);
        ASSERT(state_==INIT||state_==TERM);
        cb_=cb;
        if(getcontext(&contex_)){
            LOG_FATAL<<"getcontext errorr...";
            return;
        }
        contex_.uc_link=NULL;
        contex_.uc_stack.ss_size=stack_size_;
        contex_.uc_stack.ss_sp=stack_;
        makecontext(
            &contex_,
            &Fiber::MainFunc,
            0
         );
        state_=INIT;    
    }
    void Fiber::call(){
        setCurrentFiber(this);
        state_=EXEC;
         if(swapcontext(&t_threadFiber->contex_,&contex_)){
            LOG_FATAL<<"swapcontext error at"<<"call";
            return;
        }
    }

        void Fiber::YieldToReady(){
            Fiber::ptr cur=GetCurrentFiber();
            ASSERT(cur->state_==EXEC);
            cur->state_=READY;
            cur->swapOut();
        }

        void Fiber::YieldToHold(){
            Fiber::ptr cur=GetCurrentFiber();
            ASSERT(cur->state_==EXEC);
            cur->swapOut();
        }

    Fiber::ptr Fiber::GetCurrentFiber(){
           if(Current_Fiber){
               return Current_Fiber->shared_from_this();
           }
           Fiber::ptr main(new Fiber);
           ASSERT(Current_Fiber==main.get());
           t_threadFiber=main;
           return main->shared_from_this();
    }
    uint64_t Fiber::FiberCount(){
        return Current_Count_;
    }
    void Fiber::MainFunc(){
        Fiber::ptr cur=GetCurrentFiber();
        try
        {
            cur->cb_();
            cur->state_=TERM;
            cur->cb_=nullptr;
        }
        catch(const std::exception& e)
        {
            cur->state_=EXCET;
            LOG_FATAL<<"MainFunc error";
        }catch(...){
            cur->state_=EXCET;
        }
        Fiber* raw=cur.get();
        cur.reset();
        raw->swapOut();

        ASSERT1(false,"never come to here");
    }
    void Fiber::setCurrentFiber(Fiber*f){
        Current_Fiber=f;
        return;
    }
    void Fiber::BackExternFunc(){
        Fiber::ptr cur=GetCurrentFiber();
        try
        {
            cur->cb_();
            cur->state_=TERM;
            cur->cb_=nullptr;
        }
        catch(const std::exception& e)
        {
            cur->state_=EXCET;
            LOG_FATAL<<"BackExternFunc error";
        }catch(...){
            cur->state_=EXCET;
        }
        Fiber* raw=cur.get();
        cur.reset();
        raw->back();

        ASSERT1(false,"never come to here");
    }

}
