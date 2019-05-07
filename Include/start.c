#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../strutture/listH.h"
#include "functionDeclarations.c"
#include "gestioneComandi.c"

int cen_processCmd(char **command, NodoPtr);
char* cen_getLine();
char** cen_splitLine(char *line);




int cen_start(){
    char *command;
    char **params;

    size_t bufS = 0;
    int status =1;
    NodoPtr procList = NULL;
    //Inserisco nella lista il pid corrente indicante la centraline stessa --> TODO secondo me sbagliato 
    procList = listInit(getpid());

    //Continuo ad ascoltare in input su stdin
    do{
        printf("Inserisci il comando:>");

        //Prendo il comando dall'utente
        command = getLine();

        //Splitta la linea in singoli parametri/argomenti
        params = splitLine(command);

        //Esegue il comando
        status = cen_processCmd(params, procList);
    }while(status);
}



int cen_processCmd(char **command, NodoPtr procList){
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
    int i=0;
    //se inserisco un comando vuoto richiedo di inserire un nuovo comando
    if(command[0] == NULL)
        return 1;

    for(i; i<cen_numCommands(); i++){
        if(strcmp(command[0],builtin_cmd[i])==0)
            return builtin_func[i](command, procList);
    }

    //Se comando inserito non esiste
    printf("Comando non riconosciuto, digitare \"help\" per la lista\n");
    return 1;
}



