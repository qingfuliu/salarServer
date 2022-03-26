#ifndef __UTIL_LOG_H
#define __UTIL_LOG_H
#include<pthread.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<stdint.h>
#include<assert.h>
#include"string"
#include<vector>
namespace sylar{

pid_t GetThreadId();

uint64_t GetFiberId();

std::string GetThreadName();

void BackTrace(std::vector<std::string>&strs_,int size,int skip);

std::string BackTraceToString(int size,int skip=2,std::string prefix="");

uint64_t GetCurrentMs();
}
#endif