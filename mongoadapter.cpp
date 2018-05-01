#include "mongoadapter.h"

std::mutex MongoAdapter::_lock;

MongoAdapter::MongoAdapter(Config& config, Logger *logger) : logger(logger)
{
    this->mongoUrl = config.getMongoUrl();
    this->mongoPort = config.getMongoPort();
}

MongoAdapter::~MongoAdapter()
{
    if (collection) mongoc_collection_destroy(collection);
    if (database) mongoc_database_destroy(database);
    if (client) mongoc_client_destroy(client);
    mongoc_cleanup();
    logger->info("Disconnected from Mongodb");
}

void MongoAdapter::init()
{
    mongoc_init();

    // Format connection string
    std::stringstream uri;
    std::string connectionString;
    uri << mongoUrl;
    uri << ":";
    uri << mongoPort;
    uri >> connectionString;

    client = mongoc_client_new(connectionString.c_str());
    mongoc_client_set_appname(client, "slserver");
    database = mongoc_client_get_database(client, M_DATABASE);
    collection = mongoc_client_get_collection(client, M_DATABASE, M_COLLECTION);
    logger->info("Using Mongodb (%s:%d)", mongoUrl.c_str(), mongoPort);
}

void MongoAdapter::addAction(std::string name, std::string bytecode, int timeout, int maxmem)
{
    std::lock_guard<std::mutex> lock(_lock);

    bson_oid_t oid;
    bson_error_t error;
    bson_t *doc = bson_new();

    bson_oid_init(&oid, nullptr);
    BSON_APPEND_OID(doc, "_id", &oid);
    BSON_APPEND_UTF8(doc, "name", name.c_str());
    BSON_APPEND_INT32(doc, "timeout", timeout);
    BSON_APPEND_INT32(doc, "maxmem", maxmem);
    BSON_APPEND_INT32(doc, "size", bytecode.size());
    BSON_APPEND_BINARY(doc, "bytecode", BSON_SUBTYPE_BINARY, (const uint8_t *) bytecode.c_str(), bytecode.length());
    if(!mongoc_collection_insert_one(collection, doc, nullptr, nullptr, &error))
    {
        logger->error("Mongodb error: %s", error.message);
    }
    bson_destroy(doc);
}

const ActionDefinition MongoAdapter::getAction(std::string name)
{
    std::lock_guard<std::mutex> lock(_lock);

    const bson_t *doc;
    bson_t *query = bson_new();
    BSON_APPEND_UTF8(query, "name", name.c_str());

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, query, nullptr, nullptr);

    ActionDefinition action;

    while (mongoc_cursor_next(cursor, &doc))
    {
        bson_iter_t it;

        bson_iter_init(&it, doc);
        bson_iter_find(&it, "name");
        action.name = bson_iter_utf8(&it, nullptr);

        bson_iter_init(&it, doc);
        bson_iter_find(&it, "size");
        action.size = bson_iter_int32(&it);

        bson_iter_init(&it, doc);
        bson_iter_find(&it, "timeout");
        action.timeout = bson_iter_int32(&it);

        bson_iter_init(&it, doc);
        bson_iter_find(&it, "maxmem");
        action.maxmem = bson_iter_int32(&it);

        bson_iter_init(&it, doc);
        bson_iter_find(&it, "bytecode");
        const char *buffer;
        uint32_t len;
        bson_subtype_t subtype;
        bson_iter_binary(&it, &subtype, &len, (const uint8_t **) &buffer);
        std::string bytecode(buffer, len);
        action.bytecode = bytecode;
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    return action;
}

bool MongoAdapter::hasAction(std::string name)
{
    std::lock_guard<std::mutex> lock(_lock);

    bson_t *query = bson_new();
    bson_error_t error;
    BSON_APPEND_UTF8(query, "name", name.c_str());

    int count = mongoc_collection_count(collection, MONGOC_QUERY_NONE,
                                        query, 0, 0, nullptr, &error);

    bson_destroy(query);

    if (count < 0)
    {
        logger->error("Mongodb error: %s", error.message);
        return true;
    }

    return count > 0;
}

void MongoAdapter::getActions(std::vector<ActionDefinition> &list)
{
    std::lock_guard<std::mutex> lock(_lock);

    const bson_t *doc;
    bson_t *query = bson_new();
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, query, nullptr, nullptr);

    while (mongoc_cursor_next(cursor, &doc))
    {
        ActionDefinition action;

        bson_iter_t it;

        bson_iter_init(&it, doc);
        bson_iter_find(&it, "size");
        action.size = bson_iter_int32(&it);

        bson_iter_init(&it, doc);
        bson_iter_find(&it, "name");
        action.name = bson_iter_utf8(&it, nullptr);

        bson_iter_init(&it, doc);
        bson_iter_find(&it, "maxmem");
        action.maxmem = bson_iter_int32_unsafe(&it);

        bson_iter_init(&it, doc);
        bson_iter_find(&it, "timeout");
        action.timeout = bson_iter_int32_unsafe(&it);

        list.push_back(action);
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
}

void MongoAdapter::deleteAction(std::string &name)
{
    std::lock_guard<std::mutex> lock(_lock);

    bson_t *query = bson_new();
    bson_error_t error;
    BSON_APPEND_UTF8(query, "name", name.c_str());
    if(!mongoc_collection_delete_one(collection, query, nullptr, nullptr, &error))
    {
        logger->error("Mongodb error: %s", error.message);
    }
    bson_destroy(query);
}
