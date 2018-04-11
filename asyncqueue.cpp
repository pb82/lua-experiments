#include "asyncqueue.h"

int AsyncQueue::counter = 0;

AsyncQueue& AsyncQueue::instance()
{
    static AsyncQueue instance;
    return instance;
}

AsyncQueue::AsyncQueue() : loop(uv_default_loop())
{
    // uv_idle_init(loop, &idler);
    // uv_idle_start(&idler, idleCallback);
}

AsyncQueue::~AsyncQueue()
{
    uv_loop_close(loop);
}

void AsyncQueue::run()
{
    uv_run(loop, UV_RUN_DEFAULT);
}

void AsyncQueue::submit(ActionBaton *job)
{
    uv_queue_work(loop, &job->req, actionRun, actionCleanup);
}

void AsyncQueue::actionCleanup(uv_work_t *req, int status)
{
    ActionBaton *action = (ActionBaton *) req->data;

    if (action->code != Success)
    {
        AsyncQueue::instance().logger().error(action->msg.c_str());
    }

    delete action;
}

void AsyncQueue::actionRun(uv_work_t *req)
{
    ActionBaton *action = (ActionBaton *) req->data;
    std::string bytecode = AsyncQueue::instance().persistence().getAction(action->name);

    Sandbox sandbox;
    sandbox.mslimit = action->timeout;
    sandbox.kblimit = action->maxmem;
    RunCode result = sandbox.runAction(action->name, bytecode, &action->msg);
    action->code = result;
}

void AsyncQueue::idleCallback(uv_idle_t *t)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    AsyncQueue::counter++;
}

void AsyncQueue::setLogger(Logger *logger)
{
    this->_logger = logger;
}

void AsyncQueue::setPersistence(Persistence *persistence)
{
    this->_persistence = persistence;
}

Logger& AsyncQueue::logger()
{
    return *_logger;
}

Persistence &AsyncQueue::persistence()
{
    return *_persistence;
}
