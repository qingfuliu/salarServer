#ifndef __SCHEDULER__H__
#define __SCHEDULER__H__
#include<memory>
#include<vector>
#include<functional>
#include<atomic>
#include"thread.h"
#include"fiber.h"
namespace sylar{
    class Scheduler{
    public:
        typedef std::shared_ptr<Scheduler> ptr;
         /**
     * @brief 构造函数
     * @param[in] threads 线程数量
     * @param[in] use_caller 是否使用当前调用线程
     * @param[in] name 协程调度器名称
     */
        Scheduler(size_t threadPool_size_,bool use_caller=true,const std::string &name="Scheduler Thread:");

        ~Scheduler();


        template<class FiberOrCb>
        void scheduler(FiberOrCb fc,int thread=-1){
            bool need_tickel=false;
            {
                MutexLockGuard Lock(mutex_);
                need_tickel=schedulerNoLock(fc,thread);
            }
            if(need_tickel){
                tickle();
            }
        }

        template<class InputIterator>
        bool scheduler(InputIterator begin,InputIterator end){
            bool need_tickel=false;
            {
                MutexLockGuard Lock(mutex_);
                while(begin!=end){
                    need_tickel=schedulerNoLock(*begin,-1)||need_tickel;
                    ++begin;
                }
            }
            if(need_tickel){
                tickle();
            }
            return need_tickel;
        }
        template <class FiberOrCb>
        bool schedulerNoLock(FiberOrCb fc,int thread){
            bool need_tickel=fibers_.empty();
            FiberAnThread p(fc,thread);
            fibers_.push_back(p);
            return need_tickel;
        }
        void start();
        void stop();
        const std::string&getName()const{return name_;}

        static  const std::string&GetCurrentSchName();
        static Scheduler* GetCurrentSch();
        static Fiber* GetMainFiber();
        std::ostream& dump(std::ostream& os);
        void switchTo(int thread=-1);
    private:
            struct FiberAnThread{
                public:
                std::function<void()>cb;
                Fiber::ptr fiber;
                int thread;

                FiberAnThread():
                thread(-1){}

                FiberAnThread(Fiber::ptr fiber_,int thread_):
                fiber(fiber_),
                thread(thread_){}

                FiberAnThread(std::function<void()>cb_,int thread_):
                cb(cb_),
                thread(thread_){}

                FiberAnThread(std::function<void()>*cb_,int thread_):
                thread(thread_){
                    cb.swap(*cb_);
                }

                FiberAnThread(Fiber::ptr* fiber_,int thread_):
                thread(thread_){
                    fiber.swap(*fiber_);
                }

                void reset(){
                    cb=nullptr;
                    fiber=nullptr;
                    thread=-1;
                }
        };
        //线程池
        std::vector<Thread::ptr>threads_;
        std::vector<int>threadIds_;
        //协程池
        std::vector<FiberAnThread>fibers_;
        //主协程
        Fiber::ptr main_fiber_;
        //名字
        std::string name_;
        Fiber::ptr root_Fiber_=nullptr;
    protected:
        virtual void tickle();
        virtual void idle();
        virtual bool stopping();
         void setThis();
        bool hasIdleThreads(){return idle_threads_count_>=0;}
        void run();
    protected:
        MutexLock mutex_;
        int threadPool_size_;
        int main_thread_=0;
        std::atomic<int>active_threads_count_={0};
        std::atomic<int>idle_threads_count_={0};
        bool stopping_=true;
        bool auto_stop_=false;
        bool is_activate_=false;
    };
}
#endif
