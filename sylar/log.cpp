#include"log.h"
#include<iostream>
#include<functional>
#include<map>
#include<memory>
#include"Singleton.h"
#include"config.h"
#include"thread.h"
namespace sylar{
    
    LogEvent::LogEvent(Logger::ptr logger, LogLevel::Level level,
        const char*file, int32_t elapse,int32_t line,int32_t threadId,
    uint32_t fiberId, time_t time,const std::string&threadName
    ):
    file_(file),
    elapse_(elapse),
    line_(line),
    threadId_(threadId),
    fiberId_(fiberId),
    time_(time),
    logger_(logger),
    level_(level),
    threadName_(threadName)
    {

    }

    LogWrap::LogWrap( LogEvent::ptr event):
    event_(event){

    }

    LogWrap::~LogWrap(){
        event_->GetLogger()->log(event_->GetLevel(),event_);
    }

    std::stringstream & LogWrap::getStringStream(){
            return event_->getStringStream();
    }



    Logger::Logger(const std::string&name):
    name_(name),
    level_(LogLevel::DEBUG){
        formatter_.reset(new LogFormatter("%d [%N]:%t %F [%p]  [%c] <%f :%l> %m %n"));
    }

    void Logger::log(LogLevel::Level level,const LogEvent::ptr event){
           RWMutex::ReLockGuard lock(mutex_);
            if(level>=level_){
                auto self=shared_from_this();
                if(!appenders_.empty())
                {
                    for(const LogAppender::ptr&appender:appenders_){
                        appender->log(self,level,event);
                        }
                }else{
                    root_->log(level,event);
                }
            }
    }

    void Logger::debug(LogEvent::ptr event){
            log(LogLevel::DEBUG,event);
    }

    void Logger::info(LogEvent::ptr event){
         log(LogLevel::INFO,event);
    }

    void Logger::warn(LogEvent::ptr event){
         log(LogLevel::WARN,event);
    }

    void Logger::error(LogEvent::ptr event){
         log(LogLevel::ERROR,event);
    }

    void Logger::fatal(LogEvent::ptr event){
         log(LogLevel::FATAL,event);
    }

    void Logger::clearAppender(){
        RWMutex::WrLockGuard lock(mutex_);
        appenders_.clear();
    }

    void Logger::addAppender(LogAppender::ptr appender){
    RWMutex::WrLockGuard lock(mutex_);
        if(!appender->getFormatter()){
            appender->setFormatter(this->formatter_);
        }
        appenders_.push_back(appender);
    }

    void Logger::deleteAppender(LogAppender::ptr appender){
       RWMutex::WrLockGuard lock(mutex_);
        auto it=std::find(appenders_.begin(),appenders_.end(),appender);
        if(it!=appenders_.end())
        appenders_.erase(it);
    }
   void StdoutAppender::log(std::shared_ptr<Logger>logger,
   LogLevel::Level& level,LogEvent::ptr event) {
            if(level>=level_){
                std::cout<<formatter_->format(logger,level,event);
            }
    }

    FileAppender::FileAppender(const std::string&Filename):
    fileName_(Filename){
        reopen();
    }

    bool FileAppender::reopen(){
        if(fileStream_){
            fileStream_.close();
        }
        fileStream_.open(fileName_,std::ios::app);
        return !fileStream_;
    }

void FileAppender::log(std::shared_ptr<Logger>logger,
      LogLevel::Level& level,LogEvent::ptr event) {
           LogFormatter::ptr copyOfFormatter_;
          {
              MutexLockGuard lock(mutex_);
              copyOfFormatter_=formatter_;
          }
         if(level>level_){
             fileStream_<<copyOfFormatter_->format(logger,level,event);
         }
     }  

LogFormatter::LogFormatter(const std::string&pattern):
        pattern_(pattern)
    {
        init();
    }
std::string LogFormatter::format(std::shared_ptr<Logger>logger,
     LogLevel::Level level,LogEvent::ptr event){
            std::stringstream ss;
            for(const auto&item:items_){
                item->format(ss,logger,level,event);
            }
            return ss.str();
     }

