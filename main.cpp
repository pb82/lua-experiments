#include <iostream>
#include <signal.h>

#include "logger.h"
#include "nulldb.h"
#include "mongoadapter.h"
#include "asyncqueue.h"
#include "pluginregistry.h"
#include "httpserver.h"
#include "invocationring.h"

using namespace std;

static Persistence *persistence;

void getPersistenceLayer(Config &config, Logger &logger)
{
    switch(config.getPersistenceType())
    {
    case NULLDB:
        logger.warn("Using in memory storage (Nulldb)");
        persistence = new Nulldb();
        break;
    case MONGODB:
        logger.info("Using Mongodb persistence");
        persistence = new MongoAdapter(config, &logger);
        persistence->init();
        break;
    default:
        throw PersistenceError("No persistence layer configured");
    }
}

void onSignal(int sig)
{
    std::cout << "Received signal: " << sig << std::endl;
    delete persistence;
    exit(sig);
}

void connectSignals()
{
    signal(SIGABRT, onSignal);
    signal(SIGTERM, onSignal);
    signal(SIGINT,  onSignal);
    signal(SIGSEGV, onSignal);
}

int main()
{
    connectSignals();

    Config config;
    config.loadConfig();

    Logger logger(config);
    logger.info("Config loaded");

    getPersistenceLayer(config, logger);
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
