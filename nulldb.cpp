#include "nulldb.h"

void Nulldb::addAction(std::string name, std::string bytecode)
{
    storage[name] = bytecode;
}

const std::string& Nulldb::getAction(std::string name)
{
    return storage[name];
}
