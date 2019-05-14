#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "addDevice.c"
//#include "gestioneComandi.c"
char* broadcast(NodoPtr procList, char** comando, char* comando_compatto);

//Comandi centralina
int cen_prova(char **args, NodoPtr procList, NodoPtr dispList);
int cen_clear(char **args, NodoPtr procList, NodoPtr dispList);
int cen_exit(char **args, NodoPtr procList, NodoPtr dispList);
int cen_help(char **args, NodoPtr procList, NodoPtr dispList);
int cen_add(char **args, NodoPtr procList, NodoPtr dispList);
int cen_list(char **args, NodoPtr procList, NodoPtr dispList);
int cen_delete(char **args, NodoPtr procList, NodoPtr dispList);
int cen_switch(char **args, NodoPtr procList, NodoPtr dispList);
int cen_info(char **args, NodoPtr procList, NodoPtr dispList);

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

//Pointers list to a Function associated to each command
int (*builtin_func[]) (char **, NodoPtr, NodoPtr) = {
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
    NodoPtr tmp, tmp1;
    tmp = procList;
    tmp1 = dispList;
    while(tmp!=NULL){
            kill(tmp->data, SIGQUIT);
            tmp=tmp->next;
    }
    while(tmp1!=NULL){
            kill(tmp1->data, SIGQUIT);
            tmp1=tmp1->next;
    }

    //Dealloco la lista procList
    freeList(procList);
    freeList(dispList);

    return 0;
}

