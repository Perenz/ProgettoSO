#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int main(char **args){
    int id=getpid();
    int idPar = getppid();

    printf("Lampadina creata\n");
    printf("Pid: %d\nPid padre: %d\n", id, idPar);
    
    //Invio segnale al padre
    kill(idPar, SIGCONT);
    

    exit(0);
}
