// Name: Gabrielle Hieken
// Class: 4760 Operating Systems
// Date: 3/19/2023

#include <stdio.h>
#include <sys/types.h>
#include <getopt.h> //Needed for optarg function
#include <stdlib.h> //EXIT_FAILURE
#include <time.h> //to create system time
#include <stdbool.h> //for boolean values
#include <sys/shm.h> //Shared memory
#include <sys/msg.h> //message queues
#include <string.h> //remedy exit() warning
#include <sys/wait.h> //wait for child
#include <unistd.h> //for pid_t and exec

#include <errno.h>
#include <sys/ipc.h>

#include "oss.h" //for PCB/Table

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
    double total_CPU_time;    //Time quantum used by process
    double total_system_time; //Total time from start to finish
};
struct queue* ready_queue;
struct queue* blocked_queue;


struct queue getItem(struct queue* my_queue){
    int i;
    int lowest_position = 999;
    int lowest_position_num = -1;
    for(i=0;i<20;i++){
        if(my_queue[i].position < lowest_position && my_queue[i].position != -1){
            lowest_position = my_queue[i].position;
            lowest_position_num = i;
        }
    }

    if(lowest_position_num == -1){
        printf("ERROR: nothting is in queue\n");
        //exit(1);
    }
    struct queue return_block = my_queue[lowest_position_num];
    my_queue[lowest_position_num].position = -1;
    my_queue[lowest_position_num].processNum = -1;
    my_queue[lowest_position_num].total_CPU_time = -1;
    my_queue[lowest_position_num].total_system_time = -1;

    return return_block;
}

