#include "invocationring.h"

std::atomic<std::map<long, Invocation> *> InvocationRing::ptr;
std::mutex InvocationRing::_lock;

InvocationRing::InvocationRing()
{
    invocations = new std::map<long, Invocation>();
    ptr.store(invocations);
}

void InvocationRing::putCallback(long invocationId, std::function<void (void *)> cb)
{
    std::lock_guard<std::mutex> lock(_lock);
    std::map<long, Invocation>::iterator item;
    std::map<long, Invocation> *i = ptr.load();

    if ((item = i->find(invocationId)) != i->end())
    {
        void *action = item->second.action;
        if (action)
        {
            cb(action);
            i->erase(invocationId);
        }
    } else
    {
        Invocation inv;
        inv.callback = cb;
        (*i)[invocationId] = inv;
    }
}

void InvocationRing::putAction(long invocationId, void *action)
{
    std::lock_guard<std::mutex> lock(_lock);
    std::map<long, Invocation>::iterator item;
    std::map<long, Invocation> *i = ptr.load();

    if ((item = i->find(invocationId)) != i->end())
    {
        std::function<void(void *)> cb = item->second.callback;
        cb(action);
        i->erase(invocationId);
    } else
    {
        Invocation inv;
        inv.action = action;
        (*invocations)[invocationId] = inv;
    }
}
