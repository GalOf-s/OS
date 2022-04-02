#include "Thread.h"



Thread::Thread(thread_entry_point entryPoint) {
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

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t Thread::_translate_address(address_t addr) {
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

void Thread::setState(State newState) {
    _state = newState;
}

void Thread::incQuantumCounter() {
    _quantumCounter++;
}

int Thread::block()
{
	if(_state == RUNNING){
		_state = BLOCKED;
		// TODO: send signal to scheduler
	}
	_state = BLOCKED; // TODO: remove from queue
	return 0;
}

