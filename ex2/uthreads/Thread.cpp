#include "Thread.h"
#include "Scheduler.h"




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


void Thread::ThreadInitMain() {

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
		Scheduler::switchThread(SIGUSR1);
	}
	_state = BLOCKED; // TODO: remove from queue
    Scheduler::removeThreadFromReady(_id);
	return 0;
}

int Thread::resume()
{
	if (_state == BLOCKED){
		_state = READY;
        //Scheduler::addThreadToReady(_id)
	}
	return 0;
}

int Thread::terminate()
{
	if(_state == READY){
        Scheduler::removeThreadFromReady(_id); // removes thread from ready queue
	}
    //TODO free stack
	ThreadManager::deleteThread(_id);
	return 0;
}

Thread::~Thread()
{
	delete _stack;
    _stack = nullptr;
}

int Thread::sleep() {
    return 0;

}
