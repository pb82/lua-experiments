#include "httpserver.h"

Matcher HttpServer::route_action_block("/actions/:id/block");
Matcher HttpServer::route_action_noblock("/actions/:id");
Matcher HttpServer::route_get_invocation("/invocation/:id");

JSON::Printer HttpServer::printer;

void HttpServer::requestHandler(mg_connection *c, int ev, void *p)
{
    if (ev != MG_EV_HTTP_REQUEST)
    {
        return;
    }

    http_message *req = static_cast<http_message *>(p);
    std::string uri(req->uri.p, req->uri.len);
    std::string method(req->method.p, req->method.len);
    std::map<std::string, std::string> vars;

    if (HttpServer::route_action_block.match(uri, vars))
    {
        bool exists = AsyncQueue::instance().hasAction(vars[":id"]);
        if (exists)
        {
            ActionBaton *act = new ActionBaton(vars[":id"]);
            act->timeout = 1000;
            act->maxmem = 1000;
            act->callback = [c](int code, JSON::Value result) {
                if (code == Success)
                {
                    std::string json = printer.print(result);
                    mg_send_head(c, 200, json.size(), "Content-Type: application/json");
                    mg_printf(c, json.c_str());
                } else
                {
                    mg_send_head(c, 500, 0, nullptr);
                }
            };
        } else {
            std::string reply = "Action not found";
            mg_send_head(c, 404, reply.size(), "Content-Type: text/plain");
            mg_printf(c, reply.c_str());
        }
    } else if (route_get_invocation.match(uri, vars)) {
        long invocationId = std::stol(vars[":id"]);
        InvocationRing *invocations = static_cast<InvocationRing *>(c->user_data);
        invocations->putCallback(invocationId, [c](void *action){
            ActionBaton *act = static_cast<ActionBaton *>(action);
            if (act->code == Success)
            {
                std::string json = printer.print(act->result);
                mg_send_head(c, 200, json.size(), "Content-Type: application/json");
                mg_printf(c, json.c_str());
            } else
            {
                mg_send_head(c, 500, 0, nullptr);
            }
        });
    } else if (HttpServer::route_action_noblock.match(uri, vars)) {
        bool exists = AsyncQueue::instance().hasAction(vars[":id"]);
        if (exists)
        {
            ActionBaton *act = new ActionBaton(vars[":id"]);
            act->timeout = 1000;
            act->maxmem = 1000;
            long invocationId = act->invocationId;
            AsyncQueue::instance().submit(act);
            AsyncQueue::instance().run();
            JSON::Value result = invocationId;
            std::string json = printer.print(result);
            mg_send_head(c, 200, json.size(), "Content-Type: application/json");
            mg_printf(c, json.c_str());
        } else {
            std::string reply = "Action not found";
            mg_send_head(c, 404, reply.size(), "Content-Type: text/plain");
            mg_printf(c, reply.c_str());
        }

    } else
    {
        std::string reply = "Unknown route";
        mg_send_head(c, 200, reply.size(), "Content-Type: text/plain");
        mg_printf(c, reply.c_str());
    }
}

void HttpServer::serverLoop()
{    
    for(;;)
    {        
        mg_mgr_poll(server.load(), 10);
    }
}

HttpServer::HttpServer(Logger *logger) : logger(logger)
{
    mg_mgr_init(&mgr, nullptr);
    c = mg_bind(&mgr, "8080", requestHandler);
    c->user_data = invocations;
    mg_set_protocol_http_websocket(c);
    server.store(&mgr);    
}
