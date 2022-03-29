//
// Created by gofer on 3/29/2022.
//

#ifndef UTHREADS_THREADMANAGER_H
#define UTHREADS_THREADMANAGER_H

#include <vector>
#include "Thread.h"


class ThreadManager
{
public:
	std::vector<Thread> threads;
	int minFreeId;
	Thread getThreadById(int id);
};


#endif //UTHREADS_THREADMANAGER_H
