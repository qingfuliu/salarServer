#ifndef _SINGLETON_H
#define _SINGLETON_H
#include<pthread.h>
#include"log.h"
#include<memory>

template<int level,class Appender=sylar::StdoutAppender>//,class Appender=sylar::StdoutAppender>
class SingletonLogger{
public:
    static sylar::Logger::ptr& getInstence(){
        pthread_once(&ponce_,&SingletonLogger::init);
        value_->setName_(std::string("root"));
        return value_;
    };
    static sylar::Logger::ptr& getInstence(std::string name){
        pthread_once(&ponce_,&SingletonLogger::init);
        value_->setName_(name);
        return value_;
    };
    SingletonLogger(const SingletonLogger&)=delete;
    SingletonLogger& operator=(const SingletonLogger&)=delete;
private:
    SingletonLogger();
    ~SingletonLogger();
    static void init(){
         value_.reset(new sylar::Logger);
         value_->addAppender(sylar::LogAppender::ptr(new Appender));
         value_->setLevel(sylar::LogLevel::Level(level));
    }
    static sylar::Logger::ptr value_;
    static pthread_once_t ponce_;
};
template<int level,class Appender>
 sylar::Logger::ptr SingletonLogger<level,Appender>::value_=nullptr;

template<int level,class Appender>
 pthread_once_t SingletonLogger<level,Appender>::ponce_=PTHREAD_ONCE_INIT;

 template<class T>
 class SingLeton{
    public:
    static T getInstence(){
        if(value_==nullptr)
        init();
        return *value_;
    }
    static void init(){
        value_.reset(new T);
    }
    SingLeton& operator=(const SingLeton&)=delete;
    SingLeton(const SingLeton&)=delete;
private:
    SingLeton(){}
    ~SingLeton(){}
    static std::shared_ptr<T>value_;
    static pthread_once_t ponce_;
 };

  template<class T>
  std::shared_ptr<T> SingLeton<T>::value_=nullptr;

  template<class T>
 pthread_once_t SingLeton<T>::ponce_=PTHREAD_ONCE_INIT;
 #endif