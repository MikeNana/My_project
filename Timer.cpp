#include "Timer.h"
#include <sys/time.h>
TimerNode::TimerNode(shared_ptr<HttpData> SP_Httpdata, int timeout) :
SPHttpData(SP_Httpdata),
deleted_(false)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}
TimerNode::~TimerNode()
{
    SPHttpData->handleclose();
}
TimerNode::TimerNode(const TimerNode& tn) : 
    SPHttpData(tn.SPHttpData),
    deleted_(false),
    expiredTime_(0)
{}
void TimerNode::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expiredTime_ = 
        (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}
bool TimerNode::isValid()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t tmp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
    if(tmp < expiredTime_)
        return false;
    else 
        return true;
}
void TimerNode::clearReq()
{
    SPHttpData.reset();
    this->setDeleted();
}
void TimerNode::setDeleted()
{
    deleted_ = true;
}
bool TimerNode::isDeleted() const
{
    return deleted_;
}
size_t TimerNode::getExpTime() const
{
    return expiredTime_;
}
TimerManager::TimerManager()
{

}
TimerManager::~TimerManager()
{

}
void TimerManager::add_timer(shared_ptr<HttpData> SPHttpData, int timeout)
{
    shared_ptr<TimerNode> new_node(new TimerNode(SPHttpData, timeout));
    timerNode_queue.push(new_node);
    SPHttpData->linktimer(new_node);
}
void TimerManager::handle_expirdEvents()
{
    while(!timerNode_queue.empty())
    {
        shared_ptr<TimerNode> cur_node = timerNode_queue.top();
        if(cur_node->isValid() || cur_node->isDeleted())
            timerNode_queue.pop();
        else
            break;
    }
}