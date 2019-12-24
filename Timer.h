#pragma once 
#include "HttpData.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Epoll.h"
#include <queue>
#include <memory> 

using std::shared_ptr;
using std::priority_queue;
using std::deque;
using std::priority_queue;


class HttpData;
class Channel;

class TimerNode
{
public:
    TimerNode(shared_ptr<HttpData>, int timeout);
    ~TimerNode();
    TimerNode(const TimerNode& tn);
    void updata(int timeout);
    bool isValid();
    void clearReq();
    void setDeleted();
    bool isDeleted() const; 
    size_t getExpTime() const;
private: 
    bool deleted_;
    size_t expiredTime_;
    shared_ptr<HttpData> SPHttpData;
};
struct timerNode_cmp
{
    bool operator()(shared_ptr<TimerNode> tn1, shared_ptr<TimerNode> tn2)
    {
        return tn1->getExpTime() < tn2->getExpTime();
    }
};

class TimerManager
{
private: 
    typedef shared_ptr<TimerNode> SP_timerNode;
    priority_queue<SP_timerNode, deque<SP_timerNode>, timerNode_cmp> timerNode_queue;
public: 
    TimerManager();
    ~TimerManager();
    void add_timer(shared_ptr<HttpData> SPHttpData, int timeout);
    void handle_expirdEvents();
};