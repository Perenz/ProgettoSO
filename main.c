#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Include/start.c"

void sigusr2_handler(int sig){
    //printf("Sono nel signal handler della centralina\n");
    return getManualPid(procList, dispList);
}

int main(){
    signal(SIGUSR2, sigusr2_handler);

    cen_start();

    exit(0);
}