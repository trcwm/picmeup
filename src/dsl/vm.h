#pragma once
#include <array>
#include <cstdint>

namespace VM
{

/*

    VM has a data stack pointer SP and a data base pointer BP.
    The stack grows towards upper memory.

*/

enum class Instruction : uint8_t
{
    LIT     = 0,    /* push 16 bit immediate literal n16 onto the stack */
    RES,            /* reserve n8 words of stack space for local variables */
    LOAD,           /* load variable from BP offset n8 onto the stack */
    STORE,          /* store variable to BP offset n8 onto the stack */
    DEC,            /* decrement TOS */
    JNZ,            /* check TOS and jump if not zero (doesn't pop) */
    POP,            /* pop TOS */
    POPN,           /* pop n8 words off the stack */    
    DUP,            /* duplicate the TOS */
    WAIT,           /* wait n16 nanoseconds */
    SETOUTPUT,      /* set pin n8 to output */
    SETINPUT,       /* set pin n8 to input */
    SETBIT,         /* set bit n8 of pin m8 */
    CLRBIT,         /* clear bit n8 of pin m8 */
    CALL,           /* call function at address n16, sets BP to the first argument */
    RET,            /* return from function, restores BP */
    RX16,           /* receive 16 bits from UART */
    TX16,           /* transmit 16 bits to UART */
    RX8,            /* receive 8 bits from UART */
    TX8             /* transmit 8 bits to UART */
};

class Machine
{
public:

    void execInstruction();

protected:
    uint16_t getWord(const uint16_t address) const;
    void push(uint16_t v);
    uint16_t pop();

    uint16_t                  m_ip;
    uint8_t                   m_sp;
    uint8_t                   m_bp;
    std::array<uint16_t,32>   m_stack;
    std::array<uint8_t, 1024> m_code;
};

};
