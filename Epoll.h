//该类旨在实现一个轮询器
//主要数据成员包括
//1.记录HttpData和Channel的两个数组
//2.一个定时器管理器
//3.存储就绪事件的vector
//4.epoll文件描述符
//完成的功能有
//1.封装epoll的添加、更改、删除功能
//2.添加定时器到定时管理器中
//3.获取就绪事件并存储
//4.处理超时事件(实际是由timermanager管理的)
#include <memory>
#include <vector>
#include <iostream>
#include "Timer.h"
#include "Channel.h"
#include "HttpData.h"
#include <sys/epoll.h>
using namespace std;

class Epoll
{
public:
    Epoll();
    ~Epoll();
    void epoll_add(shared_ptr<Channel> channel, int timeout);
    void epoll_mod(shared_ptr<Channel> channel, int timeout);
    void epoll_del(shared_ptr<Channel> channel);
    vector<shared_ptr<Channel>> poll();
    void add_timer(shared_ptr<Channel> channel, int timeout);
    int get_epoll_fd();
    void handle_expired_events();
private:
    vector<shared_ptr<Channel>> get_ready_events(int events_num);

    static const int MAXFDS = 10000;
    bool started_;
    shared_ptr<Channel> fd_channel[MAXFDS];
    shared_ptr<HttpData> fd_httpdata[MAXFDS];
    TimerManager timer_manager;
    vector<epoll_event> ready_events;
    int epoll_fd;
};