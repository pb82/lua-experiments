#include "asyncqueue.h"

std::atomic<long> ActionBaton::invocations(1);

AsyncQueue& AsyncQueue::instance()
{
    static AsyncQueue instance;
    return instance;
}

AsyncQueue::AsyncQueue() : loop(uv_default_loop())
{
    uv_rwlock_init(&lock);
    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, idleCallback);
}

AsyncQueue::~AsyncQueue()
{
    uv_loop_close(loop);
}

void AsyncQueue::run()
{
    uv_run(loop, UV_RUN_NOWAIT);
}

void AsyncQueue::submit(ActionBaton *job)
{
    uv_queue_work(loop, &job->req, actionRun, actionCleanup);
}

void AsyncQueue::actionCleanup(uv_work_t *req, int)
{
    ActionBaton *action = (ActionBaton *) req->data;
    if (action->code != Success)
    {
        AsyncQueue::instance().logger().error(action->msg.c_str());
    }

    if (action->callback)
    {
        action->callback(action->code, action->result);
        delete action;
    } else
    {
        AsyncQueue::instance().invocations()->putAction(action->invocationId, action);
    }
}

void AsyncQueue::actionRun(uv_work_t *req)
{
    ActionBaton *action = (ActionBaton *) req->data;
    std::string bytecode = AsyncQueue::instance().persistence().getAction(action->name);

    Sandbox sandbox;
    sandbox.registry = AsyncQueue::instance().registry();
    sandbox.mslimit = action->timeout;
    sandbox.kblimit = action->maxmem;
    RunCode result = sandbox.runAction(action->name,
                                       bytecode, &action->msg, action->result);
    action->code = result;
}

void AsyncQueue::idleCallback(uv_idle_t *)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

bool AsyncQueue::hasAction(std::string name)
{
    return _persistence->hasAction(name);
}

PluginRegistry *AsyncQueue::registry()
{
    return _registry;
}

void AsyncQueue::setRegistry(PluginRegistry *registry)
{
    this->_registry = registry;
}

InvocationRing *AsyncQueue::invocations()
{
    return this->_invocations;
}

void AsyncQueue::setInvocations(InvocationRing *invocations)
{
    this->_invocations = invocations;
}
