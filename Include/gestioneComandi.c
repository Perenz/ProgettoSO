#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include "../strutture/listH.h"



#define CEN_DELIM " \t\r\n\a"
#define CEN_BUFSIZE 128
#define ANSWER 32

int ascolta_risposta(NodoPtr nodo, char* answer);
char* broadcast(NodoPtr procList, char** comando, char* comando_compatto);
char* broadcast_list(NodoPtr procList, char** comando, char* comando_compatto);

#define BUFSIZE 128
char* getLine(){
    char *cmd=NULL;
    //Dimensione buffer per riallocazione
    size_t  bufS = 0;
    getline(&cmd, &bufS, stdin);

    /*
     * Non più necessario come lo splitLine
     *
    //Rimuovo newLine \n a fine stringa
    cmd = strtok(cmd, "\n");
    */

    return cmd;
}

//Restituita una stringa la funzione la splitta in
//diverse stringhe secondo i delimitatori specificati
//nella macro CEN_DELIM
char** splitLine(char *line){
    int pos=0, bufS = CEN_BUFSIZE;
    char **commands = malloc(bufS * sizeof(char));
    char *cmd;

    //IF error in the allocation of commands
    if(!commands){
        fprintf(stderr, "cen: allocation (malloc) error\n");
        //Exit with error
        exit(1);
    }

    cmd=strtok(line, CEN_DELIM);
    while(cmd!=NULL){
        commands[pos++]=cmd;

        //Realocation of the buffer if we have exceeded its size
        if(pos >= bufS){
            bufS += CEN_BUFSIZE;
            commands = realloc(commands, bufS * sizeof(char));
            //IF error in the allocation of commands
            if(!commands){
                fprintf(stderr, "cen: allocation (malloc) error\n");
                //Exit with error
                exit(1);
            }
        }
        cmd = strtok(NULL, CEN_DELIM);
    }
    commands[pos]=NULL;
    return commands;
}

char* compatta(char** comando){
   
}

//da mettere in function declaration
char* broadcast(NodoPtr procList, char** comando, char* comando_compatto){
    //compatto il comando da mandare in broadcast
    /*
    char* comando_compatto = malloc(BUFSIZE);
    comando_compatto = compatta(comando);
    */
    NodoPtr nodo = procList;
    //char* answer = malloc(BUFSIZE);
    char* answer = malloc(ANSWER);
    while(nodo != NULL){
        //TODO gestire errori
        write(nodo->fd_writer,comando_compatto, strlen(comando_compatto));
        kill(nodo->data, SIGUSR1);            
        //TODO
        //pause();

        //gestione read diversa per ogni comando --> potrei farla qua la gestione
        int toRtn = ascolta_risposta(nodo, answer);
        if(toRtn == 1){
            //printf("%s\n", answer);
            return answer;
        }
        memset(answer, 0, ANSWER);
        //memset(tmp,0,30);
        //strcat(msg,tmp);
        nodo = nodo->next;
    }
    return "0";
}

//nell'hub manda e concatena
char* broadcast_list(NodoPtr procList, char** comando, char* comando_compatto){
    //compatto il comando da mandare in broadcast
    /*
    char* comando_compatto = malloc(BUFSIZE);
    comando_compatto = compatta(comando);
    */
    
    NodoPtr nodo = procList;
    char* answer = malloc(ANSWER);
    char* list_answer = malloc(BUFSIZE*10);
    memset(answer, 0, ANSWER);
    memset(list_answer, 0, BUFSIZE*10);
    //printList(procList);
    
    while(nodo != NULL){
        //TODO gestire errori
        write(nodo->fd_writer,comando_compatto,strlen(comando_compatto));
        kill(nodo->data, SIGUSR1);            
        //TODO
        
        //pause();
        
        int err = ascolta_risposta(nodo, answer);
        strcat(list_answer,"\t\t-");
        strcat(list_answer, answer);
        memset(answer,0,ANSWER);
        nodo = nodo->next;
        
    }
    //return "0";
    return list_answer;
    
    
}

//ritorna 0 se non trovato
int ascolta_risposta(NodoPtr nodo, char* answer){
    int temp = read(nodo->fd_reader, answer, ANSWER);
    if(strcmp(&answer[0], "0") != 0){
        return 1;
    }else{
        return 0;
    }

}


