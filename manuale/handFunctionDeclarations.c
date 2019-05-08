#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int hand_control(char **); //Dovra ritornare il pid del dispositivo specificato come argomento
int hand_help(char **);
int hand_exit(char **);

char *noControl_builtin_cmd[]={
        "control", //Prende in input un parametro, il nome/id del dispositivo su cuiu vogliamo agire
        "help",
        "exit"
};

//Pointers list to a Function associated to each command
int (*noControl_builtin_func[]) (char **) = {
        &hand_control,
        &hand_help,
        &hand_exit
};

char *control_builtin_cmd[]={
        "exit"
};

int (*control_builtin_func[]) (char **) = {
        &hand_exit
};



int cen_numCommands(int type){
    if(type==0)
        return (sizeof(noControl_builtin_cmd)/ sizeof(char*));
    else{
        return (sizeof(control_builtin_cmd)/ sizeof(char*));
    }
}

int hand_control(char **args){
    if(args[1]==NULL || args[2]!=NULL){
            printf("Errore nei parametri\n");
            printf("Usage: control <pid/nome>\n");
            
            return -1;
    }
    else{
            //Tramite comando info????
            printf("Chiedo alla centralina il pid del dispositivo richiesto\n");

            //TODO

            //Per ora ritorno -1;
            return -1;
    }
}

int hand_help(char **args){     
    printf("Progetto SO realizzato da: Paolo Tasin, "
           "Stefano Perenzoni, Marcello Rigotti\n");
    printf("Centralina per controllo domotico\n");
    printf("Comandi accettati se non si controlla alcun dispositivo:\n");

    int i=0;
    for(i; i<cen_numCommands(0);i++){
        printf("> %s\n", noControl_builtin_cmd[i]);
    }

    printf("Comandi accettati mentre si controlla qualche dispositivo:\n");
    i=0;
    for(i; i<cen_numCommands(1);i++){
        printf("> %s\n", control_builtin_cmd[i]);
    }

    return -1;
}

int hand_exit(char **args){
        return 0;
}