#include "Thread.h"
#include "Scheduler.h"



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
