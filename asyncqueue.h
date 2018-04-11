#ifndef ASYNCQUEUE_H
#define ASYNCQUEUE_H

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <uv.h>

#include "logger.h"
#include "persistence.h"
#include "sandbox.h"

class ActionBaton
{
public:
    ActionBaton(const char *name) : name(name)
    {
        req.data = this;
    }

    uv_work_t req;

    std::string name;
    long timeout = 0;
    int maxmem = 0;

    std::string msg;
    RunCode code;
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

    void run();
    void submit(ActionBaton *job);

private:
    AsyncQueue();

    Logger *_logger = nullptr;
    Persistence *_persistence = nullptr;

    uv_idle_t idler;
    uv_loop_t *loop;

    static void idleCallback(uv_idle_t*);
    static void actionCleanup(uv_work_t *req, int status);
    static void actionRun(uv_work_t *req);
    static int counter;
};

#endif // ASYNCQUEUE_H
