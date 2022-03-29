//
// Created by gofer on 3/18/2022.
//
#include <cstdio>
#define OSM_NULLSYSCALL asm volatile( "int $0x80 " : : \
        "a" (0xffffffff) /* no such syscall */, "b" (0), "c" (0), "d" (0) /*:\
        "eax", "ebx", "ecx", "edx"*/)

int main(){
	OSM_NULLSYSCALL;
	OSM_NULLSYSCALL;
	printf("hello world!");
	return 0;
}