void setItem(struct queue* my_queue, int processNum, double total_CPU_time, double total_system_time){
    int i;
    int highest_position = -1;
    int highest_position_num = -1;
    for(i=0;i<20;i++){
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
    set_block.total_CPU_time = total_CPU_time;
    set_block.total_system_time = total_system_time;

    bool worked = false;
    for(i=0;i<20;i++){
        if(my_queue[i].processNum == -1){
            my_queue[i] = set_block;
            worked = true;
            break;
        }
    }

    if(!worked){
		printf("\n ERROR: all positions in this queue are full!!!\n");
        exit(1);
    }
}

int randomNumberGenerator(int limit){
    int sec;
    sec = (rand() % (limit)) + 1;
    return sec;
}

int main(int argc, char *argv[]){

    //My constant time quantum
    int const quantum = 15200;

    char quantumForPID[10];                                //initialize char for conversion
    snprintf(quantumForPID, sizeof(quantumForPID), "%i", quantum); //convert quantum int to string

    //max random time user processes will be created between each process
    int const maxSec = 1;
    int const maxNano = BILLION - 1;

    //Initialize our clock
    int clock_sec = 0;
    int clock_nano = 0;

    //for creating a simulated clock 
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

    // for(j = 0; j < 20; j++){
    //     printf("In ready queue # %i, is positoin %i, processnum %i \n", j, ready_queue[j].position, ready_queue[j].processNum);
    // }
    // struct queue grabber = getItem(ready_queue);
    // printf("highest priority is stored %i, with processnum %i \n", grabber.position, grabber.processNum);

    // printf("Settng first item\n");
    // setItem(ready_queue, 69);

    // for(j = 0; j < 20; j++){
    //     printf("In ready queue # %i, is positoin %i, processnum %i \n", j, ready_queue[j].position, ready_queue[j].processNum);
    // }
    // printf("\n\n");
    // grabber = getItem(ready_queue);
    // printf("highest priority after putting in 69 is %i, with processnum %i\n", grabber.position, grabber.processNum);

    // setItem(ready_queue, 69);
    // setItem(ready_queue, 70);

    // for(j = 0; j < 20; j++){
    //     printf("In ready queue # %i, is positoin %i, processnum %i \n", j, ready_queue[j].position, ready_queue[j].processNum);
    // }
    // printf("\n\n");
    // grabber = getItem(ready_queue);
    // printf("highest priority after putting in 69 and 70 is %i, with processnum %i\n", grabber.position, grabber.processNum);

    // return 0;

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
    msgbuffer buf;
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

    //start the program clock (used for checking if 3 real time seconds have passed and CPU time)
    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      perror( "clock gettime" );
      return EXIT_FAILURE;
    }

    double newProcsSec = randomNumberGenerator(maxSec) + start.tv_sec;
    double newProcsNS = randomNumberGenerator(maxNano) + start.tv_nsec;

    printf("random number - seconds: %i", newProcsSec);
    printf("\trandom number - Nano: %i\n", BILLION);
    
    //initialization of each value for use in while loop
    bool past3s = false; //for tracking three seconds
    int procNum = 1; //tracking process number
    int currentP;  //tracking proceess taken out of queue
    int simPID = 10000; //creating simulated PID and incrementing by one each loop
    int childrenToLaunch = 0;

    int childNum = 0; 
    pid_t child[100]; //create array for child number to send messages to respective child

    int i = 0;
    pid_t pid;
    msgbuffer rcvbuf;

    struct queue queueGrabber; //for grabbing the struct out of getItem

    while(1) {

    //ALL OUTPUT
    //OSS: Generating process with PID 3 and putting it in queue 0 at time 0:5000015
    //OSS: Dispatching process with PID 3 from queue 0 at time 0:5000805,
    //OSS: total time this dispatch was 790 nanoseconds,
    //OSS: Receiving that process with PID 3 ran for 270000 nanoseconds,
    //OSS: **WHAT DID IT CHOOSE IN WORKER**(not using its entire time quantum, used it's entire time quamtum, terminatedetc.)
    //OSS: **WHAT QUEUE DOES IT GO IN AFTER CHOOSING**(Putting process with PID 3 into blocked queue 'OR' Putting process with PID 3 into ready queue)



        

        // printf("The stop time: %i", sec);
        // printf("\tThe stop time: %i\n", nano);

        // if(stop.tv_sec - start_prog.tv_sec >= 3){
        //     past3s = true;
        // }
       
        //puts first process in ready queue
        setItem(ready_queue, procNum, 0, 0);

        //if the process number in the queue is -1, then there are no processes in the queue
        if(ready_queue[0].processNum == -1){
            printf("No items in the ready queue");
        }

        //take process out of the ready queue that has been in there the longest 
        queueGrabber = getItem(ready_queue);
        currentP = queueGrabber.processNum;

        //create child if there is a process in the ready queue
        if(currentP > -1){
            pid = fork();
            if (pid > 0) {
                // save this child's pid for sending message
                child[childNum] = pid;
            }
            else if (pid == 0){
                // in child, so lets exec off child executable
                execlp("./worker","./worker",(char *)NULL);

                // should never get here if exec worked
                printf("Exec failed for first child\n");
                exit(1); //quit program if it reaches error
            }
            else {
                // fork error
                perror("fork failed in parent");
            }
            printf("created child %i with pid %d \n", childNum, child[childNum]);
        
            // lets send a message only to specific child
            buf.mtype = child[childNum];
            buf.intData = child[childNum]; // we will give it the pid we are sending to, so we know it received it

            printf("Sending message to child %i with pid %d \n", childNum, child[childNum]);

            //message contains constant time quantum initialized in oss
            strcpy(buf.strData, quantumForPID);

            //send message to worker process
            if (msgsnd(msqid, &buf, sizeof(msgbuffer)-sizeof(long), 0) == -1) {
                perror("msgsnd to child 1 failed\n");
                exit(1);
            }

            printf("finsihed sending message to child %i with pid %d \n", childNum, child[childNum]);
        }

        //recieve message back from child in worker, decide where it goes in the queue
        if (msgrcv(msqid, &rcvbuf,sizeof(msgbuffer), getpid(),0) == -1) {
            perror("failed to receive message in parent\n");
            exit(1);
        }
        printf("Parent %d received message: %s my int data was %d\n",getpid(),rcvbuf.strData,rcvbuf.intData);

        int recievedFromWorker = atoi(rcvbuf.strData); //converts message string from worker to an integer

        //used all time, put in ready queue
        if(recievedFromWorker == quantum){
            printf("used all time, put in ready queue!\n");
            //puts current process back in ready queue
            setItem(ready_queue, procNum, 0, 0);

            for(j = 0; j < 20; j++){
            printf("In ready queue # %i, is positoin %i, processnum %i \n", j, ready_queue[j].position, ready_queue[j].processNum);
            }
            printf("\n\n");
        }
        //used up part of the time, blocked queue
        else if(recievedFromWorker < quantum && recievedFromWorker > 0){
            printf("used up part of the time, put in blocked queue!\n");

            //puts current process in blocked queue
            setItem(blocked_queue, procNum, 0, 0);

            for(j = 0; j < 20; j++){
                printf("In blocked queue # %i, is positoin %i, processnum %i \n", j, blocked_queue[j].position, blocked_queue[j].processNum);
            }
            printf("\n\n");
        }
        else if(recievedFromWorker < 0){
            //terminate
            printf("terminating!\n"); //process stays removed from ready queue

            if(clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
                perror( "clock gettime" );
                return EXIT_FAILURE;
            } 
            sec = (stop.tv_sec - start.tv_sec); 
            nano = (double)( stop.tv_nsec - start.tv_nsec);

            printf("The stop time: %i", sec);
            printf("\tThe stop time: %i\n", nano);
            
            
            processTable[childrenToLaunch].total_system_time = 0;
        }
        else{
            //This should never print, but just in case
            printf("ERROR: Worker returned a number that doesnt match any option. Number returned-> %i", recievedFromWorker);
        }

        //update all values in the table
        processTable[childrenToLaunch].occupied = 1;
        processTable[childrenToLaunch].pid = child[childNum];
        processTable[childrenToLaunch].sim_pid = simPID;
        processTable[childrenToLaunch].processNum = procNum;
        processTable[childrenToLaunch].total_CPU_time = 0;

        
        printTable(fileLogging);

        //if(all process table is compelted, and requdy quee and blocked queue is empty){
            //break out of loop, end program
        //}
        
        childNum++; //increment child for new message
        procNum++; //set next process (will be 2)
        simPID++; //increment simulated PID (will be 10001)
        childrenToLaunch++; //for the table

        printf("sleeping for a sec\n\n\n\n");
        sleep(maxSec);
        if(childNum == 1){
            break;
        }
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

//Print the process table
void printTable(FILE* fileLogging){
    printf("Position\tOccupied\tPID\tSimulated PID\t\tProcessNumber\t\tCPU Time\t\tSystem Time\n");
    fprintf(fileLogging, "Position\tOccupied\tPID\tSimulated PID\t\tProcessNumber\t\tCPU Time\t\tSystem Time\n");
    
    int i;
    for(i=0;i<18;i++){
        if(processTable[i].pid == 0 ){
            break;
        }
        
        printf("%i\t%i\t\t%d\t\t%d\t%i\n%i\n%f", i, processTable[i].occupied, (long)processTable[i].pid, (long)processTable[i].sim_pid, processTable[i].processNum,processTable[i].total_CPU_time, processTable[i].total_system_time);
        fprintf(fileLogging, "%i\t%i\t\t%d\t\t%d\t%i\n%i\n%f", i, processTable[i].occupied, (long)processTable[i].pid, (long)processTable[i].sim_pid, processTable[i].processNum,processTable[i].total_CPU_time, processTable[i].total_system_time);     
    }
}