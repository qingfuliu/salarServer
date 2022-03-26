#include"config.h"
#include<list>
#include<yaml-cpp/yaml.h>
#include"defineLogger.h"
#include"log.h"
namespace sylar
{
ConfigVarBase::ptr Config::LookUpBase(const std::string&name){
    auto it=getDatas_().find(name);
    return it==getDatas_().end()?nullptr:it->second;
}
static void LoadAllNumber(const std::string & prefix,
                                        const YAML::Node&node,
                                        std::list<std::pair<std::string,const YAML::Node>>& output){
     if(prefix.find_first_not_of("abcdefghigklmnopqrstuvwxyz._0123456789")!=std::string::npos)
     {
        SYLAR_LOG_FATAL(LOGGER_DEBUG_ROOT)<<"Config invaild name "<<prefix<<":"<<node;
        return;
     }
     output.push_back(std::make_pair(prefix,node));
    if(node.IsMap()){
            for(auto it=node.begin();it!=node.end();++it){
                LoadAllNumber(prefix.empty() ? it->first.Scalar()
                    : prefix + "." + it->first.Scalar(),it->second,output);
        }
    }
}
void Config::LoadFromYaml(const YAML::Node&root){
    std::list<std::pair<std::string,const YAML::Node>>allNodes;
    LoadAllNumber("",root,allNodes);
    MutexLockGuard Lock(mutex_);
    for(auto& i:allNodes){
        std::string key=i.first;
        if(key.empty())continue;
        std::transform(key.begin(),key.end(),key.begin(),::tolower);
        ConfigVarBase::ptr val=LookUpBase(key);
        if(val){
            if(i.second.IsScalar()){
                val->fromString(i.second.Scalar());
            }else{
                std::stringstream ss;
                ss<<i.second;
                val->fromString(ss.str());
            }
        }
    }
}
    void Config::init(){
        YAML::Node root=YAML::LoadFile("/home/lqf/cpp/sylar/conf/config.YAML");
        LoadFromYaml(root);
    }
    MutexLock Config::mutex_;
} // namespace sylarConfigVar
