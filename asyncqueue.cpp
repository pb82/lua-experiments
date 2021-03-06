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

int AsyncQueue::bytecodeWriter(lua_State *, const void *p, size_t sz, void *ud)
{
    std::string *buffer = (std::string *)ud;
    buffer->append((const char *)p, sz);
    return 0;
}

bool AsyncQueue::compileAction(const char* action, std::string *buffer)
{
    int status;
    lua_State *L = luaL_newstate();

    // Check if the chunk was compiled without errors
    if ((status = luaL_loadstring(L, action)) != LUA_OK)
    {
        logger().error(lua_tostring(L, -1));
        return false;
    }

    lua_dump(L, bytecodeWriter, buffer, true);
    lua_close(L);
    return true;
}

void AsyncQueue::run()
{    
    uv_run(loop, UV_RUN_DEFAULT);
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
    const ActionDefinition &def = AsyncQueue::instance().persistence().getAction(action->name);

    Sandbox sandbox;
    sandbox.registry = AsyncQueue::instance().registry();
    sandbox.mslimit = def.timeout;
    sandbox.kblimit = def.maxmem;
    RunCode result = sandbox.runAction(action->name,
                                       def.bytecode, &action->msg,
                                       action->result, action->argument);
    action->code = result;
}

void AsyncQueue::idleCallback(uv_idle_t *)
{
    ActionBaton *nextAction = AsyncQueue::instance().dequeue();
    if (nextAction)
    {
        AsyncQueue::instance().submit(nextAction);
    } else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
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

void AsyncQueue::enqueue(ActionBaton *action)
{
    queuedActions.push(action);
}

ActionBaton *AsyncQueue::dequeue()
{
    ActionBaton *result = nullptr;
    // uv_rwlock_wrlock(&lock);
    if (queuedActions.size() > 0)
    {
        result = queuedActions.front();
        queuedActions.pop();
    }
    // uv_rwlock_wrunlock(&lock);
    return result;
}
