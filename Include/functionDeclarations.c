#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Functions for commands
int cen_prova(char **args);
int cen_clear(char **args);
int cen_exit(char **args);
int cen_help(char **args);
int cen_add(char **args);
int cen_numCommands();

//Commands list
char *builtin_cmd[]={
        "prova",
        "clear",
        "help",
        "exit",
        "add"
};

//Pointers list to a Function associated to each command
int (*builtin_func[]) (char **) = {
        &cen_prova,
        &cen_clear,
        &cen_help,
        &cen_exit,
        &cen_add
};

int cen_numCommands(){
    return (sizeof(builtin_cmd)/ sizeof(char*));
}

int cen_prova(char **args){
    printf("Hai inserito il comando %s\n", args[0]);
    return 1;
}

int cen_clear(char **args){
    system("clear");
    return 1;
}

int cen_exit(char **args){
    return 0;
}

int cen_help(char **args){
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