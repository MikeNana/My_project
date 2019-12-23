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
//4.处理超时事件
#include "HttpData.h"
#include "Timer.h"
#include <sys/epoll.h>
#include <vector>
#include <iterator>
#include <memory>
#include "Channel.h"
class Epoll
{
public:
    Epoll();
    ~Epoll();
    typedef std::shared_ptr<Channel> SP_Channel;
    std::vector<SP_Channel> poll();
    std::vector<SP_Channel> getRevents();
    void epoll_add(SP_Channel request, int timeout);
    void epoll_mod(SP_Channel request, int timeout);
    void epoll_del(SP_Channel request);
    void add_timer(SP_Channel request, int timeout);
    void handleExpired();
    int get_epoll_fd();
private:
    static const int MAXFDS = 10000;
    std::shared_ptr<HttpData> fd_http[MAXFDS];
    std::shared_ptr<Channel>  fd_channel[MAXFDS];
    TimerManager timermanager;
    int fd_;
};