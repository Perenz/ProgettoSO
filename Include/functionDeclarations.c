#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "../strutture/listH.h"

//Functions for commands
int cen_prova(char **args, NodoPtr procList);
int cen_clear(char **args, NodoPtr procList);
int cen_exit(char **args, NodoPtr procList);
int cen_help(char **args, NodoPtr procList);
int cen_add(char **args, NodoPtr procList);
int cen_list(char **args, NodoPtr procList);
int cen_numCommands();
int fd[2]; //fd[0]legge fd[1]scrive


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
int (*builtin_func[]) (char **, NodoPtr) = {
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

    int i=0;
    for(i; i<cen_numCommands();i++){
        printf("> %s\n", builtin_cmd[i]);
    }

    return 1;
}

void sign_cont_handler(int sig){
        return;
}




int cen_list(char **args, NodoPtr procList){
        printf("Stampo la lista dei dispositivi tramite il loro pid\n");
        //printList(procList);
        char tmp[30];
        NodoPtr Nodo = procList;

        //Escludo la centralina dal while e la stampo SINGOLARMENTE
        Nodo = Nodo->next;
        printf("\nCen %d accesa\n", getpid());

        signal(SIGCONT, sign_cont_handler);


        /*
        PROBABILE ERRORE:
        La variabile fd a cui facciamo riferimento all'interno del while non è la stessa
        fd su cui apriamo la pipe in addDevice.
        è necessario quindi prendere quella fd, memorizzarla in una struttura ed assicurarsi
        di utilizzarla nuovamente per la lettura
        */
        while(Nodo != NULL){
                kill(Nodo->data, SIGUSR1);
                //printf("Mi metto in read dal figlio %d sul canale %d\n", Nodo->data, Nodo->fd[0]);
                //pause();
                
                //TODO
                int temp = read(Nodo->fd[0],tmp,30);
                printf("%s\n", tmp);
                memset(tmp,0,30);
                //strcat(msg,tmp);
                Nodo = Nodo->next;
        }
        
        return 1;
}