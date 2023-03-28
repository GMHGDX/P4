// Name: Gabrielle Hieken
// Class: 4760 Operating Systems
// Date: 3/19/2023

#include <stdio.h>
#include <getopt.h> //Needed for optarg function
#include <stdlib.h> //EXIT_FAILURE
#include <time.h> //to create system time
#include "oss.h" //for PCB/Table
#include <stdbool.h> //for booleans

#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>


#define BILLION 1000000000L //for nanoseconds

#define PERMS 0644

//message queue struct
typedef struct msgbuffer {
    long mtype;
    char strData[100];
    int intData;
} msgbuffer;

//blocked and ready queue struct
struct queue{
    int processNum;
    int position;
};
struct queue* ready_queue;
struct queue* blocked_queue;


struct queue getItem(struct queue* my_queue){
    int i;
    int lowest_position = 999;
    int lowest_position_num = -1;
    for(i=0;i<=20;i++){
        if(my_queue[i].position < lowest_position){
            lowest_position = my_queue[i].position;
            lowest_position_num = i;
        }
    }
    struct queue return_block = my_queue[lowest_position_num];
    my_queue[lowest_position_num].position = -1;
    my_queue[lowest_position_num].processNum = -1;

    return return_block;
}

struct queue* setItem(struct queue* my_queue, int processNum){
    int i;
    int highest_position = -1;
    int highest_position_num = -1;
    for(i=0;i<=20;i++){
        if(my_queue[i].position > highest_position){
            highest_position = my_queue[i].position;
            highest_position_num = i;
            printf("found a higher position %i, with num %i \n", highest_position, highest_position_num);
        }
    }
    if(highest_position == -1){
        highest_position_num = 0;
    }
    struct queue set_block;
    set_block.position = highest_position+1;
    set_block.processNum = processNum;


    bool worked = false;
    for(i=0;i<=20;i++){
        if(my_queue[i].processNum == -1){
            my_queue[i] = set_block;
            worked = true;
            break;
        }
    }

