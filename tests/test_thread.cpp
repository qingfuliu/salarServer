#include"../sylar/sylar.h"
#include<vector>
#include<memory>
int count=0;
sylar::RWMutex rwmutex_;
void fun(){
    // int *ans=static_cast<int*>(arg);
    // LOG_DEBUG<<"the number is: "<<*ans;
    // LOG_DEBUG<<"the number is: "<<*ans;
    // LOG_DEBUG<<"the number is: "<<*ans;
    // //YAML::Node root=YAML::LoadFile("/home/lqf/cpp/sylar/conf/config.YAML");
    // LOG_DEBUG<<"the number is: "<<*ans;
    // LOG_DEBUG<<"the number is: "<<*ans;
    //  root=YAML::LoadFile("/home/lqf/cpp/sylar/conf/config.YAML");
    // sylar::Config::LoadFromYaml(root);
    for(int i=0;i<5000;++i){
        sylar::RWMutex::WrLockGuard lock(rwmutex_);
        ++count;
    }
}
int main(){
    YAML::Node root=YAML::LoadFile("/home/lqf/cpp/sylar/conf/config.YAML");
    sylar::Config::LoadFromYaml(root);
    std::vector<sylar::Thread::ptr>vec;
    for(int i=0;i<10;++i){
        std::string name="funcation: "+std::to_string(i);
        sylar::Thread::ptr p(new sylar::Thread(fun,name,(void*)new int(i)));
        vec.push_back(p);
    }
    for(int i=0;i<10;++i){
        vec[i]->join();
    }
    return 0;
}
