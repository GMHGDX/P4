// Name: Gabrielle Hieken
// Class: 4760 Operating Systems
// Date: 3/19/2023

#include <stdio.h>
#include <getopt.h> //Needed for optarg function
#include <stdlib.h> //EXIT_FAILURE
#include <time.h> //to create system time


#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>

#define PERMS 0644
typedef struct msgbuffer {
    long mtype;
    char strData[100];
    int intData;
} msgbuffer;

int randomNumberGenerator(int limit){
    int sec;
    sec = (rand() % (limit)) + 1;
    return sec;
}

int main(int argc, char *argv[]){

    //My constant time quantum
    int const q = 15200;

    //max random time user processes will be created between each process
    int const maxSec = 1;
    int const maxNano = BILLION - 1;

    //INtialize our clock
    int clock_sec = 0;
    int clock_nano = 0;


    struct timespec start, stop, start_prog;
    double sec;
    double nano;

    //default logfile name
    char* logFile = "logfile";

    //for creating the logfile 
    FILE *fileLogging;

    //Parse through command line options
	char opt;
    while((opt = getopt(argc, argv, "hf:")) != -1 )
    {
        switch (opt)
        {
        //help message
        case 'h':
			printf("To run this project: \n\n");
            printf("run the command: ./oss -f 'logfile string'\n\n");
                    printf("\t-f = command line argument for logfile\n");
                    printf("\t'logfile string' = name of logfile you wish to write the output to\n\n");
                    printf("Note: If you leave out '-f' in the logfile, it will create a logfile with the default name 'logfile'\n\n");
                    printf("Have fun :)\n\n");

                    exit(0);
            break;
        //logfile command
        case 'f':
            logFile = optarg; 
            break;
        default:
            printf ("Invalid option %c \n", optopt);
            return (EXIT_FAILURE);
        }
    } 

    //initialize the process table
    int j;
    for(j=0;j<20;j++){
        processTable[j].total_CPU_time = (double)0;
        processTable[j].total_system_time = (double)0;
        processTable[j].sim_pid = -1;
        processTable[j].processNum = -1;
        processTable[j].pid = -1;
        processTable[j].occupied = 0;
    }

    //initialize blocked and ready queue
    struct queue{
        int processNum;
        int position;
    };
    struct queue ready_queue[20];
    struct queue blocked_queue[20];
    
    for(j=0;j<20;j++){
        ready_queue[j].processNum = -1;
        ready_queue[j].position = 0;
        blocked_queue[j].processNum = -1;
        blocked_queue[j].position = 0;
    }

    //Open the log file before input begins 
    fileLogging = fopen(logFile, "w+");
    
    //beginning of sending and recieving messages 
    msgbuffer buf0;
    int msqid;
    key_t key;

    system("touch oss.c");

    // get a key for our message queue
    if ((key = ftok("oss.c", 1)) == -1) {
        perror("ftok");
        exit(1);
    }

    // create our message queue
    if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) {
        perror("msgget in parent");
        exit(1);
    }

    printf("Message queue set up\n");
    int childNum = 0;
    pid_t child[5];

    int k;
    for(k=0;k<5;k++){
        child[k] = 0;
        printf("Intialized %i to %d\n", k, child[k]);
    }
    int i = 0;
    pid_t pid;
    msgbuffer rcvbuf;


    //start the program clock (used for checking if 3 real time secs have passed)
    if( clock_gettime( CLOCK_REALTIME, &start_prog) == -1 ) {
      perror( "clock gettime" );
      return EXIT_FAILURE;
    }

    double newProcsSec = randomNumberGenerator(maxSec) + start_prog.tv_sec;
    double newProcsNS = randomNumberGenerator(maxNano) + start_prog.tv_nsec;

    bool past3s = false;

    while(1) {// store pids of our first two children to launch

        if(clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        return EXIT_FAILURE;
        } 
        if(stop.tv_sec - start_prog.tv_sec >= 4){
            past3s = true;
        }

        //check if enough time has passed to create a new process


        //todo: find process in ready queue, tell child the quantum, remove process from ready queue, check if process table is full, check if clock isnt passed time
        if (a process is in the ready queue){
            pid = fork();
            if (pid > 0) {
                // save this child's pid
                child[i] = pid;
                childNum++;
            }
            else if (pid == 0) {
                // in child, so lets exec off child executable
                execlp("./worker","./worker",(char *)NULL);

                // should never get here if exec worked
                printf("Exec failed for first child\n");

                exit(1);
            }
            else {
                // fork error
                perror("fork failed in parent");
            }
            printf("created child %i with pid %d \n", childNum, child[childNum]);

            // lets send a message only to child1, not child0
            buf0.mtype = child[childNum];
            buf0.intData = child[childNum]; // we will give it the pid we are sending to, so we know it received it

            printf("Sending message to child %i with pid %d \n", childNum, child[childNum]);

            if (childNum == 0){
                strcpy(buf0.strData,"Message to child 0\n");
                //send message to worker process
                if (msgsnd(msqid, &buf0, sizeof(msgbuffer)-sizeof(long), 0) == -1) {
                    perror("msgsnd to child 1 failed\n");
                    exit(1);
                }
            }
            if (childNum == 1){
                strcpy(buf0.strData,"Message to child 1\n");
                //send message to worker process
                if (msgsnd(msqid, &buf0, sizeof(msgbuffer)-sizeof(long), 0) == -1) {
                    perror("msgsnd to child 1 failed\n");
                    exit(1);
                }
            }
            printf("finsihed sending message to child %i with pid %d \n", childNum, child[childNum]);
        }


        // Then let me read a message, but only one meant for me
        // ie: the one the child just is sending back to me
        //Check if child goes back to ready, blocked or if terminated
        if (msgrcv(msqid, &rcvbuf,sizeof(msgbuffer), getpid(),0) == -1) {
            perror("failed to receive message in parent\n");
            exit(1);
        }

        printf("Parent %d received message: %s my int data was %d\n",getpid(),rcvbuf.strData,rcvbuf.intData);

        


        if(all process table is compelted, and requdy quee and blocked queue is empty){
            //break out of loop, end program
        }


        //childNum++;
        // printf("sleeping for a sec\n\n\n\n");
        // sleep(maxSec);
        
        // if(childNum == 3){
        //     printf("3 processes done, exiting loop");
        //     break;
        // }
    }

    //wait(0); //wait for all processes to complete then exit. should check for process tbale to empty actually so make sure you reveieve messages


    // get rid of message queue
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl to get rid of queue in parent failed");
        exit(1);
    }

    //close the log file
    fclose(fileLogging);

    return 0;
}