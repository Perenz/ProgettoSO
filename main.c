#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Include/start.c"

void sigusr2_handler(int sig){
    //printf("Sono nel signal handler della centralina\n");
    return getManualPid(procList, dispList);
}

void sigint_handler(int sig){
    char **args;
    remove("/tmp/getPidCenFIFO");
    cen_exit(args, procList, dispList);
    exit(0);
}

int main(){
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGINT, sigint_handler);

    cen_start();

    exit(0);
}