#include"../sylar/sylar.h"
#include<sys/socket.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
void test_IOManager(){
    LOG_DEBUG<<"test_IOManager";
}
sylar::Timer::ptr timer;
void test(){
     sylar::IOManager iom{10};
    timer=iom.addTimer(1000,[](){
         static int i=1;
        LOG_DEBUG<<"timer"<<i;
         if(++i>10)
         timer->reset(500,true);
     },true);
}
void test2(){
    sylar::IOManager iom{10};
    int sock=socket(AF_INET,SOCK_STREAM,0);
    fcntl(sock,F_SETFL,O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(80);
    inet_pton(AF_INET,"39.156.66.18",&addr.sin_addr.s_addr);
    if(!connect(
        sock,(sockaddr*)&addr,sizeof(addr)
    )){

    }else if(errno==EINPROGRESS){
        sylar::IOManager::GetCurrentIOManager()->addEvent(
            sock,sylar::IOManager::Event::READD,[](){
                LOG_DEBUG<<"READ event";
            }
        );
           sylar::IOManager::GetCurrentIOManager()->addEvent(
            sock,sylar::IOManager::Event::WRITE,[=](){
           bool f= sylar::IOManager::GetCurrentIOManager()->cancelEvent(sock,sylar::IOManager::READD);
            LOG_DEBUG<<"WRITE event";;
            close(sock);
            }
        );

    }else{
         LOG_DEBUG<<"error";
    }

}
int main(){
    YAML::Node root=YAML::LoadFile("/home/lqf/cpp/sylar/conf/config.YAML");
    sylar:: Config::LoadFromYaml(root);

    test();
    return 0;
}