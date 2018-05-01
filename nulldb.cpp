#include "nulldb.h"

void Nulldb::addAction(std::string name, std::string bytecode, int timeout, int maxmem)
{    
    ActionDefinition *def = new ActionDefinition;
    def->bytecode = bytecode;
    def->timeout = timeout;
    def->maxmem = maxmem;
    def->name = name;
    def->size = bytecode.size();

    storage[name] = std::unique_ptr<ActionDefinition>(def);
}

Nulldb::~Nulldb()
{
}

const ActionDefinition Nulldb::getAction(std::string name)
{
    return *storage[name];
}

bool Nulldb::hasAction(std::string name)
{
    return storage.find(name) != storage.end();
}

void Nulldb::getActions(std::vector<ActionDefinition> &list)
{
    ActionMap::iterator it;
    for (it = storage.begin(); it != storage.end(); it++)
    {
        list.push_back(*it->second);
    }
}

void Nulldb::deleteAction(std::string &name)
{
    storage.erase(name);
}