    if(!worked){
		printf("\n ERROR, all positions in this queue are full!!!\n");
    }
}


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

    //Initialize our clock
    int clock_sec = 0;
    int clock_nano = 0;


    struct timespec start, stop, start_prog;
    double sec;
    double nano;

    //For ready and blocked queue
    int queue;

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
    for(j = 0; j < 20; j++){
        processTable[j].total_CPU_time = (double)0;
        processTable[j].total_system_time = (double)0;
        processTable[j].sim_pid = -1;
        processTable[j].processNum = -1;
        processTable[j].pid = -1;
        processTable[j].occupied = 0;
    }

    //initialize blocked and ready queue
    ready_queue = (struct queue *) malloc(sizeof(struct queue) * 20);
    blocked_queue = (struct queue *) malloc(sizeof(struct queue) * 20);
    for(j = 0; j < 20; j++){
        ready_queue[j].processNum = -1;
        ready_queue[j].position = -1;
        blocked_queue[j].processNum = -1;
        blocked_queue[j].position = -1;
    }

    for(j = 0; j < 20; j++){
        printf("In ready queue # %i, is positoin %i, processnum %i \n", j, ready_queue[j].position, ready_queue[j].processNum);
    }
    struct queue grabber = getItem(ready_queue);
    printf("highest priority is stored %i, with processnum %i \n", grabber.position, grabber.processNum);


    printf("Settng first item\n");
    setItem(ready_queue, 69);

    for(j = 0; j < 20; j++){
        printf("In ready queue # %i, is positoin %i, processnum %i \n", j, ready_queue[j].position, ready_queue[j].processNum);
    }
    printf("\n\n");
    grabber = getItem(ready_queue);
    printf("highest priority after putting in 69 is %i, with processnum %i\n", grabber.position, grabber.processNum);

    setItem(ready_queue, 69);
    setItem(ready_queue, 70);
    grabber = getItem(ready_queue);
    printf("highest priority after putting in 69 and 70 is %i, with processnum %i\n", grabber.position, grabber.processNum);



    return 0;



    //Create shared memory, key
    const int sh_key = 3147550;

    //Create key using ftok() for more uniqueness
    key_t msqkey;
    if((msqkey = ftok("oss.h", 'a')) == (key_t) -1){
        perror("IPC error: ftok");
        exit(1);
    }

    //open an existing message queue or create a new one
    int msqid;
    if ((msqid = msgget(msqkey, PERMS | IPC_CREAT)) == -1) {
      perror("Failed to create new private message queue");
      exit(1);
   }

    //create shared memory
    int shm_id = shmget(sh_key, sizeof(struct PCB), IPC_CREAT | 0666);
    if(shm_id <= 0) {
        fprintf(stderr,"ERROR: Failed to get shared memory, shared memory id = %i\n", shm_id);
        exit(1);
    }

    //attatch memory we allocated to our process and point pointer to it 
    struct PCB *shm_ptr = (struct PCB*) (shmat(shm_id, NULL, 0));
    if (shm_ptr <= 0) {
        fprintf(stderr,"Shared memory attach failed\n");
        exit(1);
    }

    //Open the log file before input begins 
    fileLogging = fopen(logFile, "w+");
    
    //beginning of sending and recieving messages 
    msgbuffer buf0;
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
    pid_t child[15];

    int k;
    for(k = 0; k < 15; k++){
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

    printf("random number - seconds: %i", newProcsSec);
    printf("\trandom number - Nano: %i\n", newProcsNS);
    
    //to break when program has reached 3 real seconds
    bool past3s = false;

    while(1) {// store pids of our first two children to launch

    //ALL OUTPUT
    //OSS: Generating process with PID 3 and putting it in queue 0 at time 0:5000015
    //OSS: Dispatching process with PID 3 from queue 0 at time 0:5000805,
    //OSS: total time this dispatch was 790 nanoseconds,
    //OSS: Receiving that process with PID 3 ran for 270000 nanoseconds,
    //OSS: **WHAT DID IT CHOOSE IN WORKER**(not using its entire time quantum, used it's entire time quamtum, terminatedetc.)
    //OSS: **WHAT QUEUE DOES IT GO IN AFTER CHOOSING**(Putting process with PID 3 into blocked queue 'OR' Putting process with PID 3 into ready queue)

        if(clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        return EXIT_FAILURE;
        } 

        sec = (stop.tv_sec - start.tv_sec); 
        nano = (double)( stop.tv_nsec - start.tv_nsec);

        printf("The stop time: %i", sec);
        printf("\tThe stop time: %i\n", nano);

        if(stop.tv_sec - start_prog.tv_sec >= 3){
            past3s = true;
        }

        //check if enough time has passed to create a new process


        //todo: find process in ready queue, tell child the quantum, remove process from ready queue, check if process table is full, check if clock isnt passed time
        //if (a process is in the ready queue){
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
        //}


        // Then let me read a message, but only one meant for me
        // ie: the one the child just is sending back to me
        //Check if child goes back to ready, blocked or if terminated
        if (msgrcv(msqid, &rcvbuf,sizeof(msgbuffer), getpid(),0) == -1) {
            perror("failed to receive message in parent\n");
            exit(1);
        }

        printf("OSS: Dispatching process with PID %d from queue %i at time %i:%ld,", child[childNum], queue, sec, nano);
        printf("OSS: total time this dispatch was %ld nanoseconds", nano);

        printf("Parent %d received message: %s my int data was %d\n",getpid(),rcvbuf.strData,rcvbuf.intData);

        


        //if(all process table is compelted, and requdy quee and blocked queue is empty){
            //break out of loop, end program
        //}


        childNum++;
        printf("sleeping for a sec\n\n\n\n");
        sleep(maxSec);
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