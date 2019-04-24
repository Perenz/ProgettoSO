#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>



int main(char **args){
    pid_t id=getpid();
    pid_t idPar = getppid();

    printf("Lampadina creata\n");
    printf("Pid: %d\nPid padre: %d\n", id, idPar);
    
    //Invio segnale al padre
    int ris = kill(idPar, SIGCONT);

    //Child va in pausa
    printf("Child va in pausa\n");
    pause();

    printf("Child ora termina\n");   
    exit(0);
}


