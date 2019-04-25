#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>



int main(char **args){
    pid_t id=getpid(); // chiedo il mio pid
    pid_t idPar = getppid(); //chiedo il pid di mio padre
    //0 spenta
    //1 accesa
    int status = 0;

    printf("\nFridge creato\n");
    printf("Pid: %d\nPid padre: %d\n", id, idPar);

    //Invio segnale al padre
    int ris = kill(idPar, SIGCONT); 

    //Child va in pausa
    printf("Child va in pausa\n");
    pause();


    printf("Child ora termina\n");   
    exit(0);
}