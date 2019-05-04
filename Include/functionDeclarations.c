#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "addDevice.c"


//Functions for commands
int cen_prova(char **args, NodoPtr procList);
int cen_clear(char **args, NodoPtr procList);
int cen_exit(char **args, NodoPtr procList);
int cen_help(char **args, NodoPtr procList);
int cen_add(char **args, NodoPtr procList);
int cen_list(char **args, NodoPtr procList);
int cen_delete(char **args, NodoPtr procList);
int cen_switch(char **args, NodoPtr procList);

int cen_numCommands();




//Commands list
char *builtin_cmd[]={
        "prova",
        "clear",
        "help",
        "exit",
        "add",
        "list",
        "delete",
        "switch"
};

//Pointers list to a Function associated to each command
int (*builtin_func[]) (char **, NodoPtr) = {
        &cen_prova,
        &cen_clear,
        &cen_help,
        &cen_exit,
        &cen_add,
        &cen_list,
        &cen_delete,
        &cen_switch
};



int cen_numCommands(){
    return (sizeof(builtin_cmd)/ sizeof(char*));
}

int cen_prova(char **args, NodoPtr procList){
    printf("Hai inserito il comando %s\n", args[0]);
    return 1;
}

int cen_clear(char **args, NodoPtr procList){
    system("clear");
    return 1;
}

int cen_exit(char **args, NodoPtr procList){
    signal(SIGQUIT, SIG_IGN);
    while(procList!=NULL){
            kill(procList->data, SIGQUIT);
            procList=procList->next;
    }

    //Bisogna deallocare la lista procList
    freeList(procList);

    return 0;
}

int cen_help(char **args, NodoPtr procList){
    printf("Progetto SO realizzato da: Paolo Tasin, "
           "Stefano Perenzoni, Marcello Rigotti\n");
    printf("Centralina per controllo domotico\n");
    printf("Digitare i seguenti comandi:\n");

    for(int i=0; i<cen_numCommands();i++){
        printf("> %s\n", builtin_cmd[i]);
    }

    return 1;
}

void sign_cont_handler(int sig){
        return;
}
//COMANDO CHE CENTRALINA IMPARTISCE --> l
int cen_list(char **args, NodoPtr procList){
        printf("Stampo la lista dei dispositivi tramite il loro pid\n");
        //printList(procList);
        char tmp[30];
        NodoPtr Nodo = procList;
        //printList(Nodo);
        printf("\nCen %d accesa\n", Nodo->data);

        //Escludo la centralina dal while e la stampo SINGOLARMENTE Modifica Paolo --> centralina non l'aggiungo
        Nodo = Nodo->next;

        signal(SIGCONT, sign_cont_handler);

        //TODO possibile metterlo in funzione "broadcast"
        while(Nodo != NULL){
                //TODO gestire errori
                write(Nodo->fd_writer,"l\0",2);
                kill(Nodo->data, SIGUSR1);
                //printf("Mi metto in read dal figlio %d sul canale %d\n", Nodo->data, Nodo->fd[0]);
                //pause();
                
                //TODO
                pause();
                int temp = read(Nodo->fd_reader,tmp,30);
                printf("%s\n", tmp);
                memset(tmp,0,30);
                //strcat(msg,tmp);
                Nodo = Nodo->next;
        }
        
        return 1;
}
//COMANDO CHE CENTRALINA IMPARTISCE --> d <tipo> <id>
int cen_delete(char **args, NodoPtr procList){
        if(args[1]==NULL || args[2]==NULL){
                printf("Argomenti non validi\n");
                printf("Utilizzo: del <device> <num>\n");
                printf("Comando 'device' per vedere la lista di quelli disponibili\n");
                return 1;
        }else{
            
            for(int i=0; i<device_number(); i++){
                if(strcmp(args[1], builtin_device[i])==0){
                    NodoPtr Nodo = procList;
                    //Escludo la centralina dal while
                    Nodo = Nodo->next;

                    signal(SIGCONT, sign_cont_handler);
                    
                    char* tmp = malloc(strlen(args[1]) + strlen(args[2]) + 10);//4 perché c'è il comando, 2 spazi di sep. e la terminazione
                    
                    //concateno per poter scrivere sulla pipe, non mi piace, posso farlo come scritto a inizio metodo così
                    //è semplice da leggere
                    char* tipo = malloc(2);//2 caratteri
                    //prendo solo l'iniziale del tipo per identificare il tipo di dispositivo
                    tipo[0] = args[1][0];
                    tipo[1] = '\0';

                    strcat(tmp,"d ");
                    strcat(tmp, tipo);
                    strcat(tmp," ");
                    strcat(tmp, args[2]);
                    strcat(tmp, "\0");
                    printf("scrittura lato padre: %s\n", tmp);

                    while(Nodo != NULL){
                        //scrivo il comando sulla pipe
                        printf("Dimensione tmp: %d\n", strlen(tmp));
                        write(Nodo->fd_writer, tmp, strlen(tmp) + 1);
                        kill(Nodo->data, SIGUSR1);
                        //printf("Mi metto in read dal figlio %d sul canale %d\n", Nodo->data, Nodo->fd[0]);
                        pause();
                            
                        //TODO
                        int temp = read(Nodo->fd_reader,tmp,30);
                        printf("funzica: %s\n", tmp);
                        //removeNode(atoi(tmp));
                        //memset(tmp,0,30);
                        //strcat(msg,tmp);
                        Nodo = Nodo->next;
                    }
                    
                    
                    return 1; //esci che sennò va avanti
                }
                    
            }
        }
        printf("Device indicato non riconosciuto\n");
        printf("Utilizzo: del <device> <num>\n");
        printf("Digitare il comando 'device' per la lista di quelli disponibili\n");
        
    return 1;
}

