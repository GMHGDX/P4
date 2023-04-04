#include <stdio.h>
#include <unistd.h> //for pid_t and exec

#define BILLION 1000000000L
#define PERMS 0644

struct PCB {
int occupied;             // either true or false
pid_t pid;                // process id of this child
int sim_pid;              // simulated/created process ID in oss
int processNum;           //which process is currently running
double total_CPU_time;    //Time quantum used by process
double total_system_time; //Total time from start to finish
};
struct PCB processTable[20];