#include "CommonConnectionPool.h"
#include "public.h"


//线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool; //lock和unlock
    return &pool;
}


bool ConnectionPool::loadConfigFile()
{
    FILE *pf = fopen("/home/fdl/桌面/ConnectionPool/src/mysql.conf", "r");
    if(pf == NULL)
    {
        LOG("mysql.init file is not exist!");
        return false;
    }

    while(!feof(pf))
    {
        char line[1024] = {0};
        fgets(line,1024,pf);
        string str = line;
        int idx = str.find('=',0);
        if(idx == -1) //无效的配置项
        {
            continue;
        }
        int endidx = str.find('\n',idx);
        string key = str.substr(0,idx);
        string value = str.substr(idx+1,endidx - idx -1);

        if(key == "ip")
        {
            _ip = value;
        }else if(key == "port")
        {
            _port = atoi(value.c_str());
        }else if(key == "username")
        {
            _username = value;
        }else if(key == "dbname"){
            _dbname = value;
        }
        else if(key == "password")
        {
            _password = value;
        }else if(key ==  "initSize")
        {
            _initSize = atoi(value.c_str());
        }else if(key == "maxSize")
        {
            _maxSize = atoi(value.c_str());
        }else if(key == "maxIdleTime")
        {
            _maxIdleTime = atoi(value.c_str());
        }else if(key == "connectionTimeout")
        {
            _connectionTimeout = atoi(value.c_str());
        }
    
        cout << endl;
    }
    return true;
}

ConnectionPool::ConnectionPool()
{
    //加载配置项
    if(!loadConfigFile())
    {
        return;
    }
    //创建初始数量的连接
    for(int i =0; i < _initSize; ++i)
    {
        Connection *p = new Connection();
        p->connect(_ip, _port,_username,_password,_dbname);
        p->refreshAliveTime(); //刷新一下开始空闲的起始时间
        _connectionQue.push(p);
        _connectionCnt++;
    }

    //启动一个新的线程，作为一个连接的生产者
    thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));
    produce.detach();

    //启动一个新的定时线程，扫描多余的空闲连接，超过maxIdleTime时间的空闲连接，进行多余的连接回收
    thread scanner(std::bind(&ConnectionPool::scannerConnectionTask,this));
    scanner.detach();
}

void ConnectionPool::produceConnectionTask()
{
    for(;;)
    {
        unique_lock<mutex> lock(_queueMutex);
        while(!_connectionQue.empty())
        {
            cv.wait(lock);//队列不空，此处生产线程进入等待状态
        }
        //连接数量到达上限，继续创建新的连接
        if(_connectionCnt <= _maxSize)
        {
            Connection *p = new Connection();
            p->connect(_ip, _port,_username,_password,_dbname);
            p->refreshAliveTime();
            _connectionQue.push(p);
            _connectionCnt++;
        }
        //通知消费者线程，可以消费连接了
        cv.notify_all();
    }
}


//给外部提供接口，从连接池中获取一个可用的空闲连接
shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(_queueMutex);
    while(_connectionQue.empty())
    {
        //sleep
        if(cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
        {
            if(_connectionQue.empty())
            {
                LOG("获取空闲链接超时...获取连接失败");
                return nullptr;
            }
        }
    }
    /*
    shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于调用connection的析构函数
    connection就被close掉了
    这里需要自定义shared_ptr的释放资源的方式，把connnection直接归还到queue当中
    */
    shared_ptr<Connection> sp(_connectionQue.front(),[&](Connection *pcon){
        //这是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
        unique_lock<mutex> lock(_queueMutex);
        pcon->refreshAliveTime();
        _connectionQue.push(pcon);
    });

    _connectionQue.pop();
    //消费完连接以后，通知生产者线程检查一下，如果队列为空了，赶紧进行生产
    cv.notify_all();
    return sp;
}

//扫描多余的空闲连接，超过maxIdleTime时间的空闲连接，进行多余的连接回收
void ConnectionPool::scannerConnectionTask()
{
    for(;;)
    {
        //通过sleep模拟定时效果
        this_thread::sleep_for(chrono::seconds(_maxIdleTime));
        //扫描整个队列，释放多余的连接
        unique_lock<mutex> lock(_queueMutex);
        while(_connectionCnt > _initSize)
        {
            Connection *p = _connectionQue.front();
            if(p->getAliveTime() >= (_maxIdleTime*1000))
            {
                _connectionQue.pop();
                _connectionCnt--;
                delete p; //调用～Connection()释放连接
            }else{
                break;//队头的连接没有超过_maxIdleTime,其他连接肯定也没有超过
            }
        }

    }
}