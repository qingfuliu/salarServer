#include"../sylar/sylar.h"
void test_fiber(){

    LOG_DEBUG<<"test_fiber brfore swapout";
}
void thread_cb(){
    sylar::Fiber::GetCurrentFiber();
    sylar::Fiber::ptr p(new sylar::Fiber(test_fiber,0));
    LOG_DEBUG<<"main before swapin";
    p->swapIn();
    LOG_DEBUG<<"main after swapout2";
}
void test(){
    LOG_DEBUG<<"test!!!";
}
int main(){
    YAML::Node root=YAML::LoadFile("/home/lqf/cpp/sylar/conf/config.YAML");
    sylar:: Config::LoadFromYaml(root);

    // sylar::Thread::ptr thread(new sylar::Thread(thread_cb,"naem_1"));
    // sylar::Thread::ptr thread2(new sylar::Thread(thread_cb,"naem_2"));
    // sylar::Thread::ptr thread3(new sylar::Thread(thread_cb,"naem_3"));
    // sylar::Thread::ptr thread4(new sylar::Thread(thread_cb,"naem_4"));
    sylar::Scheduler sc(0);
   sc.scheduler(test);
   sc.scheduler(test);
   sc.scheduler(test);
   sc.scheduler(test);
   sc.scheduler(test);
   sc.scheduler(test);
    sc.start();
    sc.stop();
    //sc.stop();
    return 0;
}
