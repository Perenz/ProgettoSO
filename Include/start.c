#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_CMD "exit"
#define CLEAR_CMD "clear"

int processCmd(char *command);


int start(){
    char *command;
    ssize_t bufS = 0;
    int status =1;

    //Continuo ad ascoltare in input su stdin
    do{
        printf("Inserisci il comando:>");
        //Prendo il comando dall'utente
        getline(&command, &bufS, stdin);
        command = strtok(command, "\n");

        //Esegue il comando
        status = processCmd(command);
    }while(status);
}

int processCmd(char *command){
    //Esco se cmd "exit"
    if(strcmp(command, EXIT_CMD)==0) {
        return 0;
    }

    //Pulisco terminale se cmd "clear"
    if(strcmp(command, CLEAR_CMD)==0) {
        system("clear");
        return 1;
    }

    printf("Hai inserito %s", command);
    return 1;
}



