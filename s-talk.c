// Main s-talk file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "list.h"

int main(int argCount, char** args){
    // Check number of input arguments for validity
    if(argCount < 4){
        printf("Error: s-talk invalid request: too few arguments supplied\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }
    else if(argCount > 4){
        printf("Error: s-talk invalid request: too many arguments supplied\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }

    // input arguments:
    int my_port = atoi(args[1]);    // local port number
    char* rem_name = args[2];       // remote machine name
    int rem_port = atoi(args[3]);   // remote port number

    // check input arguments:
    if ( (my_port == 0) || (my_port < 1024) || (my_port > 49151) ){
        printf("Error: s-talk invalid request: invalid local port number (%s)\n", args[1]);
        printf("    Port numbers must be integers in the range 1024 to 49151.\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }
    if ( (rem_port == 0) || (rem_port < 1024) || (rem_port > 49151) ){
        printf("Error: s-talk invalid request: invalid remote port number (%s)\n", args[3]);
        printf("    Port numbers must be integers in the range 1024 to 49151.\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }

    printf("successful arguments supplied!\n");
    printf("s-talk %d %s %d\n", my_port, rem_name, rem_port);

    // TODO!!!
    // setup the threads, sockets, etc.

}