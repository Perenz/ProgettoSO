#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <assert.h>
#include <sched.h>
int status;
int fd;
pid_t idPar;
pid_t id;

void sighandle_int(int sig) {
    if(sig==SIGQUIT)
        _exit(0);      
    //pause();
}

//SIGUSR1 usato per l'implementazione del comando list
void sighandle_usr1(int sig){
    if(sig == SIGUSR1){
        //printf("lampadina %d sta scrivendo\n", id);
        char str[50];
        sprintf(str, "Bulb %d", id);
        //printf("Figlio ora scrive\n");
        strcat(str ,(status==1?" accesa\n":" spenta\n"));
        
        int esito = write(fd,str,strlen(str)+1);
        //printf("Figlio ha scritto con esito %d\n", esito);
        kill(idPar,SIGCONT);
        pause();
    }
}

void signhandle_quit(int sig){
    _exit(0);
}


int main(int argc, char *args[]){
    id =getpid(); // chiedo il mio pid
    idPar = getppid(); //chiedo il pid di mio padre

    //0 spenta
    //1 accesa
    status = 0; 

    //leggo args per prendere gli argomenti passati(puntatore al lato di scrittura della pipe)
    fd = atoi(args[1]);

    
    //Mando il msg sul fd e ne stampo l'esito
    char msg[50];
    sprintf(msg, "Bulb %d", id);
    //printf("Figlio ora scrive su canale %d\n", fd);
    strcat(msg ,(status==1?" accesa\n":" spenta\n"));
    int rtn = write(fd,msg,strlen(msg)+1);
    //printf("Esito invio %d\nmsg inviato: %s\n", rtn, msg);

    
    signal(SIGINT, sighandle_int);
    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1

    printf("\nLampadina creata\n");
    printf("Pid: %d\nPid padre: %d\n", id, idPar);

    //Invio segnale al padre
    int ris = kill(idPar, SIGCONT); 

    //Child va in pausa
    
    
    while(1){
        printf("Child va in pausa\n");
        pause();
    }

    //printf("Child ora termina\n");   
    exit(0);
}


