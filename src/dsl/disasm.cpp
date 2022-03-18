#include <sstream>
#include "disasm.h"

VM::DisasmResult VM::disasm(const uint8_t *code)
{
    DisasmResult result;

    auto opcode = static_cast<VM::Instruction>(code[0]);

    std::stringstream ss;
    uint16_t imm16 = static_cast<uint16_t>(code[1]) | (static_cast<uint16_t>(code[2]) << 8);
    switch(opcode)
    {
    case VM::Instruction::LIT:
        ss << "\tLIT " << imm16 << "\n";
        result.m_bytes = 3;
        break;
    case VM::Instruction::LOAD:
        ss << "\tLOAD @" << imm16 << "\n";
        result.m_bytes = 3;
        break;
    case VM::Instruction::STORE:
        ss << "\tSTORE @" << imm16 << "\n";
        result.m_bytes = 3;
        break;        
    case VM::Instruction::CALL:
        ss << "\tCALL @" << imm16 << "\n";
        result.m_bytes = 3;
        break;        
    case VM::Instruction::SHR:
        ss << "\tSHR\n";
        result.m_bytes = 1;
        break;     
    case VM::Instruction::DEC:
        ss << "\tDEC\n";
        result.m_bytes = 1;
        break;
    case VM::Instruction::INC:
        ss << "\tINC\n";
        result.m_bytes = 1;
        break;
    case VM::Instruction::RET:
        ss << "\tINC\n";
        result.m_bytes = 1;
        break;        
    case VM::Instruction::POP:
        ss << "\tPOP\n";
        result.m_bytes = 1;
        break;        
    case VM::Instruction::JNZ:
        ss << "\tJNZ @" << imm16 << "\n";
        result.m_bytes = 3;
        break;
    case VM::Instruction::WAIT:
        ss << "\tWAIT\n";
        result.m_bytes = 1;
        break;        
    case VM::Instruction::RES:
        ss << "\tRES " << static_cast<int>(code[1]) << "\n";
        result.m_bytes = 2;
        break;        
    case VM::Instruction::POPN:
        ss << "\tPOPN " << static_cast<int>(code[1]) << "\n";
        result.m_bytes = 2;
        break;
    case VM::Instruction::SETOUTPUT:
        ss << "\tSETOUTPUT " << static_cast<int>(code[1]) << "\n";
        result.m_bytes = 2;
        break;
    case VM::Instruction::SETINPUT:
        ss << "\tSETOUTPUT " << static_cast<int>(code[1]) << "\n";
        result.m_bytes = 2;
        break;
    case VM::Instruction::SETPIN:
        ss << "\tSETPIN " << static_cast<int>(code[1]) << "\n";
        result.m_bytes = 2;
        break;
    case VM::Instruction::GETPIN:
        ss << "\tPINBIT " << static_cast<int>(code[1]) << "\n";
        result.m_bytes = 2;
        break;
    case VM::Instruction::SETBIT:
        ss << "\tSETBIT " << static_cast<int>(code[1]) << "\n";
        result.m_bytes = 2;
        break;
    case VM::Instruction::GETBIT:
        ss << "\tGETBIT " << static_cast<int>(code[1]) << "\n";
        result.m_bytes = 2;
        break;
    default:
        ss << "\t??????\n";
        result.m_bytes = 1;
        break;    
    }

    result.m_txt = ss.str();
    return result;
}

