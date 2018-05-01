#include "httpserver.h"

Matcher HttpServer::route_action_block("POST", "/actions/:id/block");
Matcher HttpServer::route_action_noblock("POST", "/actions/:id");
Matcher HttpServer::route_get_invocation("GET", "/invocation/:id");
Matcher HttpServer::route_ping("GET", "/ping");
Matcher HttpServer::route_list_actions("GET", "/actions");
Matcher HttpServer::route_action_create("POST", "/actions");
Matcher HttpServer::route_action_delete("DELETE", "/actions/:id");

JSON::Printer HttpServer::printer;
JSON::Parser HttpServer::parser;

void HttpServer::requestHandler(mg_connection *c, int ev, void *p)
{
    if (ev != MG_EV_HTTP_REQUEST)
    {
        return;
    }

    // Awful code ahead

    http_message *req = static_cast<http_message *>(p);
    std::string method(req->method.p, req->method.len);
    if (method.compare("POST") == 0)
    {
        if (req->body.len <= 0)
        {
            std::string message("POST without body received");
            AsyncQueue::instance().logger().error(message.c_str());
            mg_send_head(c, 500, message.size(), "text/plain");
            mg_printf(c, message.c_str());
            return;
        }
    }

    std::map<std::string, std::string> vars;

    if (HttpServer::route_action_delete.match(req, vars))
    {
        std::string name = vars[":id"];
        if (AsyncQueue::instance().hasAction(name))
        {
            AsyncQueue::instance().persistence().deleteAction(name);
            const char *resp = "OK";
            AsyncQueue::instance().logger().info("Action removed successfully");
            mg_send_head(c, 200, std::strlen(resp), "Content-Type: text/plain");
            mg_printf(c, resp);
        } else
        {
            std::string err("Action does not exist");
            AsyncQueue::instance().logger().error(err.c_str());
            mg_send_head(c, 500, err.size(), "text/plain");
            mg_printf(c, err.c_str());
            return;
        }
    } else if (HttpServer::route_action_create.match(req, vars))
    {
        JSON::Value val;
        try {
            std::string body(req->body.p, req->body.len);
            parser.parse(val, body);
        } catch(std::runtime_error err) {
            mg_send_head(c, 500, strlen(err.what()), "text/plain");
            mg_printf(c, err.what());
            return;
        }

        std::string name = val["name"].as<std::string>();
        if (name.length() <= 0) {
            std::string err("Missing parameter `name`");
            mg_send_head(c, 500, err.size(), "text/plain");
            mg_printf(c, err.c_str());
            return;
        }

        if (AsyncQueue::instance().hasAction(name))
        {
            std::string err("Action already exists");
            AsyncQueue::instance().logger().error(err.c_str());
            mg_send_head(c, 500, err.size(), "text/plain");
            mg_printf(c, err.c_str());
            return;
        }

        std::string base64 = val["code"].as<std::string>();
        std::string sourcecode;
        Base64::Decode(base64, &sourcecode);

        std::string bytecode;
        bool success = AsyncQueue::instance().compileAction(sourcecode.c_str(), &bytecode);
        if (!success) {
            std::string err("Error compiling action");
            mg_send_head(c, 500, err.size(), "text/plain");
            mg_printf(c, err.c_str());
            return;
        }

        int timeout = val["timeout"].as<int>();
        if (timeout < 0) timeout = 0;
        int maxmem = val["maxmem"].as<int>();
        if (maxmem< 0) maxmem = 0;

        AsyncQueue::instance().persistence().addAction(name, bytecode, timeout, maxmem);

        const char *resp = "OK";
        AsyncQueue::instance().logger().info("Action compiled successfully");
        mg_send_head(c, 200, std::strlen(resp), "Content-Type: text/plain");
        mg_printf(c, resp);
    } else if (HttpServer::route_list_actions.match(req, vars))
    {
        std::vector<ActionDefinition> actions;
        AsyncQueue::instance().persistence().getActions(actions);

        JSON::Array result;
        for (const ActionDefinition& action: actions)
        {
            JSON::Object actObj = {
                {"name", action.name},
                {"size", (long) action.size},
                {"timeout", action.timeout},
                {"maxmem", action.maxmem}
            };

            result.push_back(actObj);
        }

        std::string json = printer.print(result);
        mg_send_head(c, 200, json.size(), "Content-Type: application/json");
        mg_printf(c, json.c_str());
    } else if (HttpServer::route_ping.match(req, vars))
    {
        const char *resp = "OK";
        AsyncQueue::instance().logger().info("Ping request received");
        mg_send_head(c, 200, std::strlen(resp), "Content-Type: text/plain");
        mg_printf(c, resp);
    } else if (HttpServer::route_action_block.match(req, vars))
    {
        bool exists = AsyncQueue::instance().hasAction(vars[":id"]);
        if (exists)
        {
            JSON::Value val;            
            try {
                std::string body(req->body.p, req->body.len);
                parser.parse(val, body);
            } catch(std::runtime_error err) {
                mg_send_head(c, 500, strlen(err.what()), "text/plain");
                mg_printf(c, err.what());
                return;
            }

            ActionBaton *act = new ActionBaton(vars[":id"]);
            act->argument = val;
            act->callback = [c](int code, JSON::Value result) {
                if (code == Success)
                {
                    std::string json = printer.print(result);
                    mg_send_head(c, 200, json.size(), "Content-Type: application/json");
                    mg_printf(c, json.c_str());
                } else
                {
                    JSON::Value err = {{"err", code}};
                    std::string json = printer.print(err);
                    mg_send_head(c, 500, json.size(), "Content-Type: text/plain");
                    mg_printf(c, json.c_str());
                    return;
                }
            };
            AsyncQueue::instance().enqueue(act);
        } else {
            std::string reply = "Action not found";
            mg_send_head(c, 404, reply.size(), "Content-Type: text/plain");
            mg_printf(c, reply.c_str());
        }
    } else if (route_get_invocation.match(req, vars))
    {
        long invocationId;
        try {
            invocationId = std::stol(vars[":id"]);
        } catch (std::invalid_argument err) {
            AsyncQueue::instance().logger().error(err.what());
            mg_send_head(c, 400, std::strlen(err.what()), "Content-Type: text/plain");
            mg_printf(c, err.what());
            return;
        } catch (std::out_of_range err) {
            AsyncQueue::instance().logger().error(err.what());
            mg_send_head(c, 400, std::strlen(err.what()), "Content-Type: text/plain");
            mg_printf(c, err.what());
            return;
        }

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
                mg_send_head(c, 500, act->msg.size(), "Content-Type: text/plain");
                mg_printf(c, act->msg.c_str());
            }
        });
    } else if (HttpServer::route_action_noblock.match(req, vars))
    {
        bool exists = AsyncQueue::instance().hasAction(vars[":id"]);
        if (exists)
        {
            JSON::Value val;
            try {
                std::string body(req->body.p, req->body.len);
                parser.parse(val, body);
            } catch(std::runtime_error err) {
                mg_send_head(c, 500, strlen(err.what()), "text/plain");
                mg_printf(c, err.what());
                return;
            }

            ActionBaton *act = new ActionBaton(vars[":id"]);
            act->argument = val;

            long invocationId = act->invocationId;
            AsyncQueue::instance().enqueue(act);
            JSON::Value result = JSON::Object {{"InvocationId", invocationId}};
            std::string json = printer.print(result);
            mg_send_head(c, 200, json.size(), "Content-Type: application/json");
            mg_printf(c, json.c_str());
        }  else
        {
            std::string reply = "Action not found";
            mg_send_head(c, 404, reply.size(), "Content-Type: text/plain");
            mg_printf(c, reply.c_str());
        }

    } else
    {
        std::string reply = "Unknown route";
        mg_send_head(c, 404, reply.size(), "Content-Type: text/plain");
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
