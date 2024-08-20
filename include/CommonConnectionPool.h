#pragma once
#include <string>
#include <queue>
#include <iostream>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <memory>
#include <functional>
#include "Connection.h"
using namespace std;
/*
实现连接池功能
*/

class ConnectionPool
{
public:
    //获取连接池对象实例
    static ConnectionPool* getConnectionPool();
    //给外部提供接口，从连接池中获取一个可用的空闲链接
    shared_ptr<Connection> getConnection();
    
    bool loadConfigFile();//从配置文件中加载配置项
private:
    //static ConnectionPool pool;
    ConnectionPool();

    //运行在独立的线程中，专门负责生产新链接
    void produceConnectionTask();

    //扫描多余的空闲连接，超过maxIdleTime时间的空闲连接，进行多余的连接回收
    void scannerConnectionTask();

    string _ip; //mysql的ip地址
    unsigned short _port;   //mysql的端口号 3306
    string _username;   //mysql登陆用户名
    string _password;   //mysql登录密码
    string _dbname;     //数据库名字（不是数据表）
    int _initSize;      //连接池的初始连接量
    int _maxSize;       //连接池的最大连接量
    int _maxIdleTime;   //连接池的最大空闲时间
    int _connectionTimeout; //连接池获取连接的超时时间

    queue<Connection*> _connectionQue; //存储mysql连接的队列
    mutex _queueMutex; //维护连接队列的线程安全互斥锁
    atomic_int _connectionCnt; //记录连接所创建的connection连接的总数量
    condition_variable cv; //设置条件变量，用于连接生产线程和消费线程的通信
};