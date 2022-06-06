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


#define CGROUPS_PATH "/sys/fs/cgroups/pids"

void set_cgroup(char* numProcesses){
	mkdir(CGROUPS_PATH, 0755); // TODO: check permissions

	std::ofstream procs_file;
	procs_file.open (CGROUPS_PATH + "/cgroup.procs");
	procs_file << "1";
	procs_file.close();

	std::ofstream max_pids_file;
	max_pids_file.open (CGROUPS_PATH + "/pids.max");
	max_pids_file << numProcesses;
	max_pids_file.close();

	std::ofstream notify_file;
	notify_file.open (CGROUPS_PATH + "/notify_on_release");
	notify_file << "1";
	notify_file.close();
}

void clean_up_cgroup(){
	remove(CGROUPS_PATH + "/cgroup.procs");
	remove(CGROUPS_PATH + "/pids.max");
	remove(CGROUPS_PATH + "/notify_on_release");
	rmdir(CGROUPS_PATH);
	rmdir("/sys/fs/cgroups");
	rmdir("/sys/fs");
}

void child(char* args[]) {
    char* newHostName = args[1];
    char* newFilesystemDirectory = args[2];
    char* numProcesses = args[3];
    char* pathToProgramToRun = args[4];
    char* argsForProgram = args[5];

    sethostname(newHostName, strlen(newHostName)); // set new host name
    chroot(newFilesystemDirectory); // set new filesystem directory
	chdir(newFilesystemDirectory);  // change working directory to newly set filesystem directory
    mount("proc", "/proc", "proc", 0, 0); // new process will be same inside proc
	set_cgroup(numProcesses);

    char* argsChild[] ={pathToProgramToRun, argsForProgram + 1, (char *)0};
    execvp(pathToProgramToRun, argsChild);
}

int main(int argc, char* argv[]) {
    char* stack = new char [STACK]; // child's stack
    int child_pid = clone(reinterpret_cast<int (*)(void *)>(child), // TODO check if ok
                          stack + STACK,
                          CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, argv);
    wait(nullptr);
	clean_up_cgroup();
    umount("proc"); // TODO check if ok
}