//
// Created by gofer on 3/29/2022.
//

#ifndef UTHREADS_THREAD_H
#define UTHREADS_THREAD_H
#include "uthreads.h"

typedef unsigned int address_t;

class Thread
{
public:
	Thread(thread_entry_point entryPoint, int id);
	int init();
	int terminate();
	int block();
	int resume();
	int sleep();

private:
	thread_entry_point entryPoint{};
	address_t sp{};
	address_t pc{};
	int id{};
	int state{};
	int quantums = 0;
};


#endif //UTHREADS_THREAD_H
