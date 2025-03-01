/**
 * @file StructObject.hpp
 * @author Edward Palmer
 * @date 2024-11-24
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once
#include "BaseNode.hpp"
#include "BaseObject.hpp"
#include "Exceptions.hpp"
#include "Scope.hpp"
#include "StructDefinitionObject.hpp"
#include <ostream>
#include <string>
#include <unordered_set>

/**
 * struct SomeStruct a;
 */
class StructObject : public BaseObject, public BaseNode
{
public:
    StructObject() = delete;

    /**
     * Create a new empty StructObject with the name.
     */
    StructObject(std::string typeName_, std::string name_);

    /**
     * Assignment operator.
     */
    StructObject &operator=(const BaseObject &other) override;

    /**
     * Finishes initializing the struct object and links to the scope. Ownership
     * should pass to the scope from the AST. Returns a pointer to itself.
     */
    BaseObject *evaluate(Scope &scope) override;

    // TODO: - implement in future.
    StructObject *clone() const override
    {
        ThrowException("not implemented!");
    }

    /**
     * Returns a description of the struct.
     */
    friend std::ostream &operator<<(std::ostream &os, const BaseObject &);

    friend class StructAccessNode;

protected:
    /**
     * The struct has its own scope for storing its own variables. It does not
     * inherit from any parent scopes. Literally just used for storing stuff.
     * When this instance goes out of scope, all variables will be deleted.
     */
    Scope instanceScope;

    /**
     * Stores names of all variables stored in set.
     */
    std::unordered_set<std::string> variableNames;

    /**
     * Ony a single evaluate call allowed.
     */
    bool active{false};

    /**
     * Stores name of instance and the type.
     */
    std::string name;
    std::string typeName;

    /**
     * Store the struct definition once active.
     */
    StructDefinitionObject *structDefinition{nullptr};
};
