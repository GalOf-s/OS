#ifndef UTHREADS_THREADMANAGER_H
#define UTHREADS_THREADMANAGER_H

#include <vector>
#include <string>
#include "Thread.h"

#define SUCCESS 0
#define FAILURE -1
#define MAIN_THREAD_ID 0

#define SYSTEM_ERROR "system error: "
#define MEMORY_ALLOCATION_ERROR "failed to allocate memory."


class ThreadManager
{
public:

    /**
     * Initialize.
     *
     * @param maxThreadsNum Maximum number of threads' that can be created.
     */
    static void ThreadManager_init(int maxThreadsNum);

    /**
     * Gets a thread by according to its given id.
     *
     * @param id Thread's id.
     * @return Thread
     */
    static Thread *getThreadById(int id);

    /**
     * Adds a new thread according to an entry point.
     *
     * @param entry_point A entry point of the new Thread.
     * @return The id of the new Thread that was created, and -1 in case of a failure.
     */
	static int addNewThread(thread_entry_point entry_point);

    /**
     * Validates thread's id.
     *
     * @param id Id to validate.
     * @return SUCCESS if the given Id is valid, and FAILURE otherwise.
     */
	static int validateThreadId(int id);

    /**
     * Blocks a thread according to a given id.
     *
     * @param id thread's id to block.
     */
	static void blockThread(int id);

    /**
     * Resume a thread according to a given id.
     *
     * @param id thread's id to resume.
     */
	static void resumeThread(int id);

    /**
     * Puts a thread to sleep by a given id to the amount of quantum's given.
     * @param id Thread's id to put to sleep.
     * @param numQuantums the amount of quantum's to put the tread to sleep.
     */
    static void sleepThread(int id, int numQuantums);

    /**
    * Terminates a thread according to a given id.
    * @param id thread's id to block.
    */
    static void terminateThread(int id);

    /**
    * Destroctor.
    */
    static void ThreadManager_destruct();

private:
	static std::vector<Thread*> s_threads;
	static int s_minFreeId;
	static int s_maxThreadsNum;

    /**
     * Generates new minimal thread id available.
     *
     * @return New Id, or -1 in case of a failure.
     */
    static int _generateNewThreadId();

    /**
     * Deletes thread in threads' vector according to an thread's id.
     *
     * @param id Thread's Id.
     */
    static void _deleteThread(int id);

    /**
     * Prints a system error message to stderr.
     * @param string String to print.
     */
    static void _systemError(const std::string& string);

};


#endif //UTHREADS_THREADMANAGER_H
