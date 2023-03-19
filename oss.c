// Name: Gabrielle Hieken
// Class: 4760 Operating Systems
// Date: 3/19/2023

#include <stdio.h>
#include <getopt.h> //Needed for optarg function
#include <stdlib.h> //EXIT_FAILURE

int main(int argc, char *argv[]){

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

    printf("The name of your logfile: ", logFile);
}