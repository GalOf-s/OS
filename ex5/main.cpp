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
#define FIRST_ARG_INDEX 4

#define SYSTEM_ERROR_MSG "system error: "
#define MEMORY_ALLOCATION_ERROR_MSG "failed to allocate memory."
#define CLONE_ERROR_MSG "error on clone"
#define MOUNT_ERROR_MSG "error on mount"
#define SETHOSTNAME_ERROR_MSG "error on sethostname"
#define CHROOT_ERROR_MSG "error on chroot"
#define CHDIR_ERROR_MSG "error on chdir"
#define EXECVP_ERROR_MSG "error on execvp"
#define WAIT_ERROR_MSG "error on wait"

struct clone_args {
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
    mkdir(CGROUPS_PIDS, PERMISSIONS);
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

void extract_args_for_program(char* args[], char*** args_for_program, int argc)
{
	char **subset = (char **) malloc((argc - FIRST_ARG_INDEX) * sizeof(char *));

	for (int j = FIRST_ARG_INDEX; j < argc; j++)
		subset[j-FIRST_ARG_INDEX] = args[j];

	*args_for_program = subset;
}

void child(clone_args* args) {
	char** argv = args->argv;
    char* newHostName = argv[1];
    char* newFilesystemDirectory = argv[2];
    char* numProcesses = argv[3];
    char* pathToProgramToRun = argv[4];

    if (sethostname(newHostName, strlen(newHostName)) < 0) { // set new host name
        systemError(SETHOSTNAME_ERROR_MSG);
    }

    if (chroot(newFilesystemDirectory) < 0) { // set new filesystem directory
        systemError(CHROOT_ERROR_MSG);
    }
    if (chdir("/") < 0) { // change working directory to newly set filesystem directory
        systemError(CHDIR_ERROR_MSG);
    }

    if (mount("proc", "/proc", "proc", 0, 0) < 0) { // new process will be same inside proc
        systemError(MOUNT_ERROR_MSG);
    }

    set_cgroup(numProcesses); // TODO errors on opening files?

	char*** argsForProgram = new char**;
	extract_args_for_program(argv, argsForProgram, args->argc);

//    char* argsChild[] ={pathToProgramToRun, *argsForProgram, (char *)0};
    if(execv(pathToProgramToRun, *argsForProgram) < 0) {
        std::cerr<<"errno: "<<std::strerror(errno)<<std::endl;
		systemError(EXECVP_ERROR_MSG);

    }
}

int main(int argc, char* argv[]) {
    char* stack = new (std::nothrow) char [STACK]; // child's stack
    if(stack == nullptr) {
        systemError(MEMORY_ALLOCATION_ERROR_MSG);
    }

	struct clone_args args = {argc, argv}; // here only for debugging
	char*** argsForProgram; // here only for debugging
	extract_args_for_program(args.argv, argsForProgram, args.argc); // here only for debugging
    if(clone(reinterpret_cast<int (*)(void *)>(child), // TODO check if reinterpret_cast ok
                          stack + STACK,
                          CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, &args) < 0) {
        systemError(CLONE_ERROR_MSG);
    }

    if(wait(nullptr) < 0) {
        systemError(WAIT_ERROR_MSG);

    }

    clean_up_cgroup();
    umount("proc"); // TODO check if ok + if need error msg
}