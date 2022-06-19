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
#define UNMOUNT_ERROR_MSG "error on unmount"
#define REMOVE_ERROR_MSG "error on remove"
#define MKDIR_ERROR_MSG "error on mkdir"
#define RMDIR_ERROR_MSG "error on rmdir"

struct ChildArgs {
    int argc;
    char** argv;
};

/**
 * Prints system error to stderr.
 */
void systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR_MSG + string << std::endl;
    exit(EXIT_FAILURE);
}

/**
 * Set up the cgroup files.
 */
void set_cgroup(char* numProcesses){
    if (mkdir(FS, PERMISSIONS) < 0) {
        systemError(MKDIR_ERROR_MSG);
    }

    if (mkdir(CGROUPS, PERMISSIONS) < 0) {
        systemError(MKDIR_ERROR_MSG);
    }

    if (mkdir( CGROUPS_PIDS, PERMISSIONS) < 0) {
        systemError(MKDIR_ERROR_MSG);
    }


    std::ofstream procs_file;
    procs_file.open (CGROUPS_PIDS CGROUPS_PROCS);
    procs_file << CHILD_ID<<std::endl;
    procs_file.close();

    std::ofstream max_pids_file;
    max_pids_file.open (CGROUPS_PIDS CGROUPS_MAX);
    max_pids_file << numProcesses << std::endl;
    max_pids_file.close();

    std::ofstream notify_file;
    notify_file.open (CGROUPS_PIDS CGROUPS_NOTIFY);
    notify_file << CHILD_ID<<std::endl;
    notify_file.close();
}

/**
 * Copies string path.
 */
void concat_paths(const char *first_path, const char *second_path, char* &result){
	result = (char*) calloc(strlen(first_path) + strlen(second_path) + 1, sizeof(char));
	strcpy(result,first_path); // copy string one into the result.
	strcat(result,second_path); // append string two to the result.
}

/**
 * Clean the cgroup files.
 */
void clean_up_cgroup(char* container_root_path){
	char* result;
	concat_paths(CGROUPS_PIDS, CGROUPS_PROCS, result);
	concat_paths(container_root_path, result, result);
    if(remove(result) < 0) {
        systemError(REMOVE_ERROR_MSG);
    }

	concat_paths(CGROUPS_PIDS, CGROUPS_MAX, result);
	concat_paths(container_root_path, result, result);
    if(remove(result) < 0) {
        systemError(REMOVE_ERROR_MSG);
    }

	concat_paths(CGROUPS_PIDS, CGROUPS_NOTIFY, result);
	concat_paths(container_root_path, result, result);
    if(remove(result) < 0) {
        systemError(REMOVE_ERROR_MSG);
    }

	concat_paths(container_root_path, CGROUPS_PIDS, result);
    if(rmdir(result) < 0) {
        systemError(RMDIR_ERROR_MSG);
    }

	concat_paths(container_root_path, CGROUPS, result);
    if(rmdir(result) < 0) {
        systemError(RMDIR_ERROR_MSG);
    }

    concat_paths(container_root_path, FS, result);
    if(rmdir(result) < 0) {
        systemError(RMDIR_ERROR_MSG);
    }
}

/**
 * The main function of the container to be created.
 * Sets all flags of the child.
 */
int child(void *args) {
    auto *childArgs = (ChildArgs *) args;
    char* newHostName = childArgs->argv[1];
    char* newFilesystemDirectory = childArgs->argv[2];
    char* numProcesses = childArgs->argv[3];
    char* pathToProgramToRun = childArgs->argv[4];

    if (sethostname(newHostName, strlen(newHostName)) < 0) { // set new host name
        systemError(SETHOSTNAME_ERROR_MSG);
    }

    if (chroot(newFilesystemDirectory) < 0) { // set new filesystem directory
        systemError(CHROOT_ERROR_MSG);
    }

    set_cgroup(numProcesses);

    if (chdir("/") < 0) { // change working directory to newly set filesystem directory
        systemError(CHDIR_ERROR_MSG);
    }

    if (mount("proc", "/proc", "proc", 0, 0) < 0) { // new process will be same inside proc
        systemError(MOUNT_ERROR_MSG);
    }

    char** argsForProgram = new char*[childArgs->argc - 5];
    argsForProgram[0] = pathToProgramToRun;
    int childIndex = 1;
    for (int i = 5; i < childArgs->argc; i++) {
        argsForProgram[childIndex] = childArgs->argv[i];
        childIndex++;
    }
    argsForProgram[childIndex] = (char *) 0;

    if(execvp(pathToProgramToRun, argsForProgram) < 0) {
        systemError(EXECVP_ERROR_MSG);
    }

    return 0;
}

/**
 * Runs a container program.
 */
int main(int argc, char* argv[]) {
    char* stack = new (std::nothrow) char [STACK]; // child's stack
    if(stack == nullptr) {
        systemError(MEMORY_ALLOCATION_ERROR_MSG);
    }

	ChildArgs childArgs = {argc, argv};
    if(clone(child,
                          stack + STACK,
                          CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,&childArgs) < 0) {
        systemError(CLONE_ERROR_MSG);
    }

    if(wait(nullptr) < 0) {
        systemError(WAIT_ERROR_MSG);
    }

    clean_up_cgroup(argv[2]);
    if(umount("proc") < 0){
        systemError(UNMOUNT_ERROR_MSG);
    }
}

