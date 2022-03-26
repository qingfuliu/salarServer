#ifndef _SYLAR_CONFIG_H_
#define _SYLAR_CONFIG_H_
#include<iostream>
#include<string.h>
#include<memory>
#include<boost/lexical_cast.hpp>
#include<map>
#include<vector>
#include<map>
#include<unordered_map>
#include<unordered_set>
#include<list>
#include<yaml-cpp/yaml.h>
#include"defineLogger.h"
#include"log.h"
#include"thread.h"
namespace sylar{
class ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string&name,const std::string description=""):
    name_(name),
    description_(description){
        std::transform(name_.begin(),name_.end(),name_.begin(),
        ::tolower);
    }

    virtual ~ConfigVarBase(){}

    const std::string getName_(){return name_;}
    const std::string getDescription_(){return description_;}
    virtual std::string getTypeName()const =0;
    virtual std::string toString()=0;
    virtual bool fromString(const std::string&val)=0;

protected:
    std::string name_;
    std::string description_;
};
template<class F,class T>
class LexicalCast{
public:
    T operator()(const F&v){
        return boost::lexical_cast<T>(v);
    }
    F operator()(const T&v){
        return boost::lexical_cast<F>(v);
    }
};

//vector序列化 反序列化
template<class T>
class LexicalCast<std::string,std::vector<T>>{
public:
    std::vector<T> operator()(const std::string& v){
        YAML::Node node=YAML::Load(v);
        typename std::vector<T>vec;
        std::stringstream ss;
        for(int i=0;i<node.size();++i){
            ss.str("");
            ss<<node[i];
            vec.push_back(LexicalCast<std::string,T>()(ss.str()));
        }
        return std::move(vec);
    }
};
template<class T>
class LexicalCast<std::vector<T>,std::string>{
public:
    std::string operator()(const std::vector<T> & v){
        YAML::Node node;
        for(const auto&i:v){
            node.push_back(YAML::Load(
                LexicalCast<T,std::string>()(i)
            ));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

//list序列化 反序列化
template<class T>
class LexicalCast<std::string,std::list<T>>{
public:
    std::list<T> operator()(const std::string& v){
        YAML::Node node=YAML::Load(v);
        typename std::list<T>vec;
        std::stringstream ss;
        for(int i=0;i<node.size();++i){
            ss.str("");
            ss<<node[i];
            vec.push_back(LexicalCast<std::string,T>()(ss.str()));
        }
        return std::move(vec);
    }
};
template<class T>
class LexicalCast<std::list<T>,std::string>{
public:
    std::string operator()(const std::list<T> & v){
        YAML::Node node;
        for(const auto&i:v){
            node.push_back(YAML::Load(
                LexicalCast<T,std::string>()(i)
            ));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};
//LogDefiner 反序列化
template<>
class LexicalCast<std::string,std::set<LogDefiner>>{
public:
        std::set<LogDefiner> operator()(const std::string&val){
            YAML::Node node=YAML::Load(val);
            std::set<LogDefiner>logDefiners;
            for(size_t i=0;i<node.size();++i){
                LogDefiner logDefiner;
                if(node[i]["name"].IsDefined()){
                    logDefiner.name=node[i]["name"].as<std::string>();
                }else{
                    std::cout<<"name is not exist"<<"   "<<"Logger Config Error..."<<"  at   LexicalCast<std::string,std::set<LogDefiner>> ::name"
                    <<std::endl;
                    continue;
                }
                if(node[i]["level"].IsDefined()){
                    logDefiner.level=LogLevel::fromString(node[i]["level"].as<std::string>());
                }
                if(node[i]["formatter_"].IsDefined()){
                    logDefiner.formatter_=node[i]["formatter_"].as<std::string>();
                }
                if(node[i]["appenders"].IsDefined()){
                    for(size_t  j=0;j<node[i]["appenders"].size();++j){
                        auto value=node[i]["appenders"][j];
                        LogAppenderDefiner logAppenderDefiner;
                        if(value["type"].IsDefined()){
                            auto type=value["type"].as<std::string>();
                            if(type=="FileLogAppender"){
                                 logAppenderDefiner.type=2;
                            }else if(type=="StdoutLogAppender"){
                                    logAppenderDefiner.type=1;
                            }else{
                            std::cout<<"name is not exist"<<"   "<<"Logger Config Error..."<<"  at   LexicalCast<std::string,std::set<LogDefiner>> ::appenders"
                            <<std::endl;
                            continue;
                            }
                        }else{
                             std::cout<<"name is not exist"<<"   "<<"Logger Config Error..."<<"  at   LexicalCast<std::string,std::set<LogDefiner>> ::appenders"
                            <<std::endl;
                            continue;
                        }
                        if(value["formattor"].IsDefined()){
                            logAppenderDefiner.formattor=value["formattor"].as<std::string>();
                        }
                        if(value["file"].IsDefined()){
                            logAppenderDefiner.file=value["file"].as<std::string>();
                        }
                        if(value["level"].IsDefined()){
                            logAppenderDefiner.level=LogLevel::fromString(value["level"].as<std::string>());
                        }
                        logDefiner.appenders_.push_back(logAppenderDefiner);
                    }
                }
              logDefiners.insert(logDefiner);
            }
            return logDefiners;
        }
};
template<>
class LexicalCast<std::set<sylar::LogDefiner>,std::string>{
public:
    std::string operator()(const std::set<sylar::LogDefiner>&val){
        YAML::Node root;
                for(auto&i:val){
                    YAML::Node node;
                    node["name"]=i.name;
                    node["level"]=LogLevel::ToString(i.level);
                    if(!i.formatter_.empty())
                    node["formatter_"]=i.formatter_;
                    for(auto&ap:i.appenders_){
                        YAML::Node apnode;
                        if(!ap.formattor.empty())
                        apnode["formattor"]=ap.formattor;
                        if(!ap.file.empty())
                        apnode["file"]=ap.file;
                        apnode["level"]=LogLevel::ToString(ap.level);
                        if(ap.type==1){
                            apnode["type"]="StdoutLogAppender";
                        }else if(ap.type==2){
                            apnode["type"]="FileLogAppender";
                        }
                        node["appenders_"].push_back(apnode);
                    }
                    root.push_back(node);
                }
                std::stringstream ss;
                ss<<root;
                return ss.str();
    }
};

//set序列化 反序列化
template<class T>
class LexicalCast<std::string,std::set<T>>{
public:
    std::set<T> operator()(const std::string& v){
        YAML::Node node=YAML::Load(v);
        typename std::set<T>vec;
        std::stringstream ss;
        for(int i=0;i<node.size();++i){
            ss.str("");
            ss<<node[i];
            vec.insert(LexicalCast<std::string,T>()(ss.str()));
        }
        return std::move(vec);
    }
};
template<class T>
class LexicalCast<std::set<T>,std::string>{
public:
    std::string operator()(const std::set<T> & v){
        YAML::Node node;
        for(const auto&i:v){
            node.push_back(YAML::Load(
                LexicalCast<T,std::string>()(i)
            ));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

//unordered_set序列化 反序列化
template<class T>
class LexicalCast<std::string,std::unordered_set<T>>{
public:
    std::unordered_set<T> operator()(const std::string& v){
        YAML::Node node=YAML::Load(v);
        typename std::unordered_set<T>vec;
        std::stringstream ss;
        for(int i=0;i<node.size();++i){
            ss.str("");
            ss<<node[i];
            vec.insert(LexicalCast<std::string,T>()(ss.str()));
        }
        return std::move(vec);
    }
};
template<class T>
class LexicalCast<std::unordered_set<T>,std::string>{
public:
    std::string operator()(const std::unordered_set<T> & v){
        YAML::Node node;
        for(const auto&i:v){
            node.push_back(YAML::Load(
                LexicalCast<T,std::string>()(i)
            ));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

//unordered_map序列化 反序列化
template<class T>
class LexicalCast<std::string,std::unordered_map<std::string,T> >{
public:
    std::unordered_map<std::string,T>  operator()(const std::string& v){
        YAML::Node node=YAML::Load(v);
        typename std::unordered_map<std::string,T> map;
        std::stringstream ss;
        for(auto i=node.begin();
        i!=node.end();
        ++i){
            ss.str("");
            ss<<i->second;
            map.insert(std::make_pair(
                i->first.Scalar(),
                LexicalCast<std::string,T>()(ss.str())
            ));
        }
        return std::move(map);
    }
};
template<class T>
class LexicalCast<std::unordered_map<std::string,T>,std::string>{
public:
    std::string operator()(const std::unordered_map<std::string,T> & v){
        YAML::Node node;
        for(const auto&i:v){
                node[i.first]=YAML::Load(LexicalCast<T,std::string>()(i.second));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

//map序列化 反序列化
template<class T>
class LexicalCast<std::string,std::map<std::string,T> >{
public:
    std::map<std::string,T>  operator()(const std::string& v){
        YAML::Node node=YAML::Load(v);
        typename std::map<std::string,T> map;
        std::stringstream ss;
        for(auto i=node.begin();
        i!=node.end();
        ++i){
            ss.str("");
            ss<<i->second;
            map.insert(std::make_pair(
                i->first.Scalar(),
                LexicalCast<std::string,T>()(ss.str())
            ));
        }
        return std::move(map);
    }
};
template<class T>
class LexicalCast<std::map<std::string,T>,std::string>{
public:
    std::string operator()(const std::map<std::string,T> & v){
        YAML::Node node;
        for(const auto&i:v){
                node[i.first]=YAML::Load(LexicalCast<T,std::string>()(
                    i.second
                ));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};


template<class T,class FromStr=LexicalCast<std::string,T>,class ToStr=LexicalCast<T,std::string>>
class ConfigVar : public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T&,const T&)> callback;
    ConfigVar(const std::string&name,const T& default_val,const std::string &description=""):
    ConfigVarBase(name,description),
    val_(default_val)
    {}
    std::string toString()override{
        try
        {
            //return boost::lexical_cast<std::string>(val_);
            return ToStr()(val_);
        }
        catch(const std::exception& e)
        {
            SYLAR_LOG_ERROR(LOGGER_DEBUG_ROOT)<<"ConfigVarBase toString error:"<<
            e.what()<<" convert: "<<typeid(val_).name()<<" to string";
        }
        return "";
    }
    bool fromString(const std::string &name)override{
        try
        {
            //val_=boost::lexical_cast<T>(name);
            setVal(FromStr()(name));
            return true;
        }
        catch(const std::exception& e)
        {
            SYLAR_LOG_ERROR(LOGGER_DEBUG_ROOT)<<"ConfigVarBase fromString error:"<<
            e.what()<<" convert: "<<" string to"<<typeid(val_).name();
        }
        return false;
    }
    std::string getTypeName()const override{return typeid(T).name();}
    const T getVal(){
        T copyOf;
        {
            MutexLockGuard Val_Lock(val_mutex_);
            copyOf=val_;
        }
        return copyOf;
        }
    void setVal(const T& val){
        std::map<uint64_t,callback> callbacks_copy_;
        MutexLockGuard Val_Lock(val_mutex_);
        {
            MutexLockGuard Lock(mutex_);
            callbacks_copy_=callbacks_;
        }
        for(auto&callback_:callbacks_copy_){
                callback_.second(val_,val);
         }
            val_=val;
    }
    void addValChangeCB(callback callback_){
        static uint64_t key;
        MutexLockGuard Lock(mutex_);
        if(getValChangeCB(key+1)==nullptr)
        callbacks_[key++]=callback_;
    }
     void delValChangeCB(uint64_t key){
        MutexLockGuard Lock(mutex_);
         callbacks_.erase(key);
    }
    callback getValChangeCB(uint64_t key){
        if(callbacks_.find(key)==callbacks_.end())
        {
            return nullptr;
        }else
        return callbacks_[key];
    }
    void ClearValChangeCB(){
        MutexLockGuard Lock(mutex_);
        callbacks_.clear();
    }
private:
    T val_;
    std::map<uint64_t,callback>callbacks_;
    MutexLock mutex_;
    MutexLock val_mutex_;
};

class Config {
public:
    typename std::shared_ptr<Config> ptr;
    typedef std::map<std::string,ConfigVarBase::ptr> ConfigVarMap;
    template<typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
                             const T& default_var,
                            const std::string&  description)
    {
         //auto temp=find<T>(name);
            MutexLockGuard Locj(mutex_);
        auto temp=getDatas_().find(name);
         if(temp!=getDatas_().end()){
             auto it=std::dynamic_pointer_cast<ConfigVar<T>>(temp->second);
             if(it==nullptr){
                  SYLAR_LOG_DEBUG(LOGGER_DEBUG_(lqf))<<"Loopup name="<< name<<" exists nut type is not"<<
                typeid(T).name()<<" real_type is "<<  temp->second->getTypeName();
                return it;
             }else{
                  SYLAR_LOG_DEBUG(LOGGER_DEBUG_(lqf))<<"Loopup name="<< name<<" exists";
                return it;
             }
         }
        if(name.find_first_of("abcdefghigklmnopqistuvwxyzABCDEFGHIGKLMNOPQISTUVWXYZ._0123456789")==std::string::npos){
           SYLAR_LOG_FATAL(LOGGER_DEBUG_(lqf))<<"Loopup name="<< name<<
            "is a invalid argument";
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,
        default_var,
        description));
        getDatas_()[name]=v;
        return v;
    }                             
    template<typename T>
    static typename ConfigVar<T>::ptr find(const std::string& name){
            auto it=getDatas_().find(name);
            if(it==getDatas_().end()){
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }  

    static void LoadFromYaml(const YAML::Node& root);
    static ConfigVarBase::ptr LookUpBase(const std::string& name);
    static void init();
private:
    static ConfigVarMap&getDatas_(){
    static ConfigVarMap datas_;
        return datas_;
    }
    static MutexLock mutex_;
};
}
#endif
