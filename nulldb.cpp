#include "nulldb.h"

void Nulldb::addAction(std::string name, std::string bytecode)
{
    storage[name] = bytecode;
}

const std::string& Nulldb::getAction(std::string name)
{
    return storage[name];
}

bool Nulldb::hasAction(std::string name)
{
    return storage.find(name) != storage.end();
}
