#pragma once
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

};
