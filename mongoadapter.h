#ifndef MONGOADAPTER_H
#define MONGOADAPTER_H

#include <mongoc.h>
#include <sstream>

#include "persistence.h"
#include "config.h"
#include "logger.h"

#define M_DATABASE "faas"
#define M_COLLECTION "actions"

class MongoAdapter : public Persistence
{
public:
    MongoAdapter(Config &config, Logger *logger);
    ~MongoAdapter();

    void init();
    void addAction(std::string name, std::string bytecode, int timeout, int maxmem);
    const ActionDefinition getAction(std::string name);
    bool hasAction(std::string name);
    void getActions(std::vector<ActionDefinition> &list);
    void deleteAction(std::string &name);

private:
    std::string mongoUrl;
    int mongoPort;

    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;

    Logger *logger;
};

#endif // MONGOADAPTER_H
