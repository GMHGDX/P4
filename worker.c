#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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

    // get a key for our message queue
    if ((key = ftok("oss.c", 1)) == -1) {
        perror("ftok");
        exit(1);
    }

    //access oss.c message queue
    if ((msqid = msgget(key, PERMS)) == -1) {
        perror("msgget in child");
        exit(1);
    }

    printf("Child %d has access to the queue\n",getpid());

    // receive a message, but only one for us
    if ( msgrcv(msqid, &buf, sizeof(msgbuffer), getpid(), 0) == -1) {
        perror("failed to receive message from parent\n");
        exit(1);
    }

    // output message from parent
    printf("Child %d received message: %s was my messageand my int data was %d\n",getpid(),buf.strData, buf.intData);

    // now send a message back to our parent
    buf.mtype = getppid();
    buf.intData = getppid();

    strcpy(buf.strData,"Message back to muh parent\n");

    if (msgsnd(msqid,&buf,sizeof(msgbuffer)-sizeof(long),0) == -1) {
        perror("msgsnd to parent failed\n");
        exit(1);
    }

    printf("Child %d is ending\n",getpid());

    return 0;
}