    class MessageFormatItem:public LogFormatter::FormatItem{
        public:
         MessageFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<event->getContent();
        }
    };
   class LevelFormatItem:public LogFormatter::FormatItem{
        public:
         LevelFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<LogLevel::ToString(level);
        }
    };
   class ElapseFormatItem:public LogFormatter::FormatItem{
        public:
         ElapseFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<event->getElapse();
        }
    };
  class LoggerNameFormatItem:public LogFormatter::FormatItem{
        public:
        LoggerNameFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<logger->getName();
        }
    };
  class ThreadIdFormatItem:public LogFormatter::FormatItem{
        public:
        ThreadIdFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<event->getThread();
        }
    };
  class FiberIdFormatItem:public LogFormatter::FormatItem{
        public:
        FiberIdFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<event->getFiberId();
        }
    };
  class DateTimeFormatItem:public LogFormatter::FormatItem{
        public:
        DateTimeFormatItem(const std::string &dateFormat="%Y:%m:%d %H:%M:%S"):
        dateFormat_(dateFormat){    
                if(dateFormat.empty()){
                    dateFormat_="%Y:%m:%d %H:%M:%S";
                }
        }
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            struct tm tm;
            time_t time=event->getTime();
            localtime_r(&time,&tm);
            char buf[64];
            strftime(buf,sizeof(buf),dateFormat_.c_str(),&tm);
            os<<buf;
        }
        private:
        std::string dateFormat_;
    };
    class FileNameFormatItem:public LogFormatter::FormatItem{
        public:
        FileNameFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<event->getFile();
        }
    };
class LineFormatItem:public LogFormatter::FormatItem{
        public:
        LineFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<event->getLine();
        }
    };
