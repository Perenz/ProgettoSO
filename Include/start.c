#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_CMD "exit\n"

int processCmd(char *command);


int start(){
    char command[60];
    int status =1;
    do{
        printf("Inserisci il comando:>");
        fgets(command, 60, stdin);

        status = processCmd(command);
    }while(status);
}

int processCmd(char *command){
    if(strcmp(command, EXIT_CMD)==0) {
        printf("ENTRATO");
        return 0;
    }

    printf("Hai inserito %s", command);
    return 1;
}



