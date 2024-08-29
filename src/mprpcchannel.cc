#include"mprpcchannel.h"
#include<string>
#include"rpcheader.pb.h"
#include<sys/types.h>
#include<sys/socket.h>
#include <errno.h>
#include "mprpcapplication.h"
#include"arpa/inet.h"
#include<unistd.h>


void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,//传入服务对象，服务对象函数名称
                        google::protobuf::RpcController * controller,
                        const google::protobuf::Message *request,
                        google::protobuf::Message * response,
                        google::protobuf::Closure* done)
{
//传入服务名称，方法名称
    const google::protobuf::ServiceDescriptor*sd =method->service();
    std::string service_name =sd->name();//服务名称
    std::string method_name =method->name();//方法名称

    //获取参数的序列化字符串长度
    int args_size=0;
    std::string args_str;
    if(request->SerializeToString(&args_str))
    {
        args_size=args_str.size();
    }
    else{
        controller->SetFailed("serialize request error!");
        return;
    }

    //定义rpc的请求header //构建一个rpcheader
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size=0;
    std::string rpc_header_str;//将传入的参数进行序列化
    if(rpcHeader.SerializeToString(&rpc_header_str)){
        header_size=rpc_header_str.size();
    }
    else{
        controller->SetFailed(" serialize rpc header error!");
        return;
    }

    //组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char*)&header_size,4));//header_size
    send_rpc_str+=rpc_header_str;
    send_rpc_str+=args_str;

    //打印调试信息
    std::cout<<"=========================================="<<std::endl;
    std::cout<<"header_size"<<header_size<<std::endl;
    std::cout<<"rpc_header_str"<<rpc_header_str<<std::endl;
    std::cout<<"service_name"<<service_name<<std::endl;
    std::cout<<"method_name"<<method_name<<std::endl;
    std::cout<<"args_str"<<args_str<<std::endl;
    std::cout<<"=========================================="<<std::endl;

    //使用tcp编程 完成远程调用  申请一个套接字
    int clientfd=socket(AF_INET, SOCK_STREAM, 0);

    if(-1==clientfd)
    {
        char errtext[512]={0};
        sprintf(errtext,"create socket error! errno:%d",errno);
        controller->SetFailed(errtext);
        return;
    }


    //std::string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    //uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr=inet_addr(ip.c_str());

    //连接rpc服务节点  绑定套接字到rpcserver服务方
    if(-1==connect(clientfd,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        char errtext[512]={0};
        sprintf(errtext,"connect error! errno:%d",errno);
        controller->SetFailed(errtext);
        return;
    }

    //发送rpc请求
    if(-1==send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0))
    {
        char errtext[512]={0};
        sprintf(errtext,"send error! errno:%d",errno);
        controller->SetFailed(errtext);
        return;
    }

    //接收rpc请求的响应值
    char recv_buf[1024]={0};
    int recv_size=0;
    if(-1==(recv_size=recv(clientfd,recv_buf,1024,0)))
    {
        char errtext[512]={0};
        sprintf(errtext,"recv error! errno: %d",errno);
        controller->SetFailed(errtext);
        return;

    }

//反序列化rpc调用的响应数量
    if(!response->ParseFromArray(recv_buf,recv_size))
    {
        char errtext[512]={0};
        sprintf(errtext,"parse error! response_str:%d",errno);
        controller->SetFailed(errtext);
        return;
    }
    close(clientfd);

}