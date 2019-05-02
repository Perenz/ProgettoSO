#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <assert.h>
#include <sched.h>
int status;

void sighandle_int(int sig) {
    if(sig==SIGQUIT)
        _exit(0);

    printf("\033[0;31m");
    printf("\n\nProva gestore custom!!!\n");
    printf("Bulb:"); printf(status==1?" accesa\n":" spenta\n");
    printf("\033[0m\n");
    //pause();
}

void signhandle_quit(int sig){
    _exit(0);
}


int main(int argc, char *args[]){
    pid_t id=getpid(); // chiedo il mio pid
    pid_t idPar = getppid(); //chiedo il pid di mio padre
    //0 spenta
    //1 accesa
    status = 0; 

    //leggo args per prendere gli argomenti passati(puntatore al lato di scrittura della pipe)
    int fd = atoi(args[1]);
    
    //Mando il msg sul fd e ne stampo l'esito
    char* msg = "ciao gruppo";
    int rtn = write(fd,msg,strlen(msg)+1);
    printf("Esito invio %d\nmsg inviato: %s\n", rtn, msg);

    
    signal(SIGINT, sighandle_int);
    signal(SIGQUIT, signhandle_quit);

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


