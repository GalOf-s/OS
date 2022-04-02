//
// Created by gofer on 3/29/2022.
//

#ifndef UTHREADS_THREAD_H
#define UTHREADS_THREAD_H
#include "uthreads.h"
#include <setjmp.h>
#include <signal.h>


typedef unsigned int address_t;

#define JB_SP 4
#define JB_PC 5

enum State{
    RUNNING,
    READY,
    BLOCKED
};
class Thread
{
public:
    sigjmp_buf env{};

    Thread(thread_entry_point entryPoint);

    void setState(State newState);
	State getState(){return _state;};

    void incQuantumCounter();
	void setId(int id){_id=id;};
	int getId(){return _id;};
	int getQuantumsCount(){return _quantumCounter;};
    int terminate();
    int block();

    int resume();

    int sleep();
private:
    int _id{};
    State _state{};
    char *_stack{};
	int _quantumCounter{};


    static address_t _translate_address(address_t addr);

};


#endif //UTHREADS_THREAD_H
