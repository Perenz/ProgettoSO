#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "./addDevice.c"


//Functions for commands
int cen_prova(char **args, NodoPtr procList, NodoPtr dispList);
int cen_clear(char **args, NodoPtr procList, NodoPtr dispList);
int cen_exit(char **args, NodoPtr procList, NodoPtr dispList);
int cen_help(char **args, NodoPtr procList, NodoPtr dispList);
int cen_add(char **args, NodoPtr procList, NodoPtr dispList);
int cen_list(char **args, NodoPtr procList, NodoPtr dispList);
int cen_numCommands();

//Commands list
char *builtin_cmd[]={
        "prova",
        "clear",
        "help",
        "exit",
        "add",
        "list"
};

//Pointers list to a Function associated to each command
int (*builtin_func[]) (char **, NodoPtr, NodoPtr) = {
        &cen_prova,
        &cen_clear,
        &cen_help,
        &cen_exit,
        &cen_add,
        &cen_list
};

int cen_numCommands(){
    return (sizeof(builtin_cmd)/ sizeof(char*));
}

int cen_prova(char **args, NodoPtr procList, NodoPtr dispList){
    printf("Hai inserito il comando %s\n", args[0]);
    return 1;
}

int cen_clear(char **args, NodoPtr procList, NodoPtr dispList){
    system("clear");
    return 1;
}

int cen_exit(char **args, NodoPtr procList, NodoPtr dispList){
    signal(SIGQUIT, SIG_IGN);
    while(procList!=NULL){
            kill(procList->data, SIGQUIT);
            procList=procList->next;
    }
    while(dispList!=NULL){
            kill(dispList->data, SIGQUIT);
            dispList=dispList->next;
    }

    //Bisogna deallocare la lista procList
    freeList(procList);
    freeList(dispList);

    return 0;
}

int cen_help(char **args, NodoPtr procList, NodoPtr dispList){
    printf("Progetto SO realizzato da: Paolo Tasin, "
           "Stefano Perenzoni, Marcello Rigotti\n");
    printf("Centralina per controllo domotico\n");
    printf("Digitare i seguenti comandi:\n");

    int i=0;
    for(i; i<cen_numCommands();i++){
        printf("> %s\n", builtin_cmd[i]);
    }

    return 1;
}

int cen_list(char **args, NodoPtr procList, NodoPtr dispList){
        printf("Stampo la lista dei dispositivi tramite il loro pid\n");
        printf("Dispositivi attivi ma non collegati: \n");
        printList(dispList);
        printf("Dispositivi collegati: \n");
        printList(procList);
}