#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../strutture/listH.h"
#include "functionDeclarations.c"


#define CEN_DELIM " \t\r\n\a"
#define CEN_BUFSIZE 128
#include "gestioneComandi.c"

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
}


int cen_start(){
    char *command;
    char **params;

    size_t bufS = 0;
    int status =1;
    int supportReadPid;
    NodoPtr procList = NULL; //lista dei dispositivi collegati 
    NodoPtr dispList = NULL; //lista dei dispositivi disponibili (aggiunti ma non collegati a niente)

    //Inserisco nella lista il pid corrente indicante la centraline stessa
    procList = listInit(getpid());
    //dispList = listInit(-1);//FUCKMARCELLO 


    //Lancio i processi di supporto
    //getCenPid che comunica tramite FIFO con manuale.c
    supportReadPid = lanciaGetCenPid();
    //TODO assicurarsi che il processo CENPIDREAD venga terminato anche quando si esce dal programma tramite ctr+C
    //Cosi come tutte le componenti


    //Continuo ad ascoltare in input su stdin
    do{
        printf("Inserisci il comando:>");

        //Prendo il comando dall'utente
        command = getLine();

        //Splitta la linea in singoli parametri/argomenti
        params = splitLine(command);
        //TODO potrei passare il comando non splittato così da poterlo mandare direttamente
        //Esegue il comando
        status = cen_processCmd(params, procList, dispList);
    }while(status);

    kill(supportReadPid, SIGQUIT);
}



int cen_processCmd(char **command, NodoPtr procList, NodoPtr dispList){
    //Esecuzione di un comando SINGOLO senza argomenti
/*
    //Eseguo la funziona cen_prova se comando inserito è "prova"
    if(strcmp(command[0], builtin_cmd[0])==0){
        return cen_prova(command);
    }
    else if(strcmp(command[0], builtin_cmd[1])==0) {
        //Eseguo la funziona cen_clear se comando inserito è "clear"
        //Pulisco terminale se cmd "clear"
        return cen_clear(command);
    }
    else if(strcmp(command[0], builtin_cmd[2])==0){
        //Eseguo la funzione cen_help se il comando inserito è "help"
        return cen_help(command);
    }
    else if(strcmp(command[0], builtin_cmd[3])==0) {
        //Eseguo la funziona cen_exit se comando inserito è "exit"
        //Esco se cmd "exit"
        return cen_exit(command);
    }
    else{
        printf("Comando non riconosciuto, digitare \"help\" per la lista\n");
        return 1;
    }
    */
    //se inserisco un comando vuoto richiedo di inserire un nuovo comando
    if(command[0] == NULL)
        return 1;

    for(int i=0; i<cen_numCommands(); i++){
        if(strcmp(command[0],builtin_cmd[i])==0)
            return builtin_func[i](command, procList, dispList);
    }

    //Se comando inserito non esiste
    printf("Comando non riconosciuto, digitare \"help\" per la lista\n");
    return 1;
}



