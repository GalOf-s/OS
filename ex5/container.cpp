#include<cstdio>
#include<sched.h>
#include<unistd.h>
#include <csignal>
#include <cstring>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

#define STACK 8192

#define PERMISSIONS 0755



#define CGROUPS_PIDS "/sys/fs/cgroups/pids"
#define CGROUPS_PROCS "/cgroup.procs"
#define CGROUPS_MAX "/pids.max"
#define CGROUPS_NOTIFY "/notify_on_release"
#define CGROUPS "/sys/fs/cgroups"
#define FS "/sys/fs"
#define CHILD_ID "1"

#define SYSTEM_ERROR_MSG "system error: "
#define MEMORY_ALLOCATION_ERROR_MSG "failed to allocate memory."
#define CLONE_ERROR_MSG "error on clone"
#define MOUNT_ERROR_MSG "error on mount"
#define SETHOSTNAME_ERROR_MSG "error on sethostname"
#define CHROOT_ERROR_MSG "error on chroot"
#define CHDIR_ERROR_MSG "error on chdir"
#define EXECVP_ERROR_MSG "error on execvp"
#define WAIT_ERROR_MSG "error on wait"

struct ChildArgs {
    int argc;
    char** argv;
};


/**
 * prints system error to stderr
 */
void systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR_MSG + string << std::endl;
    exit(EXIT_FAILURE);
}


void set_cgroup(char* numProcesses){
//    int i= mkdir(CGROUPS_PIDS, PERMISSIONS);
//    ENOTDIR
    int i= mkdir("/sys/fs", PERMISSIONS);
    int j= mkdir("/sys/fs/cgroups", PERMISSIONS);
    int m= mkdir( "/sys/fs/cgroups/pids", PERMISSIONS);

//    std::cerr << i << std::endl;
//    std::cerr << j << std::endl;
    std::cerr << m << std::endl;
    std::ofstream procs_file;

    procs_file.open (CGROUPS_PIDS CGROUPS_PROCS);
    procs_file << CHILD_ID;
    procs_file.close();

    std::ofstream max_pids_file;
    max_pids_file.open (CGROUPS_PIDS CGROUPS_MAX);
    max_pids_file << numProcesses;
    max_pids_file.close();

    std::ofstream notify_file;
    notify_file.open (CGROUPS_PIDS CGROUPS_NOTIFY);
    notify_file << CHILD_ID;
    notify_file.close();
}

void clean_up_cgroup(){
    std::string cgroups_path = std::string(CGROUPS_PIDS);
    remove(CGROUPS_PIDS CGROUPS_PROCS);
    remove(CGROUPS_PIDS CGROUPS_MAX);
    remove(CGROUPS_PIDS CGROUPS_NOTIFY);
    rmdir(CGROUPS_PIDS);
    rmdir(CGROUPS);
    rmdir(FS);
}

void child(void *args) {
    auto *childArgs = (ChildArgs *) args;
    char* newHostName = childArgs->argv[1];
    char* newFilesystemDirectory = childArgs->argv[2];
    char* numProcesses = childArgs->argv[3];
    char* pathToProgramToRun = childArgs->argv[4];

//    std::cerr << newHostName << std::endl;
//    std::cerr << newFilesystemDirectory << std::endl;
//    std::cerr << numProcesses << std::endl;
//    std::cerr << pathToProgramToRun << std::endl;
//    std::cerr << argsForProgram << std::endl;

    if (sethostname(newHostName, strlen(newHostName)) < 0) { // set new host name
        systemError(SETHOSTNAME_ERROR_MSG);
    }

    if (chroot(newFilesystemDirectory) < 0) { // set new filesystem directory
        std::cerr << errno << std::endl;
        systemError(CHROOT_ERROR_MSG);
    }

    set_cgroup(numProcesses); // TODO errors on opening files?


    if (chdir("/") < 0) { // change working directory to newly set filesystem directory
        std::cerr << errno << std::endl;
        systemError(CHDIR_ERROR_MSG);
    }


    if (mount("proc", "/proc", "proc", 0, 0) < 0) { // new process will be same inside proc
        systemError(MOUNT_ERROR_MSG);
    }

    char** argsForProgram = new char*[childArgs->argc - 5];
    argsForProgram[0] = childArgs->argv[4];
    int childIndex = 1;
    for (int i = 5; i < childArgs->argc; i++) {
        argsForProgram[childIndex] = childArgs->argv[i];
        childIndex++;
    }
    argsForProgram[childIndex] = nullptr;

    if(execvp(pathToProgramToRun, argsForProgram) < 0) {
        std::cerr << errno << std::endl;
        systemError(EXECVP_ERROR_MSG);
    }
}

int main(int argc, char* argv[]) {
    char* stack = new (std::nothrow) char [STACK]; // child's stack
    if(stack == nullptr) {
        systemError(MEMORY_ALLOCATION_ERROR_MSG);
    }

    char** argsChild = new char*[argc - 5];
    argsChild[0] = argv[4];
    int childIndex = 1;
    for (int i = 5; i < argc; i++) {
        argsChild[childIndex] = argv[i];
        childIndex++;
    }
    argsChild[childIndex] = nullptr;
    char* args[] = { argv[1], argv[2], argv[3], argv[4],  };

    ChildArgs x{ argc, argv};
    if(clone(reinterpret_cast<int (*)(void *)>(child), // TODO check if reinterpret_cast ok
                          stack + STACK,
                          CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,&x) < 0) {
        std::cerr << errno << std::endl;

        systemError(CLONE_ERROR_MSG);
    }

    if(wait(nullptr) < 0) {
        systemError(WAIT_ERROR_MSG);
    }

    clean_up_cgroup();
    umount("proc"); // TODO check if ok + if need error msg
}

//char*** createArgumentsArray(int argc, char* argv[]) {
//    char** argsChild = new char*[argc - 5];
//    int childIndex = 0;
//
//    for (int i = 5; i < argc; i++) {
//        argsChild[childIndex] = argv[i];
//        childIndex++;
//    }
//    char** args[] = {(char **) argv[1], (char **) argv[2], (char **) argv[3], (char **) argv[4], argsChild};
//    return args;
//}