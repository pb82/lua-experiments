#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>

/**
 * @brief The PersistenceError class
 * Thrown when problems occur during setup or usage of
 * any of the persistence layers
 */
class PersistenceError : public std::runtime_error
{
public:
    PersistenceError(std::string message)
        : std::runtime_error(message)
    {
    }
};

/**
 * @brief The Persistence class
 * Base class for all services that allow storage and retrieval
 * of resources
 */
class Persistence
{
public:
    virtual ~Persistence() { }

    /**
     * @brief init Allows the persistence layer to set up the backend
     * data source (create tables, etc.)
     */
    virtual void init() = 0;

    /**
     * @brief addAction Store a action
     * @param name The name under which the action will be stored
     * @param bytecode The compiled lua bytecode encoded in a string
     */
    virtual void addAction(std::string name, std::string bytecode) = 0;

    /**
     * @brief getAction Retrieve the bytecode of an action by name
     * @param name The name of the action
     * @return A reference to a string in which the bytecode is stored
     */
    virtual const std::string& getAction(std::string name) = 0;

    /**
     * @brief hasAction Checks if an action with a given name exists
     * @param name The name of the action
     * @return bool true if an action with the given name exists
     */
    virtual bool hasAction(std::string name) = 0;
};

#endif // PERSISTENCE_H
