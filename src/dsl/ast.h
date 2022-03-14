#pragma once
#include <vector>
#include <memory>
#include "dsltypes.h"
#include "lexer.h"

struct ASTNode;

struct AbstractConstVisitor
{
    virtual void visit(const ASTNode *node) = 0;
};

struct AbstractVisitor
{
    virtual void visit(ASTNode *node) = 0;
};


struct ASTNode
{
    enum class NodeType
    {
        UNKNOWN = 0,
        TOPNODE,
        PINDEF,
        PROCDEF,
        CALL,
        SHR,
        INT,
        IODIR,
        VARIABLE,
        EXPRESSION,
        ASSIGN,
        WAIT,
        REP,
        ENDREP,
        RETURN,
        VARDEF,
        ARG,
        SETPINDIR
    };

    ASTNode() = default;

    ASTNode(NodeType t) : m_type(t) {}

    NodeType m_type     = NodeType::UNKNOWN;
    IOType   m_iotype   = IOType::UNKNOWN;

    int         m_integer = -1; // literal, bit-index
    std::string m_name;         // proc name, var name
    std::vector<std::unique_ptr<ASTNode> > m_children;

    void accept(AbstractConstVisitor &v) const
    {
        v.visit(this);
    }

    void accept(AbstractVisitor &v)
    {
        v.visit(this);
    }    
};

std::ostream& operator<<(std::ostream& out, const ASTNode::NodeType &t);
