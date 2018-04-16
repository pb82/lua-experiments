#include "httpserver.h"

void HttpServer::requestHandler(mg_connection *c, int ev, void *p)
{
    if (ev == MG_EV_HTTP_REQUEST)
    {
        std::string reply = "Hello World";
        mg_send_head(c, 200, reply.size(), "Content-Type: text/plain");
        mg_printf(c, reply.c_str());
    }
}

void HttpServer::serverLoop()
{
    for(;;)
    {
        mg_mgr_poll(server.load(), 1000);
    }
}

HttpServer::HttpServer(Logger *logger) : logger(logger)
{
    mg_mgr_init(&mgr, nullptr);
    c = mg_bind(&mgr, "8080", requestHandler);
    mg_set_protocol_http_websocket(c);
    server.store(&mgr);
}
