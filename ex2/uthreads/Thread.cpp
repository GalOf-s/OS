#include "Thread.h"
#include "Scheduler.h"

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t _translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}


#endif


Thread::Thread() {
    _id = 0;
    _quantumCounter = 1;

    sigsetjmp(env, 1);
    sigemptyset(&env->__saved_mask);
}

Thread::Thread(int id, thread_entry_point entryPoint) {
    _id = id;
    _state = READY;
    _stack = new char[STACK_SIZE];
    _quantumCounter = 0;

    address_t sp = (address_t) _stack + STACK_SIZE - sizeof(address_t);
    address_t pc = (address_t) entryPoint;
    sigsetjmp(env, 1);
    (env->__jmpbuf)[JB_SP] = _translate_address(sp);
    (env->__jmpbuf)[JB_PC] = _translate_address(pc);
    sigemptyset(&env->__saved_mask);

}

void Thread::setState(State newState) {
    _state = newState;
}

void Thread::incQuantumCounter() {
    _quantumCounter++;
}

Thread::~Thread()
{
	delete[] _stack;
}
