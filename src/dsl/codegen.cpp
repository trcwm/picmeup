#include "codegen.h"

void CodeGen::CodeGenVisitor::visit(const ASTNode *node) 
{
    bool skipFirstChild = false;

    // pre-process
    switch(node->m_type)
    {
    case ASTNode::NodeType::PINDEF:
        {
            auto &pin = m_syms.create(node->m_name, SymTbl::Symbol::Type::PIN);
            pin.m_integer = node->m_integer;    // pin id
            pin.m_iotype  = node->m_iotype;
        }
        break;
    case ASTNode::NodeType::PROCDEF:
        {
            comment("# PROC ");
            comment(node->m_name);
            comment("\n");
            auto &proc = m_syms.create(node->m_name, SymTbl::Symbol::Type::PROC);
            proc.m_integer = node->m_children.size();    // number of arguments
            proc.m_address = m_address;

            // add arguments as local variables
            m_syms.enterScope();          
        }
        break;
    case ASTNode::NodeType::ARG:
        {
            comment("# ARG ");
            comment(node->m_name);
            comment("\n");
            m_syms.create(node->m_name, SymTbl::Symbol::Type::VARIABLE);
        }
        break;        
    case ASTNode::NodeType::VARDEF:
        {
            auto &varsym = m_syms.create(node->m_name, SymTbl::Symbol::Type::VARIABLE);
        }
        break;
    case ASTNode::NodeType::SETPINDIR:
        {
            comment("# SET PIN DIR\n");
            auto opt = m_syms.find(node->m_name);
            if (!opt)
            {
                std::cerr << "Unknown pin " << node->m_name << "\n";
            }
            else
            {
                emit("SETDIR ", opt->m_integer);
                emit(static_cast<int>(node->m_iotype));
            }
        }
        break;
    case ASTNode::NodeType::VARIABLE:
        {
            auto opt = m_syms.find(node->m_name);
            if (!opt)
            {
                std::cerr << "Unknown variable " << node->m_name << "\n";
            }
            else
            {
                if (opt->m_type == SymTbl::Symbol::Type::PIN)
                {
                    comment("# READ PIN ");
                    comment(node->m_name);
                    comment("\n");

                    emit("READPIN ");
                    emit(opt.value().m_integer);
                }
                else
                {
                    comment("# VARIABLE ");
                    comment(node->m_name);

                    if (node->m_integer >= 0)
                    {
                        comment("[");
                        comment(node->m_integer);
                        comment("]\n");
                        // get bit
                        emit("GETBIT ", node->m_integer);
                        emit(opt.value().m_address);
                    }
                    else
                    {
                        comment("\n");
                        // regular 16-bit load
                        emit("LOAD ");
                        emit(opt.value().m_address);
                    }                     
                }
            }
        }
        break;  
    case ASTNode::NodeType::REP:
        {
            comment("# REP\n");
            comment(node->m_name);

            // exception: visit the first child node
            // to get REP expression
            visit(node->m_children.front().get());
            skipFirstChild = true;

            m_labels.push(m_labelCount++);
            emitLabel(m_labels.top());
        }
        break;
    case ASTNode::NodeType::INT:    // literal
        {
            emit("LIT ");
            emit(node->m_integer);
        }
        break;
    }

    // child visit
    bool isFirstChild = true;
    for(auto &child : node->m_children)
    {
        if (!(isFirstChild && skipFirstChild))
        {
            visit(child.get());
        }
        isFirstChild = false;
    }

    // post-process
    switch(node->m_type)
    {
    case ASTNode::NodeType::PROCDEF:
        {
            // TODO: stack cleanup
            emit("RET\n");
            comment("# ENDPROC ");
            comment(node->m_name);
            comment("\n\n");

            m_syms.leaveScope();
        }
        break;
    case ASTNode::NodeType::CALL:    // call function
        {
            auto opt = m_syms.find(node->m_name);
            if (!opt)
            {
                std::cerr << "Unknown function " << node->m_name << "\n";
            }
            else
            {
                comment("# calling ");
                comment(node->m_name);
                comment("\n");    
                emit("CALL ");
                emit(opt.value().m_address);  // address
            }
        }
        break;       
    case ASTNode::NodeType::ASSIGN:
        {
            auto opt = m_syms.find(node->m_name);
            if (!opt)
            {
                std::cerr << "Unknown variable " << node->m_name << "\n";
            }
            else
            {
                if (opt->m_type == SymTbl::Symbol::Type::PIN)
                {
                    comment("# ASSIGN PIN ");
                    comment(node->m_name);
                    comment("\n");

                    emit("SETPIN ");
                    emit(opt.value().m_integer);
                }
                else
                {
                    comment("# ASSIGN VARIABLE ");
                    comment(node->m_name);
                    comment("\n");

                    if (node->m_integer >= 0)
                    {
                        // set bit
                        emit("SETBIT ", node->m_integer);
                        emit(opt.value().m_address);
                    }
                    else
                    {
                        // regular 16-bit store
                        emit("STORE ");
                        emit(opt.value().m_address);
                    }                                     
                }                                
            }
        }
        break;                         
    case ASTNode::NodeType::WAIT:    // literal
        {
            emit("WAIT\n");
        }
        break;        
    case ASTNode::NodeType::SHR:
        {
            emit("SHR\n");
        }
        break;        
    case ASTNode::NodeType::ENDREP:
        {
            emit("DEC\n");
            emit("JNZ loc_");
            emit(m_labels.top());
            m_labels.pop();
            emit("POP\n");
            comment("# ENDREP\n");
            //emit("RET\n");
        }
        break;
    }        
}
