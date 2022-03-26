#ifndef __SYLAR_LOG_H
#define __SYLAR_LOG_H
#include<string>
#include<stdint.h>
#include<memory>
#include<pthread.h>
#include<list>
#include<algorithm>
#include<sstream>
#include<fstream>
#include<vector>
#include<unordered_map>
#include<set>
#include<iostream>
#include<map>
#include"util.h"
#include"thread.h"
namespace sylar{

class Logger;
class LoggerManager;
//日志级别
class LogLevel{
public:
    enum Level{
            UNKNOW=0,
            DEBUG=1,
            INFO=2,
            WARN=3,
            ERROR=4,
            FATAL=5,
            del_flag=100
        };
    static const char*ToString(LogLevel::Level level);
    static  LogLevel::Level fromString(const std::string&);
};

class LogEvent{
public:
    typedef std::shared_ptr<LogEvent>ptr;
    LogEvent(){}
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
    const char*file, int32_t elapse,int32_t line,int32_t threadId,
    uint32_t fiberId, time_t time,const std::string&threadName
    );
    const char*getFile()const{return file_;}
    int32_t getElapse()const{return elapse_;}
    int32_t getLine()const{return line_;}
    int32_t getThread()const{return threadId_;}
    uint32_t getFiberId()const{return fiberId_;}
    time_t getTime()const{return time_;}  
    std::string getContent()const{return content_.str();}  
    std::stringstream & getStringStream(){return content_;}
    std::shared_ptr<Logger> GetLogger()const {return logger_;}
    LogLevel::Level GetLevel()const{return level_;}
    void setLogger(const std::shared_ptr<Logger> logger){logger_=logger;}
    std::string getThreadName(){return threadName_;}
private:
    const char*file_=nullptr;//文件名
    int32_t elapse_;            //程序启动到现在的毫秒数
    int32_t line_=0;            //行号
    int32_t threadId_=0;      //线程号
    uint32_t fiberId_=0;        //协程
    time_t time_;                   //时间戳
    std::stringstream content_;          //内容
    std::shared_ptr<Logger> logger_;
    LogLevel::Level level_;
    std::string threadName_;
};


class LogWrap{
public:
    LogWrap(LogEvent::ptr event);
    ~LogWrap();
    std::stringstream & getStringStream();
private:
    LogEvent::ptr event_;
};

//日志格式器
class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter>ptr;
    LogFormatter(const std::string&pattern);
    //把logevent format为string，提供给LogAppender输出
    std::string format(std::shared_ptr<Logger>logger,LogLevel::Level level,
    LogEvent::ptr event);
public:
    class FormatItem{
        public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem(){}
        virtual void format(std::ostream&os,std::shared_ptr<Logger>logger,
        LogLevel::Level level,LogEvent::ptr event)=0;
    };
    void init();
    bool iserror(){return if_error;}
private:
    std::string pattern_;
    std::vector<FormatItem::ptr>items_;
    bool if_error=false;
};

//日志输出地
class LogAppender{
public:
    typedef std::shared_ptr<LogAppender>ptr;

    void setFormatter(LogFormatter::ptr formatter){
        MutexLockGuard lock(mutex_);
        formatter_=formatter;
        }
    LogFormatter::ptr  getFormatter(){
        LogFormatter::ptr copyOfFormatter_;
          {
              MutexLockGuard lock(mutex_);
              copyOfFormatter_=formatter_;
          }
        return copyOfFormatter_;
    }
    virtual ~LogAppender(){}

    virtual void log(std::shared_ptr<Logger>logger,LogLevel::Level& level,
    LogEvent::ptr event)=0;
    void  setLevel(const LogLevel::Level&level){
        level_=level;
    }
protected:
    LogLevel::Level level_;
    LogFormatter::ptr formatter_;
    MutexLock mutex_;
};


//日志器
class Logger:public std::enable_shared_from_this<Logger>{
    friend LoggerManager;
public:
    typedef std::shared_ptr<Logger>ptr;
    Logger(const std::string &name="root");
    void log(LogLevel::Level level,const LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);   

