#ifndef NULLDB_H
#define NULLDB_H

#include <map>
#include <string>
#include <memory>

#include "persistence.h"

typedef std::map<std::string, std::unique_ptr<ActionDefinition>> ActionMap;

/**
 * @brief The Nulldb class
 * Implementation of the `Nulldb` default no-persistence provider.
 * Only in memory storage. Can be used for development.
 */
class Nulldb : public Persistence
{
public:
    Nulldb() { }
    ~Nulldb();

    void init()
    {
        // Nothing to initialize
    }

    void addAction(std::string name, std::string bytecode, int timeout, int maxmem);
    const ActionDefinition getAction(std::string name);
    bool hasAction(std::string name);
    void getActions(std::vector<ActionDefinition> &list);
    void deleteAction(std::string &name);
private:
    ActionMap storage;
};

#endif // NULLDB_H
