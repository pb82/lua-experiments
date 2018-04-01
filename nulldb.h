#ifndef NULLDB_H
#define NULLDB_H

#include <map>
#include <string>

#include "persistence.h"

/**
 * @brief The Nulldb class
 * Implementation of the `Nulldb` default no-persistence provider.
 * Only in memory storage. Can be used for development.
 */
class Nulldb : public Persistence
{
public:
    Nulldb() { }

    void init()
    {
        // Nothing to initialize
    }

    void addAction(std::string name, std::string bytecode);
    const std::string& getAction(std::string name);
private:
    std::map<std::string, std::string> storage;
};

#endif // NULLDB_H
