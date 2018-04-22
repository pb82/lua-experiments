#ifndef INVOCATIONRING_H
#define INVOCATIONRING_H

#include <functional>
#include <map>
#include <mutex>
#include <atomic>

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
    std::map<long, Invocation> *invocations;

    static std::mutex _lock;
    static std::atomic<std::map<long, Invocation> *> ptr;
};

#endif // INVOCATIONRING_H
