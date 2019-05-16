#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define myFIFO "/tmp/miaFifo"

void signhandle_quit(int sig){
    remove(myFIFO);
    _exit(0);
}


int main(){
    int fd;
    char msg[10];
    char *fifo = "../fifo/manualeCenFifo";
    //Variabile per memorizzare il parent pid
    //QUindi il pid della centralina
    int cenPid=getppid();

    signal(SIGQUIT, signhandle_quit);
    //Creo la fifo
    mkfifo(myFIFO, 0666);

    while(1){
        //La apro per read only
        fd = open(myFIFO, O_RDONLY);

        //Leggo dalla fifo
        read(fd, msg, 10);
        if(strcmp(msg, "hand")==0){
            //RISPONDO con il PID della cen
            //Chiudo la fifo in read only
            close(fd);
            //Apro in scrittura
            fd = open(myFIFO, O_WRONLY);

            //Rispondo con il pid della centralina
            //Cioe il ppid di tale processo
            sprintf(msg, "%d", cenPid);
            write(fd, msg, strlen(msg)+1);
            //Chiudo in scrittura
            close(fd);
        }
        memset(msg,0,10); //Resetto il buffer del messaggio
    }
    
    exit(0);
}