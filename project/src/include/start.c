#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../strutture/listH.h"
#include "functionDeclarations.c"


#define CEN_DELIM " \t\r\n\a"
#define CEN_BUFSIZE 128

NodoPtr collegati_list = NULL; //lista dei dispositivi collegati 
NodoPtr magazzino_list; //lista dei dispositivi disponibili (aggiunti ma non collegati a niente)
int supportReadPid;

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
        char *args[]={"./binaries/CENPIDREAD",NULL}; 
        execvp(args[0],args);
    }
    else if(pid >0){
        //parent code
        return pid;
    }
    //toglie il warning gg
    return 1;
}

int cen_processCmd(char **command, NodoPtr collegati_list, NodoPtr magazzino_list){
    if(command[0] == NULL)
        return 1;
    int i;
    for(i=0; i<cen_numCommands(); i++){
        if(strcmp(command[0],builtin_cmd[i])==0)
            return builtin_func[i](command, collegati_list, magazzino_list);
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
    

    
    //Inserisco nella lista il pid corrente indicante la centraline stessa
    collegati_list = listInit(getpid());
    magazzino_list = listInit(0);

    //Lancio i processi di supporto
    //getCenPid che comunica tramite FIFO con manuale.c
    supportReadPid = lanciaGetCenPid();

    //Continuo ad ascoltare in input su stdin
    do{
            printf("\033[0;34m"); //Set the text to the color blue
            printf("Inserisci il comando:>");
            printf("\033[0m");
            command = getLine();

            //Splitta la linea in singoli parametri/argomenti
            params = splitLine(command);
            //Esegue il comando
            status = cen_processCmd(params, collegati_list, magazzino_list);
        
    }while(status);

    kill(supportReadPid, SIGQUIT);
    return 1;
}







