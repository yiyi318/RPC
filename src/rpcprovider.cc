#include"rpcprovider.h"
#include "mprpcconfig.h"
#include "logger.h"
//这里是框架提供给外部使用的，可以发布rpc方法的函数接口

void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc =service->GetDescriptor();
    //获取服务的名字
    std::string service_name=pserviceDesc->name();
    //获取服务对象servuice的方法的数量
    int methodCnt=pserviceDesc->method_count();

    //std::cout<<"service_name:" <<service_name <<std::endl;
    LOG_INFO("service_name:%s",service_name.c_str());
    for(int i=0;i<methodCnt;i++){
        //获取服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor* pmethodDesc=pserviceDesc->method(i);
        std::string method_name =pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc});

        //std::cout<<"method_name: "<<method_name<<std::endl;
        LOG_INFO("method_name:%s",method_name.c_str());
    }

    service_info.m_service=service;
    m_serviceMap.insert({service_name,service_info});

}


//启动rpc服务节点
void  RpcProvider::Run()
{
    std::string ip=MprpcApplication::GetInstance().GetConfig().Load("userserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("userserverport").c_str());
    muduo::net::InetAddress address(ip,port);
    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"userProvider");
    //组合了TcpServer
    //绑定连接回调和小气读写回调方法 发呢里了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    //设置muduo库的线程数量
    server.setThreadNum(4);

    ZkClient zkCLi;
    zkCLi.Start();
    //service_name为永久性节点 method_name为临时性节点
    for( auto &sp : m_serviceMap)
    {
        //service_name
        std::string service_path="/"+sp.first;
        zkCLi.Create(service_path.c_str(),nullptr,0);//创建服务对象节点
        for(auto &mp :sp.second.m_methodMap)
        {
            std::string method_path =service_path+"/"+mp.first;
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            zkCLi.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
        std::cout<<"RpcProvider start service at ip: "<<ip<<"port"<<port<<std::endl;
    }

    server.start();
    m_eventLoop.loop();
}
//新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if(!conn->connected())
    {
        //和rpc client的连接断开了
        conn->shutdown();

    }

}
/*
在框架内 RpcProvider和RpcConsumer协商好之间通信使用的protobuf数据类型
service_name method_name args 定义proto的message类型 进行数据头的序列化和反序列化
                                service_name method_name args_size
UserService Login zhang san 123456

header_size(4个字节) + header_str + args_str
*/
//已经建立连接用户的读写事件回调 如果远程有有一个rpc服务的调用请求 那么onmessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr&conn,
                            muduo::net::Buffer* buffer,
                            muduo::Timestamp time)
{
    //网络上接收的远程rpc调用请求的字符流 Login args
    std::string recv_buf =buffer->retrieveAllAsString();
    //从字符流中读取前4个字节的内容
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);

    //更具header_size读取数据头的原始字符流
    std::string rpc_header_str =recv_buf.substr(4,header_size);

    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;

    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列成功
        service_name=rpcHeader.service_name();
        method_name =rpcHeader.method_name();
        args_size=rpcHeader.args_size();
    }
    else{
        //数据头反序列话化失败
        std::cout<<"rpc_header_str"<<rpc_header_str<<"parse error!"<<std::endl;
        return;
    }
    //获取rpc方法参数的字符流数据
    std::string  args_str=recv_buf.substr(4+header_size,args_size);
    //打印调试信息
    std::cout<<"=========================================="<<std::endl;
    std::cout<<"header_size"<<header_size<<std::endl;
    std::cout<<"rpc_header_str"<<rpc_header_str<<std::endl;
    std::cout<<"service_name"<<service_name<<std::endl;
    std::cout<<"method_name"<<method_name<<std::endl;
    std::cout<<"args_str"<<args_str<<std::endl;
    std::cout<<"=========================================="<<std::endl;

    //获取service对象和method对象
    auto it =m_serviceMap.find(service_name);
    if(it==m_serviceMap.end()){
        std::cout<<service_name<<"is not exist!"<<std::endl;
        return;
    }
    google::protobuf::Service*service=it->second.m_service;//获取service对象
    auto mit=it->second.m_methodMap.find(method_name);
    if(mit==it->second.m_methodMap.end())
    {
        std::cout<<service_name<<": "<<method_name<<"is not exist!"<<std::endl;
        return;
    }


    const google::protobuf::MethodDescriptor*method =mit->second;//获取method对象
    //生成rpc方法调用的请i去request和响应response参数
    google::protobuf::Message* request =service->GetRequestPrototype(method).New();
    request->ParseFromString(args_str);
    if(!request->ParseFromString(args_str))
    {
        std::cout<<"request parse error, content: "<<args_str<<std::endl;
        return;
    }

    google::protobuf::Message* response=service->GetResponsePrototype(method).New();
    //给下面的method方法的调用，绑定一个Closure的方法
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr&,
                                                                    google::protobuf::Message*>
                                                                    (this,
                                                                    &RpcProvider::SendRpcResponse,
                                                                    conn,response);
    //在框架上更具远程rpc请求，调用当前rpc节点上发布的方法
    //new UserService().Login(controller,request,response,done)
    service->CallMethod(method,nullptr,request,response,done);
}

//closure的回调操作 用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr&conn,google::protobuf::Message* response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str))//对response进行序列化
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送给rpc的调用方
        conn->send(response_str);

    }
    else{
        std::cout<<"serialize response_str errot!"<<std::endl;
    }
    conn->shutdown();//模拟http的短链接服务，由rpcprovider主动断开连接
}