#ifndef INVOCATIONRING_H
#define INVOCATIONRING_H

#include <functional>
#include <map>
#include <mutex>

#include "json/Value.hpp"

class Invocation
{
public:
    void *action;
    std::function<void(void *)> callback;
};

class InvocationRing
{
public:
    InvocationRing();
    void putCallback(long invocationId, std::function<void (void *)> cb);
    void putAction(long invocationId, void *action);
private:
    std::map<long, Invocation> invocations;
    std::mutex _lock;
};

#endif // INVOCATIONRING_H
