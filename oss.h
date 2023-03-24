#include <stdio.h>
#include <unistd.h> //for pid_t and exec

#define BILLION 1000000000L
#define PERMS 0644

struct PCB {
int occupied; // either true or false
pid_t pid; // process id of this child
int sim_pid;
int processNum;
double total_CPU_time;
double total_system_time;
};
struct PCB processTable[20];