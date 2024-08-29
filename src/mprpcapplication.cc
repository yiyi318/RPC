#include "mprpcapplication.h"
#include <unistd.h>
#include <iostream>

MprpcConfig MprpcApplication::m_config;//静态成员变量要在.cc文件中定义

void ShowArgsHelp()
{
    std::cout<<"format: command -i <configfile>" <<std::endl;
}

void MprpcApplication::Init(int argc,char **argv)
{
    if(argc<2){
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c=0;
    std::string config_file;
    while((c=getopt(argc,argv,"i: "))!=-1)//当然！假设我们有一个程序，它接受一个 -i 选项，并且这个选项后面需要跟一个参数。我们希望通过命令行来提供这个选项和参数。
    {
        switch(c)
        {
        case 'i':
            config_file=optarg;
            break;
        case '?':
            //std::cout<<"invalid args!"<<std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            //std::cout<<"need <configfile>!"<<std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    //开始加载配置文件了 rpcserver_ip=  rpcserver_port=  zookeeper_ip= zookeeper_port=
      m_config.LoadConfigFile(config_file.c_str());
//    std::cout<<"rpcserverip:"<<m_config.Load("rpcserverip")<<std::endl;
//    std::cout<<"rpcserverport:"<<m_config.Load("rpcserverport")<<std::endl;
//    std::cout<<"zookeeperip:"<<m_config.Load("zookeeperip")<<std::endl;
//    std::cout<<"zookeeperport:"<<m_config.Load("zookeeperport")<<std::endl;
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}
