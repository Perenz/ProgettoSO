#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "../strutture/comandiH.h"
int id_gen = 2;

int add_device(char* execPath, NodoPtr magazzino_list, char* nome);
int add_device_generale(char* execPath, NodoPtr list, info info, char* nome);
void sign_handler(int);
//device list
char *builtin_device[]={
        "bulb",
        "window",
        "fridge",
        "timer",
        "hub"
};

char *builtin_dev_path[]={
        "./binaries/BULB",
        "./binaries/WINDOW",
        "./binaries/FRIDGE",
        "./binaries/TIMER",
        "./binaries/HUB"
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
int add_device_generale(char* execPath, NodoPtr list, info info, char* nome){
    
    //info verrà gestito da ogni 
    pid_t pid, wpid;
    int fd_reader[2];
    int fd_writer[2];

    pipe(fd_reader); //creo la pipe
    pipe(fd_writer);
    pid=fork();
    if(pid == 0) {     

        char fd_writer_Tmp[10];
        char fd_reader_Tmp[10];
        close(fd_reader[0]);
        close(fd_writer[1]);
        sprintf(fd_writer_Tmp,"%d", (fd_writer[0]));
        sprintf(fd_reader_Tmp,"%d", (fd_reader[1]));
        //levo il campo info (args[3])
        char *args[]={execPath,fd_writer_Tmp, fd_reader_Tmp, nome, NULL}; 
        execvp(args[0],args); //passo gli argomenti incluso il puntatore al lato di scrittura della pipe
    } else if (pid < 0) {
        perror("errore fork");
    } 
    else{
        signal(SIGCONT, sign_handler);//?
        close(fd_reader[1]);
        close(fd_writer[0]);
        
        int err = write(fd_writer[1],&info,sizeof(info));
        if(err == -1) 
            printf("porcoddue\n");
        list = insertLast(list, pid, fd_reader[0],fd_writer[1]);
        //Vado in pausa per permettere al figlio di generarsi
        pause();

            
    }
    
    return 1;
}

int add_device(char* execPath, NodoPtr magazzino_list, char* nome){
    //add device di default chiamato dalla centralina quando viene aggiunto un dispositivo
    //id_gen+=1;
    //char info[16];
    info infoD;
    infoD.def = 1; //info default = 1
    strcpy(infoD.nome,nome);
    infoD.id = id_gen;
    
    add_device_generale(execPath, magazzino_list, infoD, nome);
    return 1;
}


