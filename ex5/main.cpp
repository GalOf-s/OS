#include<cstdio>
#include<sched.h>
#include<unistd.h>
#include <csignal>
#include <cstring>
#include <sys/mount.h>
#include <sys/wait.h>


#define STACK 8192


void child(char* args[]) {
    char* newHostName = args[1];
    char* newFilesystemDirectory = args[2];
    char* numProcesses = args[3];
    char* pathToProgramToRun = args[4];
    char* argsForProgram = args[5];

    sethostname(newHostName, strlen(newHostName)); // set new host name
    chroot(newFilesystemDirectory); // set new filesystem directory
    mount("proc", "/proc", "proc", 0, 0); // new process will be same inside proc
    char* argsChild[] ={pathToProgramToRun, argsForProgram + 1, (char *)0};
    execvp(pathToProgramToRun, argsChild);
}

int main(int argc, char* argv[]) {
    char* stack = new char [STACK]; // child's stack
    int child_pid = clone(reinterpret_cast<int (*)(void *)>(child), // TODO check if ok
                          stack + STACK,
                          CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, argv);
    wait(nullptr);
    umount("proc"); // TODO check if ok
}