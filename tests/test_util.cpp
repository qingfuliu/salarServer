#include"../sylar/sylar.h"
void test_backTrace(){
   ASSERT1(0==1,"test_assert1");
}
int main(){
    YAML::Node root=YAML::LoadFile("/home/lqf/cpp/sylar/conf/config.YAML");
    sylar::Config::LoadFromYaml(root);
    test_backTrace();
}