int cen_add(char **args, NodoPtr procList){
        if(args[1]==NULL){
                printf("Argomenti non validi\n");
                printf("Utilizzo: add <device>\n");
                printf("Comando 'device' per vedere la lista di quelli disponibili\n");
                return 1;
        }
        //3 device disponibili: bulb, window, fridge
        else{
            for(int i=0; i<device_number(); i++){
                if(strcmp(args[1], builtin_device[i])==0)
                        return add_device(bultin_dev_path[i], procList);
            }
        }

        printf("Device indicato non riconosciuto\n");
        printf("Utilizzo: add <device>\n");
        printf("Digitare il comando 'device' per la lista di quelli disponibili\n");

        return 1;        
}
//COMANDO CHE CENTRALINA IMPARTISCE --> c <tipo> <id> <stato>
int cen_switch(char **args, NodoPtr procList){
    //è uguale a cen_delete PORCODDDDIO
    if(args[1]==NULL || args[2]==NULL){
                printf("Argomenti non validi\n");
                printf("Utilizzo: del <device> <num>\n");
                printf("Comando 'device' per vedere la lista di quelli disponibili\n");
                return 1;
        }else{
            
            for(int i=0; i<device_number(); i++){
                if(strcmp(args[1], builtin_device[i])==0){
                    NodoPtr Nodo = procList;
                    //Escludo la centralina dal while
                    Nodo = Nodo->next;

                    signal(SIGCONT, sign_cont_handler);
                    
                    char* tmp = malloc(strlen(args[1]) + strlen(args[2]) + 10);//4 perché c'è il comando, 2 spazi di sep. e la terminazione
                    
                    //concateno per poter scrivere sulla pipe, non mi piace, posso farlo come scritto a inizio metodo così
                    //è semplice da leggere
                    char* tipo = malloc(2);//2 caratteri
                    //prendo solo l'iniziale del tipo per identificare il tipo di dispositivo
                    tipo[0] = args[1][0];
                    tipo[1] = '\0';

                    strcat(tmp,"c ");
                    strcat(tmp, tipo);
                    strcat(tmp," ");
                    strcat(tmp, args[2]);
                    strcat(tmp," ");
                    strcat(tmp, args[3]);
                    printf("scrittura lato padre: %s\n", tmp);

                    while(Nodo != NULL){
                        //scrivo il comando sulla pipe
                        printf("Dimensione tmp: %d\n", strlen(tmp));
                        write(Nodo->fd_writer, tmp, strlen(tmp) + 1);
                        kill(Nodo->data, SIGUSR1);
                        //printf("Mi metto in read dal figlio %d sul canale %d\n", Nodo->data, Nodo->fd[0]);
                        //pause();
                            
                        //TODO
                        pause();
                        //int temp = read(Nodo->fd_reader,tmp,30);
                        printf("funzica: %s\n", tmp);
                        //removeNode(atoi(tmp));
                        //memset(tmp,0,30);
                        //strcat(msg,tmp);
                        
                        
                        Nodo = Nodo->next;
                    }
                    
                    
                    return 1; //esci che sennò va avanti
                }
                    
            }
        }
        printf("Device indicato non riconosciuto\n");
        printf("Utilizzo: del <device> <num>\n");
        printf("Digitare il comando 'device' per la lista di quelli disponibili\n");
        
    return 1;
}
