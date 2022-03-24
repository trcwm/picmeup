#include "codegen.h"

void CodeGen::CodeGenVisitor::emit(VM::Instruction opcode)
{
    m_code.push_back(static_cast<uint8_t>(opcode));
}

void CodeGen::CodeGenVisitor::emit(VM::Instruction opcode, uint16_t value)
{
    m_code.push_back(static_cast<uint8_t>(opcode));
    m_code.push_back(value & 0xFF);
    m_code.push_back((value >> 8) & 0xFF);
}

void CodeGen::CodeGenVisitor::emit(VM::Instruction opcode, uint8_t value)
{
    m_code.push_back(static_cast<uint8_t>(opcode));
    m_code.push_back(value);
}

void CodeGen::CodeGenVisitor::emit(VM::Instruction opcode, uint8_t v1, uint8_t v2)
{
    m_code.push_back(static_cast<uint8_t>(opcode));
    m_code.push_back(static_cast<uint8_t>(v1));
    m_code.push_back(static_cast<uint8_t>(v2));
}

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
                switch(node->m_iotype)
                {
                case IOType::INPUT:
                    emit(VM::Instruction::SETINPUT, static_cast<uint8_t>(opt.value().m_integer));
                    break;
                case IOType::OUTPUT:
                    emit(VM::Instruction::SETOUTPUT, static_cast<uint8_t>(opt.value().m_integer));
                    break;
                default:
                    std::cerr << "Unknown iotype\n";
                }
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

                    emit(VM::Instruction::GETPIN, static_cast<uint8_t>(opt.value().m_integer));
                    //emit("READPIN ");
                    //emit(opt.value().m_integer);
                }
                else
                {
                    comment("# VARIABLE ");
                    comment(node->m_name);

                    emit(VM::Instruction::LOAD, static_cast<uint8_t>(opt.value().m_address));

                    // check for bit access
                    if (node->m_integer >= 0)
                    {
                        comment("[");
                        comment(node->m_integer);
                        comment("]\n");
                        
                        // get bit
                        emit(VM::Instruction::GETBIT, static_cast<uint8_t>(node->m_integer));
                        //emit("GETBIT ", node->m_integer);
                        //emit(opt.value().m_address);
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

            m_labels.push(m_code.size() /* address of next instruction */);
            //emitLabel(m_labels.top());
        }
        break;
    case ASTNode::NodeType::INT:    // literal
        {
            emit(VM::Instruction::LIT, static_cast<uint16_t>(node->m_integer));
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
            emit(VM::Instruction::RET);
            //emit("RET\n");
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
                //emit("CALL ");
                //emit(opt.value().m_address);  // address
                emit(VM::Instruction::CALL, static_cast<uint16_t>(opt.value().m_address));
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

                    //emit("SETPIN ");
                    //emit(opt.value().m_integer);
                    emit(VM::Instruction::SETPIN, static_cast<uint8_t>(opt.value().m_integer));
                }
                else
                {
                    comment("# ASSIGN VARIABLE ");
                    comment(node->m_name);
                    comment("\n");

                    if (node->m_integer >= 0)
                    {
                        // set bit
                        emit(VM::Instruction::SETBIT, static_cast<uint8_t>(node->m_integer));
                        //emit("SETBIT ", );
                        //emit(opt.value().m_address);
                    }
                    else
                    {
                        // regular 16-bit store
                        emit(VM::Instruction::STORE, static_cast<uint8_t>(opt.value().m_address));
                        //emit("STORE ");
                        //emit(opt.value().m_address);
                    }                                     
                }                                
            }
        }
        break;                         
    case ASTNode::NodeType::WAIT:    // literal
        {
            emit(VM::Instruction::WAIT);
        }
        break;        
    case ASTNode::NodeType::SHR:
        {
            emit(VM::Instruction::SHR);
        }
        break;        
    case ASTNode::NodeType::ENDREP:
        {
            emit(VM::Instruction::DEC);
            emit(VM::Instruction::JNZ, static_cast<uint16_t>(m_labels.top()));
            m_labels.pop();
            emit(VM::Instruction::POP);
            emit(VM::Instruction::RET);

            //emit("DEC\n");
            //emit("JNZ loc_");
            //emit(m_labels.top());
            //m_labels.pop();
            //emit("POP\n");
            //comment("# ENDREP\n");
            //emit("RET\n");
        }
        break;
    }        
}
