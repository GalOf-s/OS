#ifndef UTHREADS_THREAD_H
#define UTHREADS_THREAD_H
#include "uthreads.h"
#include <setjmp.h>
#include <signal.h>



enum State{
    RUNNING,
    READY,
    BLOCKED
	//TERMINATED
};

class Thread
{
public:
    sigjmp_buf env;

    explicit Thread(int id, thread_entry_point entryPoint);

    explicit Thread();

    void setState(State newState);

	State getState(){return _state;};

    void incQuantumCounter();

	int getId() const{return _id;};

	int getQuantumsCount() const{return _quantumCounter;};

    ~Thread();

private:
    int _id{};
    State _state{};
    char *_stack{};
	int _quantumCounter{};
};


#endif //UTHREADS_THREAD_H
