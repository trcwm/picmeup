#include "vm.h"

uint16_t VM::Machine::getWord(const uint16_t address) const
{
    uint16_t v = m_code[address];
    v |= static_cast<uint16_t>(m_code[address+1]) << 8;
    return v;
}

void VM::Machine::push(uint16_t v)
{
    m_sp++;
    m_stack[m_sp] = v;
}

uint16_t VM::Machine::pop()
{
    return m_stack[m_sp--];
}

void VM::Machine::execInstruction()
{
    uint8_t  imm8;
    uint16_t imm16;
    auto opcode = static_cast<Instruction>(m_code[m_ip++]);

    switch(opcode)
    {
    case Instruction::LIT:
        push(getWord(m_ip));
        m_ip+=2;
        break;
    case Instruction::RES:
        imm8 = m_code[m_ip++];
        m_sp += imm8;
        break;
    case Instruction::LOAD:
        imm8 = m_code[m_ip++];
        push(m_stack[m_bp + imm8]);
        break;
    case Instruction::STORE:
        imm8 = m_code[m_ip++];
        m_stack[m_bp + imm8] = pop();
        break;        
    case Instruction::DEC:
        m_stack[m_sp]--;
        break;
    case Instruction::JNZ:
        imm16 = getWord(m_ip);
        m_ip += 2;
        if (m_stack[m_sp] != 0)
        {
            m_ip = imm16;
        }
        break;
    case Instruction::POP:
        pop();
        break;
    case Instruction::POPN:
        imm8 = m_code[m_ip++];
        m_sp -= imm8;
        break;
    case Instruction::DUP:
        push(m_stack[m_sp]);
        break;
    case Instruction::WAIT:
        // not implemented on PC
        imm16 = getWord(m_ip);
        m_ip+=2;
        break;
    case Instruction::SETOUTPUT:
        // not implemented on PC
        imm8 = m_code[m_ip++];    // pin number
        break;
    case Instruction::SETINPUT:
        // not implemented on PC
        imm8 = m_code[m_ip++];    // pin number
        break;
    case Instruction::SETBIT:
        // not implemented on PC
        imm8 = m_code[m_ip++];    // bit number
        imm8 = m_code[m_ip++];    // pin number
        break;
    case Instruction::CLRBIT:
        // not implemented on PC
        imm8 = m_code[m_ip++];    // bit number
        imm8 = m_code[m_ip++];    // pin number
        break;
    case Instruction::CALL:
        imm16 = getWord(m_ip);  // call address
        m_ip+=2;
        push(m_bp);
        push(m_ip);             // push return address
        m_bp = m_sp;
        m_ip = imm16;
        break;
    case Instruction::RET:        
        m_ip = pop();           // get return address
        m_bp = pop();           // get previous BP
        break;
    case Instruction::RX8:
        // not implemented on PC
        push(0);
        break;
    case Instruction::RX16:
        // not implemented on PC
        push(0);
        break;
    case Instruction::TX8:
        // not implemented on PC
        pop();
        break;
    case Instruction::TX16:
        // not implemented on PC
        pop();
        break;        
    }
}
