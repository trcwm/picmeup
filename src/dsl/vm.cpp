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

uint16_t& VM::Machine::tos()
{
    return m_stack[m_sp];
}

void VM::Machine::execInstruction()
{
    uint8_t  imm8;
    uint8_t  bit8;
    uint16_t imm16;
    uint16_t temp16;
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
    case Instruction::INC:
        m_stack[m_sp]++;
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
        m_ip++;
        imm16 = pop();  // wait amount in ns
        break;
    case Instruction::SETOUTPUT:
        // not implemented on PC
        imm8 = m_code[m_ip++];    // pin number
        break;
    case Instruction::SETINPUT:
        // not implemented on PC
        imm8 = m_code[m_ip++];    // pin number
        break;
    case Instruction::SETPIN:
        // not implemented on PC
        imm8 = m_code[m_ip++];    // pin number
        // get TOS and set pin
        //imm8 = m_code[m_ip++];    // pin state
        pop();
        break;
    case Instruction::GETPIN:
        // not implemented on PC
        imm8 = m_code[m_ip++];    // pin number
        push(0);
        break;
    case Instruction::SETBIT:
        bit8 = m_code[m_ip++];    // get bit number
        imm8 = m_code[m_ip++];    // bit state
        if (imm8 == 0)
        {
            tos() &= ~(1 << bit8);
        }
        else
        {
            tos() |= (1 << bit8);
        }
        break;
    case Instruction::GETBIT:
        bit8 = m_code[m_ip++];    // get bit number
        tos() = (tos() >> bit8) & 0x01;
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
    case Instruction::SHR:
        m_stack[m_sp] >>= 1;
        break;        
    }
}
