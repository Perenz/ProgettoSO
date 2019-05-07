#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "addDevice.c"


//Comandi centralina
int cen_prova(char **args, NodoPtr procList);
int cen_clear(char **args, NodoPtr procList);
int cen_exit(char **args, NodoPtr procList);
int cen_help(char **args, NodoPtr procList);
int cen_add(char **args, NodoPtr procList);
int cen_list(char **args, NodoPtr procList);
int cen_delete(char **args, NodoPtr procList);
int cen_switch(char **args, NodoPtr procList);
int cen_info(char **args, NodoPtr procList);

int cen_numCommands();

//Lista dei comandi della centralina
char *builtin_cmd[]={
        "prova",
        "clear",
        "help",
        "exit",
        "add",
        "list",
        "delete",
        "switch",
        "info"
};

//Puntatori alle funzioni relative ai comandi della centralina
int (*builtin_func[]) (char **, NodoPtr) = {
        &cen_prova,
        &cen_clear,
        &cen_help,
        &cen_exit,
        &cen_add,
        &cen_list,
        &cen_delete,
        &cen_switch,
        &cen_info
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

    //Dealloco la lista procList
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

/*
    Funzione: elenca tutti i dispositivi con <nome>, quelli attivi con <nome> <id> 
    Sintassi lato utente: list
    Sintassi comunicata dalla centralina ai figli: l
*/
int cen_list(char **args, NodoPtr procList){
    printList(procList);
        /*
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
        */
        return 1;
}

/*
    Funzione: rimuove il dispositivo <id>, se di controllo elimina anche i dispositivi sottostanti
    Sintassi lato utente:                          delete <id>
    Sintassi comunicata dalla centralina ai figli: d <id>
*/
int cen_delete(char **args, NodoPtr procList){
    //TODO da modificare, pensavo che l'eliminazione avvenisse anche per tipo.
    if(args[1]==NULL){
            printf("Argomenti non validi\n");
            printf("Utilizzo: delete <id>\n");
            return 1;
    }else{

        NodoPtr Nodo = procList;
        //Escludo la centralina dal while
        Nodo = Nodo->next;
        signal(SIGCONT, sign_cont_handler);
        
        char* tmp = malloc(1 + strlen(args[1]) + 3);//1 per il comando + lunghezza id (args[1]) + 2 per spazi e terminazione stringa
        
        //tipo di comando
        strcat(tmp,"d ");
        //id dispositivo da spegnere
        strcat(tmp, args[1]);
        //delimitatore 
        strcat(tmp, "\0");
        printf("scrittura lato padre: %s\n", tmp);

        while(Nodo != NULL){

            //scrivo il comando sulla pipe
            write(Nodo->fd_writer, tmp, strlen(tmp));
            //mando un segnale al figlio così si risveglia e legge il contenuto della pipe
            kill(Nodo->data, SIGUSR1);
            //printf("Mi metto in read dal figlio %d sul canale %d\n", Nodo->data, Nodo->fd[0]);
            //pause();
                
            //TODO gestione errori
            //leggo il pid del figlio così da poterlo togliere dalla lista di processi
            char* answer = malloc(30);
            int err = read(Nodo->fd_reader,answer, 30);
            //TODO se il figlio ritorna 0 esso non è figlio e perciò non lo elimino dalla lista
            int ris = atoi(answer);

            printf("Lettura da pipe lato padre %d\n", ris);

            if(ris!=0){
                removeNode(procList, ris);
                return 1;
            }else
                Nodo = Nodo->next;
            //risveglio il figlio così può eliminarsi
            //kill(Nodo->data, SIGUSR1);
            //memset(tmp,0,30);
            //strcat(msg,tmp);
            
        }
        
        
        return 1; //esci che sennò va avanti
            
                
    }
    
    printf("Device indicato non riconosciuto\n");
    printf("Utilizzo: delete <id>\n");
    
    return 1;
}


/*
    Funzione: aggiunge un device al sistema e ne mostra i dettagli
    Sintassi lato utente:                          add <tipo>
    TODO Sintassi comunicata dalla centralina :    add <tipo> (centralina comunica a processo specifico)
*/
//TODO dobbiamo mostrare i dettagli
//potremo fare che la centralina impartisce un comando ad un qualche processo 
//tipo: a <tipo> <id> 
//e questo processo terrà la lista dei processi aggiunti che poi dovranno essere
//collegati per venire a far parte effettivamente del sistema.
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
/*
    Funzione: modifica <stato> di dispositivo di tipo <tipo> con <id>
    Sintassi lato utente:                          switch <id> <label> <pos>
    Sintassi comunicata dalla centralina :         c <id> <tipo> <stato>
*/
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

                    //meglio search e identifico percorso che broadcast.
                    while(Nodo != NULL){
                        //scrivo il comando sulla pipe
                            printf("Dimensione tmp: %d\n", strlen(tmp));
                        write(Nodo->fd_writer, tmp, strlen(tmp) + 1);
                        kill(Nodo->data, SIGUSR1);
                        //printf("Mi metto in read dal figlio %d sul canale %d\n", Nodo->data, Nodo->fd[0]);
                            
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

/*
    Funzione: mostra i dettagli del dispositivo 
    Sintassi lato utente:                          add <tipo>
    TODO Sintassi comunicata dalla centralina :    add <tipo> (centralina comunica a processo specifico)
*/
int cen_info(char **args, NodoPtr procList){




    return 1;
}