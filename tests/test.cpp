#include<iostream>
#include"../sylar/Singleton.h"
#include"../sylar/log.h"
#include"../sylar/util.h"
#include<yaml-cpp/yaml.h>
#include<vector>
#include"../sylar/defineLogger.h"
#include"../sylar/config.h"
sylar::ConfigVar<int>::ptr p=sylar::Config::Lookup("system.port",int(8080),"systdem.port");
sylar::ConfigVar<float>::ptr p1=sylar::Config::Lookup("system.value",(float)1.9f,"system.value");

sylar::ConfigVar<std::vector<int> >::ptr p3=sylar::Config::Lookup("system.vec",
                                                       std::vector<int>{2,2}
                                                         ,"system.vec");
sylar::ConfigVar<std::list<int> >::ptr p4=sylar::Config::Lookup("system.list",
                                                       std::list<int>{2,2}
                                                         ,"system.list");
sylar::ConfigVar<std::set<int> >::ptr p5=sylar::Config::Lookup("system.set",
                                                       std::set<int>{2,2}
                                                         ,"system.set");
sylar::ConfigVar<std::unordered_set<int> >::ptr p6=sylar::Config::Lookup("system.unordered_set",
                                                       std::unordered_set<int>{2,2}
                                                         ,"system.unordered_set");
sylar::ConfigVar<std::unordered_map<std::string,int> >::ptr p7=sylar::Config::Lookup("system.unordered_map",
                                                       std::unordered_map<std::string,int>{{"port",2}}
                                                         ,"system.unordered_map");
sylar::ConfigVar<std::map<std::string,int> >::ptr p8=sylar::Config::Lookup("system.map",
                                                       std::map<std::string,int>{{"port",2},{"k",2},{"p",2}}
                                                         ,"system.map");
    void print_yaml(const YAML::Node&node,int level,std::string name){
    if(node.IsScalar()){
    SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<std::string(level*4,' ')<<
    node.Scalar()<<" - "<<node.Tag()<<" - "<<level;
    }else if(node.IsNull()){
            SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<"NULL"<<node.Tag();
    }else if(node.IsMap()){
        for(auto it=node.begin();
        it!=node.end();
        ++it){
              SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<std::string(level*4,' ')<<it->first<<" - "<<
              node.Tag()<<" - "<<level;
               print_yaml(it->second,level+1,it->first.Scalar());
        }
    }else if(node.IsSequence()){
             for(size_t it=0;it<node.size();++it){
              SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<std::string(level*4,' ')<<it<<" - "<<
             node[it].Tag()<<" - "<<level;
               print_yaml(node[it],level+1,node[it].Scalar());
    }
}
}
void test_yam(){
    p1->addValChangeCB([](const float&a,const float&b){
         SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<"old_value: "<<a<<" new_value"<<
         b;
    });
    SYLAR_FILELOG_WARN_(lqf)<<p1->toString();
    SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<< p1->toString();
    #define XX(p,name) \
    {   \
        auto &v=p->getVal();    \
        for(auto &val:v)    \
        {   \
            SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<#name<<val;  \
            SYLAR_FILELOG_WARN_ROOT<<#name<<val;  \
        }   \
          SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<#name" tostring"<<p->toString();    \
          SYLAR_FILELOG_WARN_ROOT<<#name" tostring"<<p->toString();  \
    }
    #define XX_M(p,name) \
    {   \
        auto &v=p->getVal();    \
        for(auto &val:v)    \
        {   \
            SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<#name<<val.first<<"    "<<val.second;  \
             SYLAR_FILELOG_WARN_ROOT<<#name<<val.first<<"    "<<val.second;  \
        }   \
          SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<#name" tostring:  "<<p->toString();    \
          SYLAR_FILELOG_WARN_ROOT<<#name" tostring:  "<<p->toString();    \
    }
    XX(p3,vector);
    XX(p4,list);
    XX(p5,set);
    XX(p6,unordered_set);
    XX_M(p7,unordered_map);
    XX_M(p8,map);
    YAML::Node root=YAML::LoadFile("/home/lqf/cpp/sylar/config.YAML");
    //print_yaml(root,0,"");
    sylar::Config::LoadFromYaml(root);
    SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<<p->toString();
    SYLAR_LOG_DEBUG(LOGGER_DEBUG_ROOT)<< p1->toString();
    XX(p3,vector);
    XX(p4,list);
    XX(p5,set);
    XX(p6,unordered_set);
    XX_M(p7,unordered_map);
    XX_M(p8,map);
     #undef XX
     #undef XX_M
}
int main(){
    LOG_WARN<<"lqfd";
    //test_yam();
    return 0;
}