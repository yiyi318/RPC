#include<iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"



int main(int argc,char **argv)//接入传入的参数
{
    //整个程序启动以后 想使用mprp框架来
    MprpcApplication::Init(argc,argv);

    fixbug::GetFriendsListServiceRpc_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    fixbug::GetFriendsListRequest request;

    //传入的参数都是我自己设计的
    request.set_userid(1000);
    //stub.Login();
    fixbug::GetFriendsListResponse response;
    //发起rpc方法的调用 同步rpc调用过程 MprpcChannel::callmethod

    MprpcController controller;
    //发起rpc方法的嗲偶哦那个 同步的rpc调用过程
    stub.GetFriendsListLogin(nullptr ,&request, &response, nullptr);
    //一次rpc调用完成 读调用的结果
    if(controller.Failed())
    {
        std::cout<<controller.ErrorText()<<std::endl;
    }
    else{
        if(0==response.result().errcode())
        {
            std::cout<<"rpc GetFriendsLis response success: "<<std::endl;
            int size=response.friends_size();
            for(int i=0;i<size;i++){
                std::cout<<"index: "<<(i+1)<<"name: "<<response.friends(i)<<std::endl;
            }

        }
        else{
            std::cout<<"rpc GetFriendsLis response error"<<response.result().errmsg()<<std::endl;
        }

    }




    return 0;

}