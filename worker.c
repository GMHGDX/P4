#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h> //remedy exit() warning
#include <stdlib.h> //EXIT_FAILURE
#include <sys/msg.h> //message queues
#include <unistd.h> //for pid_t and exec

#define PERMS 0644

//message queue
typedef struct msgbuffer {
    long mtype;
    char strData[100];
    int intData;
} msgbuffer;

//returns random number between 1 and limit(100) for weighted choosing period
int randomNumberGenerator(int limit){
    int sec;
    sec = (rand() % (limit)) + 1;
    return sec;
}

//THIS CAN BE DELETED AFTER TESTING
//recieve quantum
//weighted randomly decide if it will 
        //use up all time (process goes back to ready queue)                                50%     (89%)
        //use up part of the time and get intruppted (process goes back to blocked queue)   30%     (10%)
        //use up part of the time and terimate (process finishes)                           20%     (1%)
// send message back of waht it did
// terminate this child

int main(int argc, char *argv[]){
    msgbuffer buf;
    buf.mtype = 1;
    int msqid = 0;
    key_t key;

    // get the key for our message queue
    if ((key = ftok("oss.c", 1)) == -1) {
        perror("ftok");
        exit(1);
    }

    //access oss.c message queue
    if ((msqid = msgget(key, PERMS)) == -1) {
        perror("msgget in child");
        exit(1);
    }

    //THIS CAN BE DELETED AFTER TESTING
    printf("Child %d has access to the queue\n",getpid());

    // receive a message from oss, but only one for our PID
    if (msgrcv(msqid, &buf, sizeof(msgbuffer), getpid(), 0) == -1) {
        perror("failed to receive message from parent\n");
        exit(1);
    }

    //output message from parent
    //THIS CAN BE DELETED AFTER TESTING
    printf("Child %d received message: %s was my message and my int data was %d\n",getpid(), buf.strData, buf.intData);

    //initialization for the childs random, weighted, choosing period
    int quantum = atoi(buf.strData); //converts quantum message string to an integer
    int random_event = randomNumberGenerator(100);
    int message_back;
    char usedQ[10];
    int i;

    printf("This is the random event: %i\n", random_event);


    //uses up all time, returns to ready queue in oss
    if (random_event < 50){
        message_back = quantum;  // used all of time quantum

    }
    //uses up part of the time quantum, gets blocked in oss
    else if (random_event < 80 && random_event >= 50){

        message_back = randomNumberGenerator(quantum-1);    //returns 1 - (quantum-1)
    }
    //uses up part of the time quantum, terminates in oss
    else if (random_event >= 80){

       message_back = -randomNumberGenerator(quantum-1);    //returns a negative
    } 

    //convert quantum int back to string to send message back to our parent
    snprintf(usedQ, sizeof(usedQ), "%i", message_back); 
   
    // now send a message back to our parent
    buf.mtype = getppid();
    buf.intData = getppid();

    //copy the quantum returned from choosing period into msg queue string data
    strcpy(buf.strData, usedQ);

    //send that message back to oss and have it decide what queue to put the process in or if it will terminate the process
    if (msgsnd(msqid,&buf,sizeof(msgbuffer)-sizeof(long),0) == -1) {
        perror("msgsnd to parent failed\n");
        exit(1);
    }

    //THIS CAN BE DELETED AFTER TESTING
    printf("Child %d is ending\n",getpid());

    return 0;
}