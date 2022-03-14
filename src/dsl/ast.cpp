#include "ast.h"

std::ostream& operator<<(std::ostream& out, const ASTNode::NodeType &t)
{
    switch(t)
    {        
    case ASTNode::NodeType::TOPNODE:
        out << "TOPNODE";
        break;
    case ASTNode::NodeType::PINDEF:
        out << "PINDEF";
        break;    
    case ASTNode::NodeType::PROCDEF:
        out << "PROCDEF";
        break;
    case ASTNode::NodeType::CALL:
        out << "CALL";
        break;
    case ASTNode::NodeType::SHR:
        out << "SHR";
        break;
    case ASTNode::NodeType::INT:
        out << "INT";
        break;
    case ASTNode::NodeType::IODIR:
        out << "IODIR";
        break;
    case ASTNode::NodeType::VARIABLE:
        out << "VARIABLE";
        break;
    case ASTNode::NodeType::EXPRESSION:
        out << "EXPRESSION";
        break;
    case ASTNode::NodeType::ASSIGN:
        out << "ASSIGN";
        break;                                           
    case ASTNode::NodeType::WAIT:
        out << "WAIT";
        break;
    case ASTNode::NodeType::REP:
        out << "REP";
        break;
    case ASTNode::NodeType::ENDREP:
        out << "ENDREP";
        break;
    case ASTNode::NodeType::RETURN:
        out << "RETURN";
        break;
    case ASTNode::NodeType::VARDEF:
        out << "VARDEF";
        break;
    case ASTNode::NodeType::ARG:
        out << "ARG";
        break; 
    case ASTNode::NodeType::SETPINDIR:
        out << "SETPINDIR";
        break;        
    default:
        out << "???";
        break;        
    }
    return out;
}
