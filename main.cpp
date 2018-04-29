#include <iostream>

#include "logger.h"
#include "nulldb.h"
#include "asyncqueue.h"
#include "pluginregistry.h"
#include "httpserver.h"
#include "invocationring.h"

using namespace std;

Persistence *getPersistenceLayer(Config &config, Logger &logger)
{
    switch(config.getPersistenceType())
    {
    case NULLDB:
        logger.warn("Using in memory storage (Nulldb)");
        return new Nulldb();
    default:
        throw PersistenceError("No persistence layer configured");
    }
}

int main()
{
    Config config;
    config.loadConfig();

    Logger logger(config);
    logger.info("Config loaded");

    Persistence *persistence = getPersistenceLayer(config, logger);
    PluginRegistry registry(&logger, &config);
    InvocationRing *invocations = new InvocationRing;

    AsyncQueue::instance().setLogger(&logger);
    AsyncQueue::instance().setPersistence(persistence);
    AsyncQueue::instance().setRegistry(&registry);
    AsyncQueue::instance().setInvocations(invocations);

    HttpServer server(&logger);
    server.invocations = invocations;

    std::thread serverThread(&HttpServer::serverLoop, &server);

    AsyncQueue::instance().run();

    serverThread.join();

    delete persistence;
    return 0;
}
