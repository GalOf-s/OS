#ifndef UTHREADS_SCHEDULER_H
#define UTHREADS_SCHEDULER_H


#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <csignal>
#include "ThreadManager.h"


#define MIC_TO_SEC 1000000
#define NO_ID -1

#define SYSTEM_ERROR "system error: "
#define SIGPROCMASK_ERROR "sigprocmask failed."
#define SETITIMER_ERROR "setitimer failed."
#define SIGACTION_ERROR "sigaction fail."
#define SIGEMPTYSET_ERROR "sigemptyset failed."
#define SIGADDSET_ERROR "sigaddset failed."

class Scheduler
{

public:

    /**
     * Initialization.
     *
     * @param quantum The length (in micro second) of a quantum.
     */
    static void Scheduler_init(int quantum);

    /**
     * Gets the Id of the current running thread.
     *
     * @return Current running threads.
     */
    static int getCurrentThreadId();

    /**
     * Gets the total amount of quantum's there were sense the beginning of the program.
     *
     * @return Total quantum's.
     */
    static int getTotalQuantums();

    /**
     * Adds a new Thread Id to the ready queue.
     *
     * @param id Id to add.
     */
	static void addThreadToReadyQueue(int id);

    /**
     * Adds the pair current running id and the number of quantum's he would be sleeping to the sleeping vector.
     *
     * @param numQuantums the number of quantum's the current running thread would be sleeping.
     */
    static void addThreadToSleep(int numQuantums);

   /**
    * Switching the current running thread to the next thread ready.
    *
    * @param sig A signal to handle.
    */
	static void switchThread(int sig);

    /**
     * Blocks the signals in the timer set.
     */
    static void blockTimerSig();

    /**
     * Unblocks the signals in the timer set.
     */
    static void unblockTimerSig();

    /**
     * Removes an Id from the ready queue.
     *
     * @param id Id to remove.
     */
    static void removeThreadFromReady(int id);

    /**
     * Sets the current running thread to NO_ID.
     */
    static void setNoRunningThread();

    /**
     * Check if a thread is sleeping according to it's id.
     *
     * @param id Id to check.
     * @return True if the sleeping vector contains the given id, and false otherwise.
     */
    static bool isIdSleeping(int id);

    /**
     * Removing a thread from sleeping vector according to id.
     *
     * @param id Id to remove.
     */
    static void removeThreadFromSleeping(int id);

private:
    static int s_currentThreadId;
    static int s_totalQuantums;
    static std::vector<int> s_readyThreads;
    static std::vector<std::pair<int, int>> s_sleepingThreads;
    static struct itimerval timer;
    static sigset_t timerSet;

   /**
    * Setter for the timer.
    *
    * @param quantum The length (in micro second) of a quantum.
    */
    static void _setTimer(int quantum);

    /**
     * Gets the next ready thread in the ready queue.
     * @return The next ready thread.
     */
    static Thread* _getNextReadyThread();

    /**
     * Manages the sleeping threads.
     */
    static void _manageSleepThreads();

    /**
     * Sets the timer to start a new quantum.
     */
    static void _startTimer();

    /**
     * Prints a system error message to stderr.
     *
     * @param string String to print
     */
    static void _systemError(const std::string& string);

};


#endif //UTHREADS_SCHEDULER_H
