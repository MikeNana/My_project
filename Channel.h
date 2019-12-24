#include "HttpData.h"
#include <memory>
#include <unistd.h>
#include <functional>
class EventLoop;
class HttpData;

class Channel
{
    typedef std::function<void()> CALLBACK;
//记录私有成员变量
private:
    EventLoop* loop_;
    weak_ptr<HttpData> holder;
    int fd_;    //该channel关心的文件描述符
    //事件
    int concern_events;
    int return_events;
    int last_events;
//记录私有成员函数
private:
    int prase_URI();
    int prase_HEADERS();
    int analysisRequest();

    CALLBACK read_handler;
    CALLBACK write_handler;
    CALLBACK conn_handler;
    CALLBACK error_handler;

public:
//创建一个空的channel
    Channel(EventLoop* loop);
//创建一个带有fd的channel
    Channel(EventLoop* loop, int fd);
    ~Channel();
    int get_fd();
    void set_fd(int fd);

    void set_holder(shared_ptr<HttpData> holder_)
    {
        holder = holder_;
    }
    shared_ptr<HttpData> get_holder()
    {
        shared_ptr<HttpData> ret(holder.lock());
        return ret;
    }
//将httpdata中的处理函数设置为句柄以供回调
    void setReadHandler(CALLBACK&& readhandler){ read_handler = readhandler; }
    void setWriteHandler(CALLBACK&& writehandler){ write_handler = writehandler; }
    void setErrorHandler(CALLBACK&& errorhandler){ error_handler = errorhandler;}
    void setConnHandler(CALLBACK&& connhandler){ conn_handler = connhandler; }

//回调函数，将不同的事件分发给httpdata中的处理函数处理
    void handle_events()
    {
        concern_events = 0;
        if((return_events & EPOLLHUP) && !(return_events & EPOLLIN))
        {
            concern_events = 0;
            return;
        }
        if(return_events & EPOLLERR)
        {
            concern_events = 0;
            if(error_handler)
                error_handler();
            return;
        }
        if(return_events & (EPOLLHUP | EPOLLIN | EPOLLPRI))
        {
            if(read_handler)
                read_handler();
        }
        if(return_events & EPOLLOUT)
        {
            if(write_handler)
                write_handler();
        }
        conn_handler();
    }
    void handle_error(int fd, int err_num, std::string short_msg);
    void handle_read();
    void handle_write();
    void handle_conn();

    void set_return_events(int ev){ return_events = ev; }
    void set_events(int ev){ concern_events = ev; }
    int& get_events(){ return concern_events; }

    bool equal_update_lastevent()
    {
        bool ret = (last_events == concern_events);
        last_events = concern_events;
        return ret;
    }
    unsigned int get_last_event(){ return last_events; }
};