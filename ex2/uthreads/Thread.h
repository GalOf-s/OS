//
// Created by gofer on 3/29/2022.
//

#ifndef UTHREADS_THREAD_H
#define UTHREADS_THREAD_H
#include "uthreads.h"
#include <setjmp.h>
#include <signal.h>
#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
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


enum State{
    RUNNING,
    READY,
    BLOCKED
};

class Thread
{
public:
    sigjmp_buf env{};

    explicit Thread(thread_entry_point entryPoint);
    static void ThreadInitMain();
	~Thread();

    void setState(State newState);
	State getState(){return _state;};

    void incQuantumCounter();
	void setId(int id){_id=id;};
	int getId() const{return _id;};
	int getQuantumsCount() const{return _quantumCounter;};
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
