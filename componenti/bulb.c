#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
int status;
void sighandle_int(int sig) {
    printf("\033[0;31m");
    printf("\n\nProva gestore custom!!!\n");
    printf("Bulb:"); printf(status==1?" accesa\n":" spenta\n");
    printf("\033[0m\n");
    pause();
}


int main(char **args){
    signal(SIGINT, sighandle_int);
    pid_t id=getpid(); // chiedo il mio pid
    pid_t idPar = getppid(); //chiedo il pid di mio padre
    //0 spenta
    //1 accesa
    status = 0; 

    printf("\nLampadina creata\n");
    printf("Pid: %d\nPid padre: %d\n", id, idPar);

    //Invio segnale al padre
    int ris = kill(idPar, SIGCONT); 

    //Child va in pausa
    printf("Child va in pausa\n");
    pause();


    printf("Child ora termina\n");   
    exit(0);
}


