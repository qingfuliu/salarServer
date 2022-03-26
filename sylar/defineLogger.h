#ifndef __DEFINELOGGER_H
#define __DEFINELOGGER_H
#include"log.h"
#include"Singleton.h"
#include"util.h"
#include"thread.h"
#include<time.h>
#define SYLAR_LOG_LEVEL(logger,level) \
            if(level>=logger->getLevel())    \
            sylar::LogWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger,level,__FILE__,0,__LINE__,  \
            sylar::GetThreadId(),sylar::GetFiberId(),time(0),sylar::GetThreadName()))).getStringStream()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::INFO)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::FATAL)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::WARN)

#define LOGGER_DEBUG_ROOT SingletonLogger<sylar::LogLevel::DEBUG>::getInstence()
#define LOGGER_INFO_ROOT SingletonLogger<sylar::LogLevel::INFO>::getInstence()
#define LOGGER_FATAL_ROOT SingletonLogger<sylar::LogLevel::FATAL>::getInstence()
#define LOGGER_WARN_ROOT SingletonLogger<sylar::LogLevel::WARN>::getInstence()
#define LOGGER_ERROR_ROOT SingletonLogger<sylar::LogLevel::ERROR>::getInstence()

#define FILELOGGER_DEBUG_ROOT SingletonLogger<sylar::LogLevel::DEBUG,sylar::FileAppender>::getInstence()
#define FILRLOGGER_INFO_ROOT SingletonLogger<sylar::LogLevel::INFO,sylar::FileAppender>::getInstence()
#define FILRLOGGER_FATAL_ROOT SingletonLogger<sylar::LogLevel::FATAL,sylar::FileAppender>::getInstence()
#define FILRLOGGER_WARN_ROOT SingletonLogger<sylar::LogLevel::WARN,sylar::FileAppender>::getInstence()
#define FILRLOGGER_ERROR_ROOT SingletonLogger<sylar::LogLevel::ERROR,sylar::FileAppender>::getInstence()

#define SYLAR_FILELOG_WARN_ROOT SYLAR_LOG_LEVEL(FILELOGGER_DEBUG_ROOT,sylar::LogLevel::WARN)
#define SYLAR_FILELOG_DEBUG_ROOT SYLAR_LOG_LEVEL(FILELOGGER_DEBUG_ROOT,sylar::LogLevel::DEBUG)
#define SYLAR_FILELOG_INFO_ROOT SYLAR_LOG_LEVEL(FILELOGGER_DEBUG_ROOT,sylar::LogLevel::INFO)
#define SYLAR_FILELOG_FATAL_ROOT SYLAR_LOG_LEVEL(FILELOGGER_DEBUG_ROOT,sylar::LogLevel::FATAL)
#define SYLAR_FILELOG_ERROR_ROOT SYLAR_LOG_LEVEL(FILELOGGER_DEBUG_ROOT,sylar::LogLevel::ERROR)

#define LOGGER_DEBUG_(name) \
 SingletonLogger<sylar::LogLevel::DEBUG>::getInstence(#name)
#define LOGGER_INFO_(name) \
SingletonLogger<sylar::LogLevel::INFO>::getInstence(#name)
#define LOGGER_FATAL_(name)  \
SingletonLogger<sylar::LogLevel::FATAL>::getInstence(#name)
#define LOGGER_WARN_(name)  \
SingletonLogger<sylar::LogLevel::WARN>::getInstence(#name)
#define LOGGER_ERROR_(name)  \
SingletonLogger<sylar::LogLevel::ERROR>::getInstence(#name)


#define FILRLOGGER_DEBUG_(name) \
 SingletonLogger<sylar::LogLevel::DEBUG,sylar::FileAppender>::getInstence(#name)
#define FILRLOGGER_INFO_(name) \
SingletonLogger<sylar::LogLevel::INFO,sylar::FileAppender>::getInstence(#name)
#define FILRLOGGER_FATAL_(name)  \
SingletonLogger<sylar::LogLevel::FATAL,sylar::FileAppender>::getInstence(#name)
#define FILRLOGGER_WARN_(name)  \
SingletonLogger<sylar::LogLevel::WARN,sylar::FileAppender>::getInstence(#name)
#define FILRLOGGER_ERROR_(name)  \
SingletonLogger<sylar::LogLevel::ERROR,sylar::FileAppender>::getInstence(#name)

#define SYLAR_FILELOG_WARN_(name) SYLAR_LOG_LEVEL(FILRLOGGER_WARN_(name),sylar::LogLevel::WARN)
#define SYLAR_FILELOG_DEBUG_(name) SYLAR_LOG_LEVEL(FILELOGGER_DEBUG_(name),sylar::LogLevel::DEBUG)
#define SYLAR_FILELOG_INFO_(name) SYLAR_LOG_LEVEL(FILELOGGER_INFO_(name),sylar::LogLevel::INFO)
#define SYLAR_FILELOG_FATAL_(name) SYLAR_LOG_LEVEL(FILELOGGER_FATAL_(name),sylar::LogLevel::FATAL)
#define SYLAR_FILELOG_ERROR_(name) SYLAR_LOG_LEVEL(FILELOGGER_ERROR_(name),sylar::LogLevel::ERROR)

#define SYLAR_LOGGER_NAME(XX)   \
           SingLeton<sylar::LoggerManager>::getInstence().LookUp(XX)

#define SYLAR_LOGGER   \
           SingLeton<sylar::LoggerManager>::getInstence()

#define LOG_STRINGSTREAM_(level)    \
      sylar::LogManagerWrap(sylar::LogEvent::ptr(new sylar::LogEvent(nullptr,level,__FILE__,0,__LINE__,  \
            sylar::GetThreadId(),sylar::GetFiberId(),time(0),sylar::Thread::GeCurrentName()))) 
#define LOG_DEBUG   LOG_STRINGSTREAM_(sylar::LogLevel::DEBUG)
#define LOG_INFO       LOG_STRINGSTREAM_(sylar::LogLevel::INFO)
#define LOG_FATAL  LOG_STRINGSTREAM_(sylar::LogLevel::FATAL)
#define LOG_ERROR   LOG_STRINGSTREAM_(sylar::LogLevel::ERROR)
#define LOG_WARN   LOG_STRINGSTREAM_(sylar::LogLevel::WARN)

#define ASSERT(x) \
            if(!(x)){          \
            LOG_FATAL<<"\nASSERT FATAL: "#x        \
            <<"\nbackTrace:   \n"<<     \
            sylar::BackTraceToString(10);       \
            assert(x);   \
            }

#define ASSERT1(x,description) \
            if(!(x)){          \
            LOG_FATAL<<"\nASSERT FATAL: "#x<<"\ndescription:"<<description        \
            <<"\nbackTrace:   \n"<<     \
            sylar::BackTraceToString(10);       \
            assert(x);   \
            }
#endif