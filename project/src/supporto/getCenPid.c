#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define myFIFO "/tmp/getPidCenFIFO"

void signhandle_quit(int sig){
    remove(myFIFO);
    _exit(0);
}


int main(){
    int fd;
    char msg[10];
    //Variabile per memorizzare il parent pid
    //QUindi il pid della centralina
    int cenPid=getppid();

    signal(SIGQUIT, signhandle_quit);
    //Creo la fifo
    if(mkfifo(myFIFO, 0666)<0){
        fprintf(stderr, "Errore nella creazione della fifo %s: %s\n", myFIFO, strerror(errno));
    }
    

    while(1){
        //La apro per read only
        fd = open(myFIFO, O_RDONLY);
        if(fd<0){
            fprintf(stderr, "Errore nell'apertura READ ONLY della fifo %s: %s\n", myFIFO, strerror(errno));
        }

        //Leggo dalla fifo
        if(read(fd, msg, 10)<0){
            fprintf(stderr, "Errore nella lettura dalla fifo %s: %s\n", myFIFO, strerror(errno));     
        }
        if(strcmp(msg, "hand")==0){
            //RISPONDO con il PID della cen
            //Chiudo la fifo in read only
            close(fd);
            //Apro in scrittura
            fd = open(myFIFO, O_WRONLY);
            if(fd<0){
                fprintf(stderr, "Errore nell'apertura WRITE ONLY della fifo %s: %s\n", myFIFO, strerror(errno));
            }

            //Rispondo con il pid della centralina
            //Cioe il ppid di tale processo
            sprintf(msg, "%d", cenPid);
            if(write(fd, msg, strlen(msg)+1)<0){
                fprintf(stderr, "Errore nella scrittura sulla fifo %s: %s\n", myFIFO, strerror(errno));     

            }
            //Chiudo in scrittura
            close(fd);
        }
        memset(msg,0,10); //Resetto il buffer del messaggio
    }
    
    exit(0);
}