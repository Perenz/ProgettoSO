#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../strutture/listH.h"
#include "functionDeclarations.c"


#define CEN_DELIM " \t\r\n\a"
#define CEN_BUFSIZE 128

NodoPtr procList = NULL; //lista dei dispositivi collegati 
NodoPtr dispList; //lista dei dispositivi disponibili (aggiunti ma non collegati a niente)

int cen_processCmd(char **command, NodoPtr, NodoPtr);
char* cen_getLine();
char** cen_splitLine(char *line);
int lanciaGetCenPid();


int lanciaGetCenPid(){
    int pid;
    pid=fork();
    if(pid<0){
        perror("cen");
        exit(0);
    }else if(pid == 0){
        //Child code
        char *args[]={"./supporto/CENPIDREAD",NULL}; 
        execvp(args[0],args);
    }
    else if(pid >0){
        //parent code
        return pid;
    }
    //toglie il warning gg
    return 1;
}

int cen_processCmd(char **command, NodoPtr procList, NodoPtr dispList){
    if(command[0] == NULL)
        return 1;
    int i;
    for(i=0; i<cen_numCommands(); i++){
        if(strcmp(command[0],builtin_cmd[i])==0)
            return builtin_func[i](command, procList, dispList);
    }

    //Se comando inserito non esiste
    printf("Comando non riconosciuto, digitare \"help\" per la lista\n");
    return 1;
}


int cen_start(){
    char *command;
    char **params;

    size_t bufS = 0;
    int status = 1;
    int supportReadPid;

    
    //Inserisco nella lista il pid corrente indicante la centraline stessa
    procList = listInit(getpid());
    dispList = listInit(0);

    //Lancio i processi di supporto
    //getCenPid che comunica tramite FIFO con manuale.c
    supportReadPid = lanciaGetCenPid();
    //TODO assicurarsi che il processo CENPIDREAD venga terminato anche quando si esce dal programma tramite ctr+C
    //Cosi come tutte le componenti


    //Continuo ad ascoltare in input su stdin
    do{
            printf("\033[0;34m"); //Set the text to the color red
            printf("Inserisci il comando:>");
            printf("\033[0m");
            command = getLine();

        //Splitta la linea in singoli parametri/argomenti
        params = splitLine(command);
        //TODO potrei passare il comando non splittato così da poterlo mandare direttamente
        //Esegue il comando
        status = cen_processCmd(params, procList, dispList);
    }while(status);

    kill(supportReadPid, SIGQUIT);
    return 1;
}







