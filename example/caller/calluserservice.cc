#include<iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include"mprpcchannel.h"


int main(int argc,char **argv)
{
    //整个程序启动以后 想使用mprp框架来
    MprpcApplication::Init(argc,argv);//初始化 注册信息，将服务注册到节点上

    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    //stub.Login();
    fixbug::LoginResponse response;
    //发起rpc方法的嗲偶哦那个 同步的rpc调用过程
    stub.Login(nullptr ,&request, &response, nullptr);
    //一次rpc调用完成 读调用的结果
    if(0==response.result().errcode())
    {
        std::cout<<"rpc login response success: "<<response.sucess()<<std::endl;
    }
    else{
        std::cout<<"rpc login response error"<<response.result().errmsg()<<std::endl;
    }

    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("66666666");
    fixbug::RegisterResponse rsp;
    //已同步的方式发起rpc调用请求，等待返回结果
    stub.Register(nullptr,&req,&rsp,nullptr);

    //一次rpc调用完成，读调用的结果
    if(0==rsp.result().errcode())
    {
        std::cout<<"rpc register response success:"<<rsp.sucess()<<std::endl;
    }
    else{
        std::cout<<"rpc register response error: "<<rsp.result().errmsg()<<std::endl;
    }
    return 0;

}