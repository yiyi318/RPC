#pragma once
#include "mprpcconfig.h"
#include"mprpccontroller.h"
#include"mprpcchannel.h"
#include"zookeeperutil.h"
//mprpc框架的初始化类
class MprpcApplication
{
public:
    static void Init(int argc,char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;
    MprpcApplication(){}//默认私有化初始化
    MprpcApplication(const MprpcApplication&)=delete;//拷贝构造被删除
    MprpcApplication(MprpcApplication&&)=delete;//赋值构造被删除
};