     void addAppender(LogAppender::ptr appender);

    void deleteAppender(LogAppender::ptr appender);

    void clearAppender();
    const std::string& getName()const {return name_;}
    void setName_(std::string  name){this->name_=name;}

    LogLevel::Level getLevel()const{return level_;}
    void setLevel(const LogLevel::Level &level){this->level_=level;}

    void setFormatter(LogFormatter::ptr&formatter){
        formatter_=(formatter);
    }
    void setFormatter(const std::string &val){
        LogFormatter::ptr formatter(new LogFormatter(val));
        if(!formatter->iserror()){
            formatter_=formatter;
        }
        std::cout<<"set formatter error "<<"value_="<<val<< std::endl;
    }
    LogFormatter::ptr getFormatter(){
        return formatter_;
    }
private:
    std::string name_;                                   //日志名称
    LogLevel::Level level_;                            //日志级别
    std::list<LogAppender::ptr> appenders_; //appender列表
    LogFormatter::ptr formatter_;
    Logger::ptr root_=nullptr;
    sylar::RWMutex mutex_;
};


//输出到控制控制台的appender
class StdoutAppender:public LogAppender{
public:
    typedef std::shared_ptr<StdoutAppender>ptr;
     void log(std::shared_ptr<Logger>logger,LogLevel::Level& level,
    LogEvent::ptr event) override;
private:

};
//输出到文件的appender
class FileAppender:public LogAppender{
public:
    typedef std::shared_ptr<FileAppender>ptr;
    FileAppender(const std::string&Filename="/home/lqf/cpp/sylar/logData/logData.txt");
    void log(std::shared_ptr<Logger>logger,LogLevel::Level& level,
    LogEvent::ptr event) override;
    bool reopen();
private:
    std::string fileName_;
    std::ofstream fileStream_;
};

class LoggerManager{
public:
    typedef std::shared_ptr<LoggerManager> ptr;
    LoggerManager(){
        root_.reset(new Logger);
        root_->addAppender(StdoutAppender::ptr(new StdoutAppender));
        init();
    }
    static Logger::ptr LookUp(const std::string &name){
        auto it=loggers_.find(name);
        if(it==loggers_.end()){
            return nullptr;
        }
        return it->second;
    }
    static void log(const LogLevel::Level& level,const LogEvent::ptr event );
    static Logger::ptr getLogger(const std::string&name);
    static void AddLogger(const std::string &name,Logger::ptr logger){
        loggers_[name]=logger;
        return;
    }
    static RWMutex& getMutexLock(){return mutex_;}
private:
    static std::map<std::string,Logger::ptr>loggers_;
    static Logger::ptr root_;    
    static RWMutex mutex_;
    void init();
};
struct LogAppenderDefiner{
    int type=0;//ostdout 1 file
    std::string formattor;
    std::string file;
    LogLevel::Level level=LogLevel::Level::UNKNOW;        
    bool operator = (const LogAppenderDefiner&b)const{
        return type==b.type&&
                  formattor==b.formattor&&
                  file==b.file&&
                  level==b.level;
    }  
};
class LogDefiner{
public:
    std::string name;                                   //日志名称
    sylar::LogLevel::Level level=LogLevel::Level::UNKNOW;                            //日志级别
    std::list<LogAppenderDefiner> appenders_; //appender列表
    std::string formatter_;
    LogDefiner(){

    }
    bool operator == (const LogDefiner&b)const{
        return name==b.name&&
                  level==b.level&&
                  formatter_==b.formatter_;
    }  
    bool operator<(const LogDefiner&b)const{
        return name<b.name;
    }
    bool operator>(const LogDefiner&b)const{
        return name>b.name;
    }

};


class Loginit{
public:
    Loginit();
};

class LogManagerWrap{
public:
    LogManagerWrap();
    LogManagerWrap(LogEvent::ptr event){
        event_=event;
    }
    template<class T>
    LogManagerWrap& operator<<(const T&value){
        event_->getStringStream()<<value;
        return *this;
    }
    ~LogManagerWrap();
private:
    LogEvent::ptr event_;
};
 }
#endif