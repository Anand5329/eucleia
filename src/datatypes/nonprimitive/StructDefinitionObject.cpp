/**
 * @file StructDefinitionObject.cpp
 * @author Edward Palmer
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "StructDefinitionObject.hpp"

StructDefinitionObject::~StructDefinitionObject()
{
    // We are responsible for deleting nodes.
    for (auto *ptr : variableDefs)
    {
        delete ptr;
    }

    variableDefs.clear();
}


void StructDefinitionObject::buildVariableDefHashMap()
{
    if (!allVariableDefsMap.empty())
    {
        return; // Already built.
    }

    if (parent)
    {
        parent->buildVariableDefHashMap();

        // Copy parent definitions.
        allVariableDefsMap = parent->allVariableDefsMap;
    }

    // Now we add our own variable definitions and check for any clashes.
    for (AddVariableNode *variableDef : variableDefs)
    {
        auto iter = allVariableDefsMap.find(variableDef->name);
        if (iter != allVariableDefsMap.end())
        {
            EucleiaError("duplicate class variable '%s'.", iter->first.c_str());
        }

        allVariableDefsMap[variableDef->name] = variableDef;
    }
}
