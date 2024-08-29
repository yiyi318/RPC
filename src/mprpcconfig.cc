#include "mprpcconfig.h"
#include<string>
#include <iostream>

//负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf=fopen(config_file,"r");
    if(nullptr==pf){
        std::cout<<config_file<<"is not exist!" <<std::endl;
        exit(EXIT_FAILURE);
    }
    //可以存在1 注释  2 正确的配置项 3 去掉开头的多余的空格
    while(!feof(pf))
    {
        char buf[512]={0};
        fgets(buf,512,pf);
        //去掉字符串前面多余的空格
        std::string read_buf(buf);
        Trim(read_buf);
        //判断#的注释
        if(read_buf[0]=='#' || read_buf.empty())
        {
            continue;
        }

        int idx=read_buf.find_first_not_of(' ');
        if(idx !=-1)
        {
            //说明字符串前面有空格
            read_buf=read_buf.substr(idx,read_buf.size()-idx);
        }
        //去掉字符串后面多余的空格
        idx=read_buf.find_last_not_of(' ');
        if(idx!=-1)
        {
            //说明字符串后面有空格
            read_buf=read_buf.substr(0,idx+1);
        }


        //判断配置项
        idx=read_buf.find('=');
        if(idx==-1)
        {
            //配置项不合法
            continue;
        }

        std::string key;
        std::string value;
        key=read_buf.substr(0,idx);
        Trim(key);

        int endidx = read_buf.find('\n', idx);  // 查找换行符位置
        if (endidx == std::string::npos) {
            // 如果没有找到换行符，处理边界情况
            endidx = read_buf.size();
        }

        // 提取子串
        value = read_buf.substr(idx + 1, endidx - idx - 1);

        // 清理回车符
        value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
        Trim(value);
        m_configMap.insert({key,value});

    }
}

//查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    if(m_configMap.find(key)==m_configMap.end()){
        return " ";
    }
    return m_configMap[key];
}

//去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
        int idx=src_buf.find_first_not_of(' ');
        if(idx !=-1)
        {
            //说明字符串前面有空格
            src_buf=src_buf.substr(idx,src_buf.size()-idx);
        }
        //去掉字符串后面多余的空格
        idx=src_buf.find_last_not_of(' ');
        if(idx!=-1)
        {
            //说明字符串后面有空格
            src_buf=src_buf.substr(0,idx+1);
        }
}