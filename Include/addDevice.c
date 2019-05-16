#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "../strutture/listH.h"

int id = 2;

int add_device(char*, NodoPtr, NodoPtr);
int add_device_generale(char* execPath, NodoPtr list, char* info);
void sign_handler(int);
//device list
char *builtin_device[]={
        "bulb",
        "window",
        "fridge",
        "timer",
        "hub"
};

char *bultin_dev_path[]={
        "./componenti/BULB",
        "./componenti/WINDOW",
        "./componenti/FRIDGE",
        "./componenti/TIMER",
        "./componenti/HUB"
};


int device_number(){
        return sizeof(builtin_device)/sizeof(char*);
}

//Signal handler per gestire l'arrivo di segnali
void sign_handler(int sig){
    return;
}

//qua basta una lista.

//add device generale che viene chiamato dai dispositivi di controllo per "aggiungere"
//un dispositivo che già esisteva (ergo con informazioni non di default)
int add_device_generale(char* execPath, NodoPtr list, char* info){
    //info verrà gestito da ogni 
    pid_t pid, wpid;
    int fd_reader[2];
    int fd_writer[2];

    pipe(fd_reader); //creo la pipe
    pipe(fd_writer);
    pid=fork();
    if(pid == 0) {
        printf("queste sono le info: %s", info);
        
        char fd_writer_Tmp[10];
        char fd_reader_Tmp[10];

        sprintf(fd_writer_Tmp,"%d", (fd_writer[0]));
        sprintf(fd_reader_Tmp,"%d", (fd_reader[1]));
        char *args[]={execPath,fd_writer_Tmp, fd_reader_Tmp , info, NULL}; 
        execvp(args[0],args); //passo gli argomenti incluso il puntatore al lato di scrittura della pipe
    } else if (pid < 0) {
        perror("errore fork");
    } 
    else{
        signal(SIGCONT, sign_handler);
        list = insertLast(list, pid, fd_reader[0],fd_writer[1]);
        //Vado in pausa per permettere al figlio di generarsi
        pause();
            
    }
    
    return 1;
}

int add_device(char* execPath, NodoPtr procList, NodoPtr dispList){
    //add device di default chiamato dalla centralina quando viene aggiunto un dispositivo
    id+=1;
    char info[16];
    sprintf(info, "default %d", id);
    
    add_device_generale(execPath, dispList, info);
    return 1;
}