int cen_help(char **args, NodoPtr procList, NodoPtr dispList){
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
int cen_list(char **args, NodoPtr procList, NodoPtr dispList){   
    printList(dispList);     
    printf("Stampo la lista dei dispositivi tramite il loro pid:");
    //printList(procList);
    char tmp[30];
    NodoPtr Nodo = procList;
    NodoPtr NodoS = dispList->next; //escludo il -1 dalla lista
    //printList(Nodo);
    printf("\n\tCen %d accesa\n", Nodo->data);

    //Escludo la centralina dal while e la stampo SINGOLARMENTE Modifica Paolo --> centralina non l'aggiungo
    Nodo = Nodo->next;

    signal(SIGCONT, sign_cont_handler);

    //TODO possibile metterlo in funzione "broadcast"
    while(Nodo != NULL){
        //TODO gestire errori
        write(Nodo->fd_writer,"list\0",5);
        kill(Nodo->data, SIGUSR1);            
        //TODO
        //pause();
        int temp = read(Nodo->fd_reader,tmp,30);
        printf("\t%s", tmp);
        //memset(tmp,0,30);
        //strcat(msg,tmp);
        Nodo = Nodo->next;
    }
    printf("\n");
    printf("Stampo lista dei dispositivi non ancora collegati: \n");
    printList(dispList);
    while(NodoS != NULL){
        //TODO gestire errori
        write(NodoS->fd_writer,"list\0",5);
        kill(NodoS->data, SIGUSR1);            
        //TODO
        //pause();
        int temp = read(NodoS->fd_reader,tmp,40);
        printf("\t%s", tmp);
        //memset(tmp,0,30);
        //strcat(msg,tmp);
        NodoS = NodoS->next;
    }
    
    return 1;
}

/*
    Funzione: rimuove il dispositivo <id>, se di controllo elimina anche i dispositivi sottostanti
    Sintassi lato utente:                          delete <id>
    Sintassi comunicata dalla centralina ai figli: d <id>
*/
int cen_delete(char **args, NodoPtr procList, NodoPtr dispList){
    //TODO da modificare, pensavo che l'eliminazione avvenisse anche per tipo.
    if(args[1]==NULL){
        printf("Argomenti non validi\n");
        printf("Utilizzo: delete <id>\n");
        return 1;
    }else{  
        NodoPtr nodo = dispList;
        //escludo la centralina
        nodo = nodo->next;
        signal(SIGCONT, sign_cont_handler);
        //compongo il comando
        //passo NULL al posto che char** comando 
        //la composizione del comando verrà fatta da una funzione che riassembla il comando passato in args
        char* comando = malloc(6 + strlen(args[1]) + 3);//6 per il comando + lunghezza id (args[1]) + 2 per spazi e terminazione stringa
        
        //tipo di comando
        //problema: https://en.wikipedia.org/wiki/Joel_Spolsky#Schlemiel_the_Painter.27s_algorithm
        sprintf(comando, "delete %s", args[1]);
        printf("comando padre: %s\n", comando);

        char** u = malloc(100);
        char* answer = malloc(100);
        answer = broadcast(nodo, NULL, comando);
        if(strcmp(answer, "0")!=0){//ha trovato il dispositivo
            removeNode(dispList, atoi(answer));
        }
        //printf("%s\n", answer);
        
          
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
int cen_add(char **args, NodoPtr procList, NodoPtr dispList){
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
                    return add_device(bultin_dev_path[i], procList, dispList);
        }
    }

    printf("Device indicato non riconosciuto\n");
    printf("Utilizzo: add <device>\n");
    printf("Digitare il comando 'device' per la lista di quelli disponibili\n");

    return 1;        
}
/*
    Funzione: modifica <stato> di <label> del dispositivo con <id>
        (label sta per interruttore o termostato) //per ora realizzo solo interruttore
    Sintassi lato utente:                          switch <id> <label> <pos>
    Sintassi comunicata dalla centralina :         s <id> <label> <stato>
*/
//TODO
    //Cambiare sintassi <label> <stato>
int cen_switch(char **args, NodoPtr procList, NodoPtr dispList){
    //TODO cambiare controllo 
    if(args[1]==NULL                                            //primo argomento diverso da id (errori, bisogna verificar sia un numero)
        || (strcmp(args[2], "interruttore")!=0 && strcmp(args[2], "terminazione")!=0)//label
        || (strcmp(args[3], "1")!=0 && strcmp(args[3], "0")!=0)){//stato                                                                    

        printf("Argomenti non validi\n");
        printf("Utilizzo: switch <id> <label> <pos>\n");
        printf("Comando 'device' per vedere la lista di quelli disponibili\n");
        return 1;
    }else{
        NodoPtr nodo = procList;
        //Escludo la centralina dal while
        nodo = nodo->next;
        signal(SIGCONT, sign_cont_handler);
        char* comando = malloc(7 + strlen(args[1]) + strlen(args[2]) + 4);//7 per il comando, 4 per spazi di sep. e la terminazione
        
        char* label = malloc(2);//2 caratteri: label + terminazione
        //prendo solo l'iniziale del tipo per identificare il tipo di dispositivo
            //t per termostato
            //i per interruttore
        label[0] = args[2][0];
        label[1] = '\0';
        //comando
        sprintf(comando, "switch %s %s %s", args[1], args[2], args[3]);
        /*
        strcat(comando,"switch ");
        //id
        strcat(comando,args[1]);
        strcat(comando," ");
        //label: interruttore o termostato //per ora solo iterruttore
        strcat(comando, label);
        strcat(comando," ");
        //posizione accesa o spenta //per ora solo interruttore
        strcat(comando, args[3]);
        strcat(comando,"\0");
        */
        printf("scrittura lato padre: %s\n", comando);

        char* answer = broadcast(nodo, NULL, comando);
        printf("Risposta switch: %s", answer);            
        return 1; //esci che sennò va avanti
    
            
    }
    printf("Device indicato non riconosciuto\n");
    printf("Utilizzo: del <device> <num>\n");
    printf("Digitare il comando 'device' per la lista di quelli disponibili\n");
        
    return 1;
}

/*
    Funzione: mostra i dettagli del dispositivo 
    Sintassi lato utente:                          info <id>
    TODO Sintassi comunicata dalla centralina :    i <id> (centralina comunica a processo specifico)
*/
int cen_info(char **args, NodoPtr procList, NodoPtr dispList){
    //TODO POSSIBILE UNIRE CON LIST DATO CHE è MOLTO SIMILE
    NodoPtr nodo = procList;
    //Escludo la centralina dal while
    nodo = nodo->next;

    //TODOMARCELLO
    //da fare un secondo while o il metodo generale per la seconda lista
    signal(SIGCONT, sign_cont_handler);

    char* comando = malloc(5 + strlen(args[1]) + 3);//1 per il comando + lunghezza id (args[1]) + 3 per spazi e terminazione stringa
    //tipo di comando
    sprintf(comando, "info %s", args[1]);
    printf("scrittura lato padre: %s\n", comando);
    char* answer = broadcast(nodo, NULL, comando);    
    return 1; //esci che sennò va avanti    

}

/*
    Funzione: collega due dispositivi (uno deve essere di controllo) 
    Sintassi lato utente:                          link <id1> to <id2>
    TODO Sintassi comunicata dalla centralina : 
*/
int cen_link(char** args, NodoPtr procList){
    char* command_id1 = malloc(50);//TODO
    char* command_id2 = malloc(50);//TODO
    memset(command_id1, 0, 50);
    memset(command_id2, 0, 50);

    sprintf(command_id1, "info %s", args[1]);
    sprintf(command_id1, "info %s", args[3]);

    char* answer_id1 = broadcast(procList, NULL, command_id1);
    char* answer_id2 = broadcast(procList, NULL, command_id2);

    //Verifico che id2 sia un hub


    //Verifico che id1 esista


    memset(command_id1, 0, 20);
    sprintf(command_id1, "delete %s", args[1]);
    cen_delete(procList, NULL, command_id1);

    memset(command_id2, 0, 50);
    //SINTASSI "link <id1> <id2> <infoid2>"
    sprintf(command_id2, "link %s %s %s", args[1], args[2], args[3]);
}


