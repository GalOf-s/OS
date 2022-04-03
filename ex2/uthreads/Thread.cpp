#include "Thread.h"
#include "Scheduler.h"


sigjmp_buf env[2];

Thread::Thread() {
    _id = 0;
    _state = RUNNING;
    _quantumCounter = 1;

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

int Thread::block()
{
    // TODO what happens if i block a thread that is sleeping , what happen to his quantum, can somthing like this happens?

	if(_state == RUNNING){

		Scheduler::switchThread(SIGUSR1);

	}
	_state = BLOCKED; // TODO: remove from queue
    //Scheduler::removeThreadFromReady(_id);
	return 0;
}

int Thread::resume()
{
	if (_state == BLOCKED){
		_state = READY;
	}
	return 0;
}

Thread::~Thread()
{
	delete[] _stack;
}

int Thread::sleep() {
    return 0;

}
