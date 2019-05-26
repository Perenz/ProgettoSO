#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/start.c"

void sigusr2_handler(int sig){
    return getManualPid(collegati_list, magazzino_list);
}

void sigint_handler(int sig){
    char **args;
    remove("/tmp/getPidCenFIFO");
    cen_exit(args, collegati_list, magazzino_list);
    exit(0);
}

int main(){
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGINT, sigint_handler);

    cen_start();

    exit(0);
}