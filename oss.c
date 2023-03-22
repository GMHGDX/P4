// Name: Gabrielle Hieken
// Class: 4760 Operating Systems
// Date: 3/19/2023

#include <stdio.h>
#include <getopt.h> //Needed for optarg function
#include <stdlib.h> //EXIT_FAILURE


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

int main(int argc, char *argv[]){

    //My constant time quantum
    //int q = 15200;

    //max random time user processes will be created between each process
    int maxSec = 1;
    //int maxNano;

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

    //Open the log file before input begins 
    fileLogging = fopen(logFile, "w+");
    
    //beginning of sending and recieving messages 
    msgbuffer buf0, buf1;
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
    while(1) {// store pids of our first two children to launch
        
        pid_t child[childNum];
        
        int i = 0;
        // create our child
        for (i = 0; i < 1; i++) {
            // lets fork off a child
            pid_t pid = fork();

            if (pid > 0) {
                // save this child's pid
                child[i] = pid;
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
        }

        // lets send a message only to child1, not child0
        buf0.mtype = child[childNum];
        buf0.intData = child[childNum]; // we will give it the pid we are sending to, so we know it received it
        
        // char messageToChild[7];
        // int j;
        // for(j = 0; j < 10; j++){
        //     snprintf(buf, childNum, "buf%d", j);
        // }

        if (childNum == 0){
        strcpy(buf0.strData,"Message to child 0\n");
        }
        if (childNum == 1){
        strcpy(buf0.strData,"Message to child 1\n");
        }
        if (childNum == 1){
        strcpy(buf0.strData,"Message to child 2\n");
        }

        //send message to worker process
        if (msgsnd(msqid, &buf1, sizeof(msgbuffer)-sizeof(long), 0) == -1) {
            perror("msgsnd to child 1 failed\n");
            exit(1);
        }

        msgbuffer rcvbuf;

        // Then let me read a message, but only one meant for me
        // ie: the one the child just is sending back to me
        if (msgrcv(msqid, &rcvbuf,sizeof(msgbuffer), getpid(),0) == -1) {
            perror("failed to receive message in parent\n");
            exit(1);
        }

        printf("Parent %d received message: %s my int data was %d\n",getpid(),rcvbuf.strData,rcvbuf.intData);

        // wait for children to end
        for (i = 0; i < 2; i++) {
            wait(0);
        }
        
        childNum++;
        printf("sleeping for a sec\n\n\n\n");
        sleep(maxSec);
        
        if(childNum == 3){
            printf("3 processes done, exiting loop");
            break;
        }
    }

    // get rid of message queue
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl to get rid of queue in parent failed");
        exit(1);
    }

    //close the log file
    fclose(fileLogging);

    return 0;
}