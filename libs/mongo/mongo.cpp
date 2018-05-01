#include <iostream>
#include <string>

#include <mongoc.h>

#include "../../plugin.h"
#include "../../json/Printer.hpp"
#include "../../json/Parser.hpp"

class MongoPlugin : public Plugin
{
public:
    void setup(JSON::Value &config)
    {
        this->url = config["url"].as<std::string>();
        this->database = config["database"].as<std::string>();

        client = mongoc_client_new(url.c_str());
        db = mongoc_client_get_database(client, database.c_str());
    }

    JSON::Value insert(std::string collection, JSON::Value &value)
    {
        std::string json = printer.print(value);
        bson_error_t error;
        bson_t *doc = bson_new_from_json((const uint8_t*) json.c_str(), json.size(), &error);
        bson_oid_t oid;
        bson_oid_init(&oid, nullptr);
        BSON_APPEND_OID(doc, "_id", &oid);

        mongoc_collection_t *col = mongoc_client_get_collection(client, database.c_str(), collection.c_str());
        if (!mongoc_collection_insert_one(col, doc, nullptr, nullptr, &error))
        {
            mongoc_collection_destroy(col);
            bson_destroy(doc);
            return false;
        }

        char oidString[25];
        bson_oid_to_string(&oid, oidString);
        mongoc_collection_destroy(col);
        bson_destroy(doc);

        return std::string(oidString);
    }

    JSON::Value query(std::string collection, JSON::Value &query)
    {
        const bson_t *doc;
        mongoc_collection_t *col = mongoc_client_get_collection(client, database.c_str(), collection.c_str());
        std::string json = printer.print(query);

        bson_error_t error;
        bson_t *_query = bson_new_from_json((const uint8_t*) json.c_str(), json.size(), &error);
        mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(col, _query, nullptr, nullptr);

        JSON::Array result;

        while (mongoc_cursor_next(cursor, &doc))
        {
            JSON::Value val;
            char *json = bson_as_json(doc, nullptr);
            parser.parse(val, json);
            result.push_back(val);
            bson_free(json);
        }

        bson_destroy(_query);
        mongoc_cursor_destroy(cursor);
        mongoc_collection_destroy(col);
        return result;
    }

    JSON::Value call(const std::string& action, JSON::Value& payload)
    {
        if (action.compare("insert") == 0)
        {
            return insert(payload["collection"].as<std::string>(), payload["data"]);
        } else if (action.compare("query") == 0)
        {
            return query(payload["collection"].as<std::string>(), payload["query"]);
        }

        return false;
    }

    std::string name()
    {
        return "mongo";
    }
private:
    std::string url;
    std::string database;
    JSON::Printer printer;
    JSON::Parser parser;

    mongoc_client_t *client;
    mongoc_database_t *db;
};

extern "C" Plugin *create()
{
    return new MongoPlugin;
}

