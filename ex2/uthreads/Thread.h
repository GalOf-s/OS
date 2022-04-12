#ifndef UTHREADS_THREAD_H
#define UTHREADS_THREAD_H
#include "uthreads.h"
#include <setjmp.h>
#include <signal.h>



enum State{
    RUNNING,
    READY,
    BLOCKED
};

class Thread
{
public:
    sigjmp_buf env;

    /**
     * Constructor.
     *
     * @param id Thread's id.
     * @param entryPoint Thread's entry point.
     */
    explicit Thread(int id, thread_entry_point entryPoint);

    /**
     * Main thread constructor.
     */
    explicit Thread();

    /**
     * Setter for state.
     *
     * @param newState A new state
     */
    void setState(State newState);

    /**
     * Getter for state.
     *
     * @return The state od the thread.
     */
	State getState(){return _state;};

    /**
     * increments the quantum's counter by one.
     */
    void incQuantumCounter();

    /**
     * Getter for Id.
     *
     * @return The Id.
     */
	int getId() const{return _id;};

    /**
     * Getter for the quantum counter.
     *
     * @return quantum counter.
     */
	int getQuantumsCount() const{return _quantumCounter;};

    /**
     * Destructor.
     */
    ~Thread();

private:
    int _id{};
    State _state{};
    char *_stack{};
	int _quantumCounter{};
};


#endif //UTHREADS_THREAD_H
