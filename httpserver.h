#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <atomic>
#include <thread>

#include "web/mongoose.h"
#include "logger.h"
#include "asyncqueue.h"
#include "json/Parser.hpp"

class Matcher
{
public:
    Matcher(const char *method, const char *pattern) : method(method)
    {
        split(pattern, '/', std::back_inserter(patternParts));        
    }

    bool match(http_message *req, std::map<std::string, std::string> &vars)
    {
        std::string _method(req->method.p, req->method.len);
        if (_method.compare(method) != 0)
        {
            return false;
        }

        std::string uri(req->uri.p, req->uri.len);
        std::vector<std::string> uriParts;

        split(uri, '/', std::back_inserter(uriParts));

        if (uriParts.size() != patternParts.size())
        {
            return false;
        }

        bool result = true;
        for (unsigned int i = 0; i < uriParts.size(); i++)
        {
            std::string uriPart = uriParts[i];
            std::string patPart = patternParts[i];

            // Variable
            if (patPart.rfind(":") == 0)
            {
                vars[patPart] = uriPart;
            } else
            {
                if (uriPart.compare(patPart) != 0)
                {
                    result = false;
                    vars.clear();
                    break;
                }
            }
        }

        return result;
    }

private:
    template<typename Out>
    void split(std::string s, char delim, Out result) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }

    std::vector<std::string> patternParts;
    std::string method;
};

class HttpServer
{
public:
    HttpServer(Logger *logger);
    void serverLoop();

    InvocationRing *invocations;
private:
    static void requestHandler(mg_connection *c, int ev, void *p);

    std::atomic<mg_mgr *> server;
    mg_mgr mgr;
    mg_connection *c;

    Logger *logger;

    static Matcher route_action_block;
    static Matcher route_action_noblock;
    static Matcher route_get_invocation;
    static Matcher route_ping;

    static JSON::Printer printer;
    static JSON::Parser parser;
};

#endif // HTTPSERVER_H
