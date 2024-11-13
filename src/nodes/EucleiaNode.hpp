//
//  EucleiaNode.hpp
//  Eucleia
//
//  Created by Edward on 14/01/2024.
//

#ifndef EuclieaNode_hpp
#define EuclieaNode_hpp

#include "EucleiaObject.hpp"
#include "EucleiaScope.hpp"
#include <memory>
#include <string>

class BaseNode
{
public:
    BaseNode() = default;
    virtual ~BaseNode() = default;


    template <class TNode>
    const TNode &castNode() const
    {
        return static_cast<TNode &>(*this);
    }


    template <class TNode>
    TNode &castNode()
    {
        return static_cast<TNode &>(*this);
    }


    template <class TNode>
    bool isNodeType() const
    {
        return typeid(*this) == typeid(TNode);
    }


    bool typesMatch(const BaseNode &other) const
    {
        return typeid(*this) == typeid(other);
    }

    virtual BaseObject *evaluate(Scope &scope) = 0;

    template <typename T>
    T *evaluate(Scope &scope)
    {
        return static_cast<T *>(evaluate(scope));
    }
};

class AddBoolNode : public BaseNode
{
public:
    AddBoolNode(bool state_) : boolObject(state_) {}

    // Creates a BoolObject in the scope and returns managed pointer to it.
    BoolObject *evaluate(Scope &scope) override;


    BoolObject boolObject;
};


class AddIntNode : public BaseNode
{
public:
    AddIntNode(long int_) : intObject(int_) {}

    // Creates an IntObject in the scope and returns a managed pointer to it.
    IntObject *evaluate(Scope &scope) override;


    IntObject intObject;
};


class AddFloatNode : public BaseNode
{
public:
    AddFloatNode(double float_) : floatObject(float_) {}

    // Returns a FloatObject in the current scope and returns a managed pointer.
    FloatObject *evaluate(Scope &scope) override;


    FloatObject floatObject;
};

class AddStringNode : public BaseNode
{
public:
    AddStringNode(std::string string_) : stringObject(std::move(string_)) {}

    // Creates a StringObject in the scope and returns managed pointer to it.
    StringObject *evaluate(Scope &scope) override;


    StringObject stringObject;
};


class LookupVariableNode : public BaseNode
{
public:
    LookupVariableNode(std::string variableName_) : variableName(std::move(variableName_)) {}

    // Returns the object in the scope associated with a variable name.
    BaseObject *evaluate(Scope &scope) override;


    std::string variableName;
};


class AddNewVariableNode : public LookupVariableNode
{
public:
    enum VariableType // TODO: - don't use an enum. Somehow use a typeid to pass -in the desired object type to create.
    {
        Int,
        Float,
        Bool,
        String,
        Array
        // Function	<-- allow this to be set.
    };

    std::string description() const;

    AddNewVariableNode(std::string variableName_, VariableType variableType_)
        : LookupVariableNode(std::move(variableName_)),
          variableType(variableType_) {}

    // Creates a new empty variable of a given type to the scope (i.e. int a;).
    BaseObject *evaluate(Scope &scope) override;


    //  Type checking for variable assignment.
    bool passesAssignmentTypeCheck(const BaseObject &assignObject) const;

    const VariableType variableType;
};


class ProgramNode : public BaseNode // TODO: - write proper destructor to avoid memory leaks.
{
public:
    ProgramNode(std::vector<BaseNode *> nodes_) : programNodes(std::move(nodes_)) {}

    BaseNode *operator[](size_t index) const
    {
        assert(index < programNodes.size());
        return programNodes[index];
    }

    // Evaluates a vector of nodes sequentially. Returns nullptr.
    BaseObject *evaluate(Scope &scope) override;


    std::vector<BaseNode *> programNodes;
};


class AddArrayNode : public ProgramNode
{
public:
    AddArrayNode(std::vector<BaseNode *> nodes_) : ProgramNode(std::move(nodes_)) {}

    // Create a new ArrayObject in the current scope and return a managed pointer to it.
    ArrayObject *evaluate(Scope &scope) override;
};


class FileNode : public ProgramNode
{
public:
    FileNode(std::vector<BaseNode *> nodes_) : ProgramNode(std::move(nodes_)) {}

    // Evaluates a file treating it as one large program. Does NOT create an inner
    // scope node in order to ensure that any functions declared in this file will
    // be added to the master file by using the same global scope - TODO: - think about this logic.
    BaseObject *evaluate(Scope &globalScope) override;
};

// TODO: - rewrite this...
class ArrayAccessNode : public BaseNode
{
public:
    ArrayAccessNode(BaseNode *arrayName_, BaseNode *arrayIndex_)
        : arrayName(static_cast<AddNewVariableNode *>(arrayName_)),
          arrayIndex(static_cast<AddIntNode *>(arrayIndex_))
    {
    }