class NewLineFormatItem:public LogFormatter::FormatItem{
        public:
        NewLineFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<std::endl;
        }
};
class StringFormatItem:public LogFormatter::FormatItem{
        public:
        StringFormatItem(const std::string &str=""):
       string_(str){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<string_;
        }
        private:
        std::string string_;
};
class ThreadFormatItem:public LogFormatter::FormatItem{
public:
        ThreadFormatItem(const std::string &str=""){}
        void format(std::ostream&os,Logger::ptr logger,LogLevel::Level level,
        LogEvent::ptr event){
            os<<event->getThreadName();
        }
};
void LogFormatter::init(){
            //str format type
            std::vector<std::tuple<std::string,std::string,int>>vec;
            std::string nstr;
            for(size_t i=0;i<pattern_.size();++i){
                if(pattern_[i]!='%'){
                    nstr.append(1,pattern_[i]);
                    continue;
                }
                if(i+1<pattern_.size())
                {
                    if(pattern_[i+1]=='%'){
                        nstr.append(1,'%');
                        continue;
                    }
                }
                size_t n=i+1;
                short status_=0;
                size_t fmt_begin=0;
                std::string str;
                std::string fmt;
                while(n<pattern_.size()){
                    if(!isalpha(pattern_[n])
                    &&pattern_[n]!='{'
                    &&pattern_[n]!='}')break;
                    if(status_==0){
                        if(pattern_[n]=='{'){
                            str=pattern_.substr(i+1,n-i-1);
                            status_=1;//解析格式
                            fmt_begin=n;
                            ++n;
                            continue;
                        }
                    }
                    if(status_==1){
                            if(pattern_[n]=='}'){
                                fmt=pattern_.substr(fmt_begin+1,n-fmt_begin-1);
                                status_=2;//结束
                                break;
                            }
                        }
                        ++n;
                }
                if(status_==0){
                    if(!nstr.empty()){
                        vec.push_back(std::make_tuple(nstr,"",0));
                        nstr.clear();
                    }
                    str=pattern_.substr(i+1,n-i-1);
                    vec.push_back(std::make_tuple(str,fmt,1));
                    i=n-1;
                }else if(status_==1){
                    vec.push_back(std::make_tuple("pattern error",fmt,0));
                    std::cout<<"pattern parse error: "<<pattern_<<" ----"<<pattern_.substr(i)
                    <<std::endl;
                    if_error=true;
                }else if(status_==2){
                    if(!nstr.empty()){
                        vec.push_back(std::make_tuple(nstr,"",0));
                        nstr.clear();
                    }
                    vec.push_back(std::make_tuple(str,fmt,2));
                    i=n-1;
                }
            }
             if(!nstr.empty()){
                vec.push_back(std::make_tuple(nstr,"",0));
            }
            static std::map<std::string,std::function<FormatItem::ptr(const std::string&str)>>
            s_format_items={
                #define XX(str,C) \
                {#str,[](const std::string &fmt){ return FormatItem::ptr(new C(fmt));}}
                XX(m,MessageFormatItem),
                XX(p,LevelFormatItem),
                XX(r,ElapseFormatItem),
                XX(c,LoggerNameFormatItem),
                XX(t,ThreadIdFormatItem),
                XX(n,NewLineFormatItem),
                XX(d,DateTimeFormatItem),
                XX(f,FileNameFormatItem),
                XX(l,LineFormatItem),
                XX(F,FiberIdFormatItem),
                XX(N,ThreadFormatItem),
                #undef XX
            };
            for(const auto &i :vec){
                if(std::get<2>(i)==0){
                    items_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
                }
                else{
                    auto it=s_format_items.find(std::get<0>(i));
                    if(it==s_format_items.end()){
                      items_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
                      if_error=true;
                    }else{
                          items_.push_back(it->second(std::get<1>(i)));
                    }
                }
            }
            //%m 消息体
            //%p  level
            //%r   启动后的时间
            //%c   日志名称
            //%t    县城id
            //%n   回车
            //%d   时间戳
            //%f    文件名
            //%l    行号
            //%N 线程name
     }
    const char*LogLevel::ToString(LogLevel::Level level){
            switch (level){
                #define XX(name) \
                case LogLevel::name: \
                return #name; \
                break;
                XX(DEBUG);
                XX(INFO);
                XX(WARN); 
                XX(ERROR);
                XX(FATAL);
                #undef XX
                default:
                return "UNKNOW";
            }
            return "UNKNOW";
     }
    LogLevel::Level LogLevel::fromString(const std::string&val){
            #define XX(value)   \
            if(val==#value) \
            {       \
                return Level::value;  \
            }   
            XX(DEBUG);
            XX(INFO);
            XX(WARN); 
            XX(ERROR);
            XX(FATAL);
            #undef XX
            return LogLevel::UNKNOW;
     }
    Logger::ptr LoggerManager::getLogger(const std::string&name){
        auto it=loggers_.find(name);
        if(it!=loggers_.end())return it->second;
        auto val=Logger::ptr(new Logger(name));
        val->root_=root_;
        loggers_[name]=val;
        return val;
    }
    void LoggerManager::log(const LogLevel::Level& level,const LogEvent::ptr event ){
       sylar::RWMutex::ReLockGuard lock(LoggerManager::getMutexLock());
        for(auto&val:loggers_){
            event->setLogger(val.second);
            val.second->log(level,event);
        }
    }
    void LoggerManager::init(){
        
    }
    sylar::ConfigVar<std::set<LogDefiner>>::ptr Log_Config=sylar::Config::Lookup("logs",
                                                                            std::set<LogDefiner>{},
                                                                             "log");
Loginit::Loginit(){
        
        Log_Config->addValChangeCB([](
           const  std::set<LogDefiner>&old_val,
           const    std::set<LogDefiner>&new_val
        ){
            //新增
            //修改
            //删除
            sylar::RWMutex::WrLockGuard lock(LoggerManager::getMutexLock());
            for(auto &it:new_val){
                Logger::ptr logger;
                auto i=old_val.find(it);
              SYLAR_LOG_DEBUG(LOGGER_DEBUG_(lqf))<<"reset LogDefiner";
                if(i==old_val.end()){
                    logger.reset(new Logger(it.name));
                }else{
                    //修改
                       logger=LoggerManager::LookUp(it.name);   
                }
                logger->setLevel(it.level);
                if(!it.formatter_.empty())
                logger->setFormatter(it.formatter_);    
                logger->clearAppender();
                for(auto&val:it.appenders_){
                        LogAppender::ptr ap;
                        if(val.type==2){
                            ap.reset(new FileAppender(val.file));
                        }else if(val.type==1){
                            ap.reset(new StdoutAppender());
                        }
                        ap->setLevel(val.level);
                        logger->addAppender(ap);
                }
                if(i==old_val.end())
              LoggerManager::AddLogger(it.name,logger);
            }
            //删除
            for(auto it:old_val){
                auto i=new_val.find(it);
                if(i==new_val.end()){
                    //删除
                auto logger=SYLAR_LOGGER_NAME(i->name);
                logger->clearAppender();
                logger->setLevel(LogLevel::del_flag);
                }
            }
        });
       // Config::init();
    }
    static Loginit Log_Config_init;
    LogManagerWrap::~LogManagerWrap(){
        SingLeton<sylar::LoggerManager>::getInstence().log(event_->GetLevel(),event_);
    }
     std::map<std::string,Logger::ptr>LoggerManager::loggers_;
     Logger::ptr LoggerManager::root_;
     RWMutex LoggerManager::mutex_;
}
    // const char*file_=nullptr;//文件名
    // int32_t elapse_;            //程序启动到现在的毫秒数
    // int32_t line_=0;            //行号
    // int32_t threadId_=0;      //线程号
    // uint32_t fiberId_=0;        //协程
    // time_t time_;                   //时间戳
    // std::string content_;          //内容