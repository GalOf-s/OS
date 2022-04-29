//
// Created by dorelby on 29/04/2022.
//

#ifndef EX3_JOBCONTEXT_H
#define EX3_JOBCONTEXT_H


#include "ThreadContext.h"

#define SYSTEM_ERROR "system error: "
#define PTHREAD_CREATE_ERROR "pthread create failed."
#define MEMORY_ALLOCATION_ERROR "failed to allocate memory."


class JobContext {

public:
    JobContext(int multiThreadLevel);

private:
    int _multiThreadLevel;
    std::vector<ThreadContext> threadContexts;
    pthread_t *threads;

    static void _systemError(const std::string &string);


    void _createMultiThreadLevel();
};


#endif //EX3_JOBCONTEXT_H