    BaseObject *evaluate(Scope &scope) override;

    AddNewVariableNode *arrayName{nullptr};
    AddIntNode *arrayIndex{nullptr};
};


class IfNode : public BaseNode
{
public:
    IfNode(BaseNode *condition_,
           BaseNode *thenDo_,
           BaseNode *elseDo_ = nullptr)
        : ifCondition(condition_),
          thenDo(thenDo_),
          elseDo(elseDo_)
    {
    }

    // Evaluate an if/else statement in current scope. Returns nullptr.
    BaseObject *evaluate(Scope &scope) override;


    BaseNode *ifCondition{nullptr};
    BaseNode *thenDo{nullptr};
    BaseNode *elseDo{nullptr}; // Links to next argument (if node!).
};


class DoWhileNode : public BaseNode
{
public:
    DoWhileNode(BaseNode *condition_, BaseNode *body_)
        : condition(condition_),
          body(body_)
    {
    }

    // Evaluate a do-while loop in current scope. Returns nullptr.
    BaseObject *evaluate(Scope &scope) override;


    BaseNode *condition{nullptr};
    BaseNode *body{nullptr};
};


class WhileNode : public DoWhileNode
{
public:
    WhileNode(BaseNode *_condition, BaseNode *_body)
        : DoWhileNode(_condition, _body)
    {
    }

    // Evaluate a while loop in current scope. Returns nullptr.
    BaseObject *evaluate(Scope &scope) override;
};


class ForLoopNode : public BaseNode
{
public:
    ForLoopNode(BaseNode *start_, BaseNode *condition_, BaseNode *update_, BaseNode *body_)
        : start(start_),
          condition(condition_),
          update(update_),
          body(body_)
    {
    }


    // Evaluates a for-loop in current scope. Returns nullptr.
    BaseObject *evaluate(Scope &scope) override;


    BaseNode *start{nullptr};
    BaseNode *condition{nullptr};
    BaseNode *update{nullptr};
    BaseNode *body{nullptr};
};


class FunctionCallNode : public BaseNode
{
public:
    FunctionCallNode(BaseNode *funcName_, BaseNode *funcArgs_)
        : funcName(static_cast<AddNewVariableNode *>(funcName_)),
          funcArgs(static_cast<ProgramNode *>(funcArgs_))
    {
    }

    // TODO: - don't forget to do performance profiling for Fib sequence and see memory requirements for old and new version
    // TODO: - create a new PR after this for parser to store all nodes in AST in flat array using pointers with method to delete by walking along array.
    BaseObject *evaluate(Scope &scope) override;

    BaseObject *evaluateFunctionBody(BaseNode &funcBody, Scope &funcScope);


    AddNewVariableNode *funcName{nullptr};
    ProgramNode *funcArgs{nullptr};
};


class FunctionNode : public FunctionCallNode, public std::enable_shared_from_this<FunctionNode>
{
public:
    FunctionNode(BaseNode *funcName_, BaseNode *funcArgs_, BaseNode *funcBody_)
        : FunctionCallNode(funcName_, funcArgs_),
          funcBody(static_cast<ProgramNode *>(funcBody_))
    {
    }

    BaseObject *evaluate(Scope &scope) override;

    ProgramNode *funcBody{nullptr};
};


class BreakNode : public BaseNode
{
public:
    BaseObject *evaluate(Scope &scope) override;
};


class ReturnNode : public BaseNode
{
public:
    ReturnNode(BaseNode *returnValue_ = nullptr)
        : returnNode(returnValue_)
    {
    }

    BaseObject *evaluate(Scope &scope) override;

    BaseNode *returnNode{nullptr};
};


class AssignNode : public BaseNode
{
public:
    AssignNode(BaseNode *left_, BaseNode *right_)
        : left(left_),
          right(right_)
    {
    }

    BaseObject *evaluate(Scope &scope) override;

    BaseObject *evaluateArrayAccess(Scope &scope);

    BaseNode *left{nullptr};
    BaseNode *right{nullptr};
};


class BinaryNode : public AssignNode
{
public:
    BinaryNode(BaseNode *left_, BaseNode *right_, std::string binaryOperator_)
        : AssignNode(left_, right_),
          binaryOperator(std::move(binaryOperator_))
    {
    }

    BaseObject *evaluate(Scope &scope) override;

    std::string binaryOperator;


    BaseObject *applyOperator(Scope &scope, const BaseObject &left, const BaseObject &right) const;

    BaseObject *applyOperator(Scope &scope, const IntObject &left, const IntObject &right) const;
    BaseObject *applyOperator(Scope &scope, const FloatObject &left, const FloatObject &right) const;
    BaseObject *applyOperator(Scope &scope, const StringObject &left, const StringObject &right) const;
};


#endif /* EucleiaNode_hpp */
