#ifndef ASYNCQUEUE_H
#define ASYNCQUEUE_H

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>
#include <functional>
#include <uv.h>
#include <queue>

#include "logger.h"
#include "persistence.h"
#include "sandbox.h"
#include "invocationring.h"

class ActionBaton
{
public:
    ActionBaton(std::string name) : name(name)
    {
        req.data = this;
        invocationId = ++invocations;
    }

    uv_work_t req;
    long invocationId;

    std::string name;
    long timeout = 0;
    int maxmem = 0;

    std::string msg;
    RunCode code;
    JSON::Value result;
    JSON::Value argument;
    std::function<void(int code, JSON::Value result)> callback;
    static std::atomic<long> invocations;
};

class AsyncQueue
{
public:    
    static AsyncQueue& instance();
    ~AsyncQueue();

    AsyncQueue(AsyncQueue const&) = delete;
    void operator=(AsyncQueue const&) = delete;

    void setLogger(Logger *logger);
    Logger& logger();

    void setPersistence(Persistence *persistence);
    Persistence& persistence();

    void setRegistry(PluginRegistry *registry);
    PluginRegistry* registry();

    void setInvocations(InvocationRing *invocations);
    InvocationRing* invocations();

    void run();
    void submit(ActionBaton *job);
    bool hasAction(std::string name);

    void enqueue(ActionBaton *action);
    ActionBaton *dequeue();

private:
    AsyncQueue();

    Logger *_logger = nullptr;
    Persistence *_persistence = nullptr;
    PluginRegistry *_registry = nullptr;
    InvocationRing *_invocations = nullptr;

    uv_idle_t idler;
    uv_loop_t *loop;

    static void idleCallback(uv_idle_t*);
    static void actionCleanup(uv_work_t *req, int);
    static void actionRun(uv_work_t *req);

    std::queue<ActionBaton *> queuedActions;

    uv_rwlock_t lock;
};

#endif // ASYNCQUEUE_H
