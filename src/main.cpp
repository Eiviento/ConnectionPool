#include <iostream>
using namespace std;
#include "Connection.h"
#include "CommonConnectionPool.h"

int main()
{
    // 数据库操作
    //  Connection conn;
    //  string sql = "insert into user (name,age,sex) values('John',18,'male')";
    //  //sql语句的组装方法
    //  // char sql[1024] = {0};
    //  // sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
    //  //         "zhangsan",20,"male");
    //  conn.connect("127.0.0.1",3306,"root","root","user");
    //  conn.update(sql);

    // ConnectionPool *cp = ConnectionPool::getConnectionPool();
    // cp->loadConfigFile();

    //单线程插入数据
    // clock_t begin = clock();
    // ConnectionPool *cp = ConnectionPool::getConnectionPool();
    // for (int i = 0; i < 1000; ++i)
    // {
    //     //Connection conn;
    //     char sql[1024] = {0};
    //     sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
    //             "zhangsan", 20, "male");
    //     // conn.connect("127.0.0.1", 3306, "root", "root", "user");
    //     // conn.update(sql);
    //     //ConnectionPool *cp = ConnectionPool::getConnectionPool();
    //     shared_ptr<Connection> sp = cp -> getConnection();
    //     sp->update(sql);
    // }
    // clock_t end = clock();
    // cout<< (end - begin) << "ms" << endl;



    //多线程插入数据
    clock_t begin = clock();
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    thread t1([&](){
        for (int i = 0; i < 250; ++i)
        {
            //Connection conn;
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
                    "zhangsan", 20, "male");
            shared_ptr<Connection> sp = cp -> getConnection();
            sp->update(sql);
        }
    });
    thread t2([&](){
        for(int i = 0; i < 250; ++i)
        {
            //Connection conn;
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
                    "zhangsan", 20, "male");
            shared_ptr<Connection> sp = cp -> getConnection();
            sp->update(sql);
        }
    });
    thread t3([&](){
        for (int i = 0; i < 250; ++i)
        {
            //Connection conn;
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
                    "zhangsan", 20, "male");
            shared_ptr<Connection> sp = cp -> getConnection();
            sp->update(sql);
        }
    });
    thread t4([&](){
        for (int i = 0; i < 250; ++i)
        {
            //Connection conn;
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
                    "zhangsan", 20, "male");
            shared_ptr<Connection> sp = cp -> getConnection();
            sp->update(sql);
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    clock_t end = clock();
    cout<< (end - begin) << "ms" << endl;

    return 0;
}