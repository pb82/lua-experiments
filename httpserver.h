#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <atomic>
#include <thread>

#include "web/mongoose.h"
#include "logger.h"

class HttpServer
{
public:
    HttpServer(Logger *logger);
    void serverLoop();

private:
    static void requestHandler(mg_connection *c, int ev, void *p);

    std::atomic<mg_mgr *> server;
    mg_mgr mgr;
    mg_connection *c;

    Logger *logger;
};

#endif // HTTPSERVER_H
