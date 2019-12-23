#include "Epoll.h"
#include <iostream>
#include <assert.h>
using namespace std;

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;
Epoll::Epoll() : epoll_fd(epoll_create1(EPOLL_CLOEXEC)), ready_events(EVENTSNUM)//使用epoll_create1可以设置在退出时关闭epoll打开的文件描述符
{
    assert(epoll_fd > 0);
}

Epoll::~Epoll()
{
    //线程退出时会自动关闭epoll所占文件描述符，所以不用手动关闭
}

void Epoll::epoll_add(shared_ptr<Channel> channel, int timeout)
{
    int fd = channel->get_fd();
    if(timeout > 0)
    {
        add_timer(channel, timeout);
        fd_httpdata[fd] = channel->get_holder();
    }
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->get_events;

    channel->equal_update_lastevent();

    fd_channel[fd] = channel;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
//如果添加失败，则重置该channel
        fd_channel[fd].reset();
    }
}

void Epoll::epoll_mod(shared_ptr<Channel> channel, int timeout)
{
    if(timeout > 0)
        add_timer(channel, timeout);
    int fd = channel->get_fd();
    if(!(channel->equal_update_lastevent()))
    {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = channel->get_events();
        if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0)
        {
            perror("epoll_mod error.");
            fd_channel[fd].reset();
        }
    }
}

void Epoll::epoll_del(shared_ptr<Channel> channel)
{
    int fd = channel->get_fd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = channel->get_events();
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0)
        perror("epoll_del error.");
    fd_channel[fd].reset();
    fd_httpdata[fd].reset();
}

vector<shared_ptr<Channel>> Epoll::poll()
{
    while(true)
    {
        int event_count = epoll_wait(epoll_fd, &*ready_events.begin(), ready_events.size(), EPOLLWAIT_TIME);
        if(event_count < 0)
            perror("epoll_wait error.");
        vector<shared_ptr<Channel>> request_channel = get_ready_events(event_count);
        if(request_channel.size() > 0)
            return request_channel;
    }
}

void Epoll::add_timer(shared_ptr<Channel> channel, int timeout)
{
    shared_ptr<HttpData> request_httpdata = channel->get_holder();
    if(request_httpdata)
        timer_manager.add_timer(request_httpdata, timeout);
    else
        cout << "timer add fail." << endl;
}

int Epoll::get_epoll_fd()
{
    return epoll_fd;
}

void Epoll::handle_expired_events()
{
    timer_manager.handle_expirdEvents();
}

vector<shared_ptr<Channel>> Epoll::get_ready_events(int events_num)
{
    vector<shared_ptr<Channel>> request_channel;
    for(int i = 0; i < events_num; ++i)
    {
        int fd = ready_events[i].data.fd;
        shared_ptr<Channel> cur_channel = fd_channel[fd];
        if(cur_channel)
        {
            cur_channel->set_return_events(ready_events[i].events);
            cur_channel->set_events(0);
            request_channel.push_back(cur_channel);
        }
        else
        {
            cout << "shared<Channel> is invalid." << endl;
        }
        
    }
    return request_channel;
}