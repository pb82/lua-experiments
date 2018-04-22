#include "invocationring.h"

InvocationRing::InvocationRing()
{
}

void InvocationRing::putCallback(long invocationId, std::function<void (void *)> cb)
{
    std::lock_guard<std::mutex> lock(_lock);
    std::map<long, Invocation>::iterator item;
    if ((item = invocations.find(invocationId)) != invocations.end())
    {
        void *action = item->second.action;
        if (action)
        {
            cb(action);
            invocations.erase(invocationId);
        }
    } else
    {
        Invocation i;
        i.callback = cb;
        invocations[invocationId] = i;
    }
}

void InvocationRing::putAction(long invocationId, void *action)
{
    std::lock_guard<std::mutex> lock(_lock);
    std::map<long, Invocation>::iterator item;
    if ((item = invocations.find(invocationId)) != invocations.end())
    {
        std::function<void(void *)> cb = item->second.callback;
        cb(action);
        invocations.erase(invocationId);
    } else
    {
        Invocation i;
        i.action = action;
        invocations[invocationId] = i;
    }
}
