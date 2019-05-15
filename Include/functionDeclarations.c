#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "gestioneComandi.c"
#include "addDevice.c"


//#include "gestioneComandi.c"
char* broadcast(NodoPtr procList, char** comando, char* comando_compatto);
char* broadcast_list(NodoPtr procList, char** comando, char* comando_compatto);

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
void getManualPid();

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
        //CREA NUOVO COMANDO INFOMANUALE 
        //TODO
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
    while(procList!=NULL){
            kill(procList->data, SIGQUIT);
            procList=procList->next;
    }
    while(dispList!=NULL){
            kill(dispList->data, SIGQUIT);
            dispList=dispList->next;
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
    //printList(dispList);     
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
    char* comando = malloc(10);
    sprintf(comando, "l");
    char* answer = malloc(1000);
    answer = broadcast_list(NodoS, NULL, comando);
    

    printf("%s", answer);

    
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
        sprintf(comando, "d %s", args[1]);
        printf("comando padre: %s\n", comando);

        char** u = malloc(100);
        char* answer = malloc(100);
        answer = broadcast(nodo, NULL, comando);

        if(strcmp(answer, "0")!=0){//ha trovato il dispositivo
            removeNode(dispList, atoi(answer));
        }else{//non ho trovato nessun dispositivo con quell'id
            printf("Nessun elemento ha questo id\n");
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
    //TODO cambiare controllo, potrei farlo nel dispositivo
    if(args[1]==NULL){                                          //primo argomento diverso da id (errori, bisogna verificar sia un numero)

        printf("Argomenti non validi\n");
        printf("Utilizzo: switch <id> <label> <pos>\n");
        printf("Comando 'device' per vedere la lista di quelli disponibili\n");
        return 1;
    }else{
        NodoPtr nodo = dispList;
        //Escludo la centralina dal while
        nodo = nodo->next;
        signal(SIGCONT, sign_cont_handler);
        char* comando = malloc(4 + strlen(args[1]) + strlen(args[2]) + strlen(args[3]));//7 per il comando, 4 per spazi di sep. e la terminazione
        //comando
        sprintf(comando, "s %s %s %s", args[1], args[2], args[3]);
        //printf("scrittura lato padre: %s\n", comando);

        char* answer = broadcast(nodo, NULL, comando);
        if(strcmp(answer, "0")!=0){//ha trovato il dispositivo
            printf("%s\n", answer);//stampiamo una qualche risposta daje
        }else{//non ho trovato nessun dispositivo con quell'id
            printf("Nessun elemento ha questo id o errore nel comando\n");        
        }          
        
        
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
    NodoPtr nodo = dispList;
    //Escludo la centralina dal while
    nodo = nodo->next;

    //TODOMARCELLO
    //da fare un secondo while o il metodo generale per la seconda lista
    signal(SIGCONT, sign_cont_handler);
    char* comando = malloc(5 + strlen(args[1]) + 3);//1 per il comando + lunghezza id (args[1]) + 3 per spazi e terminazione stringa
    //tipo di comando
    sprintf(comando, "info %s", args[1]);
    //printf("scrittura lato padre: %s\n", comando);
    char* answer = broadcast(nodo, NULL, comando); 

    if(strcmp(answer, "0")!=0){//ha trovato il dispositivo
        printf("%s\n", answer);
    }else{//non ho trovato nessun dispositivo con quell'id
        printf("Nessun elemento ha questo id\n");        
    }
    return 1; //esci che sennò va avanti    

}

int manualCen_info(char *arg, NodoPtr procList, NodoPtr dispList){
    int pidCercato;
    //TODO POSSIBILE UNIRE CON LIST DATO CHE è MOLTO SIMILE
    NodoPtr nodo = dispList;
    //Escludo la centralina dal while
    nodo = nodo->next;
    signal(SIGCONT, sign_cont_handler);
    char* tmp = malloc(2 + 10 + 3);//2 per il comando + lunghezza id (args[1]) + 3 per spazi e terminazione stringa
    //tipo di comando
    strcat(tmp,"im ");
    //id dispositivo da spegnere
    strcat(tmp, arg);
    //delimitatore 
    strcat(tmp, "\n");
    //printf("scrittura lato padre: %s\n", tmp);
    while(nodo != NULL){
        
        //scrivo il comando sulla pipe
        write(nodo->fd_writer, tmp, strlen(tmp));
        //mando un segnale al figlio così si risveglia e legge il contenuto della pipe
        kill(nodo->data, SIGUSR1);
        //printf("Mi metto in read dal figlio %d sul canale %d\n", Nodo->data, Nodo->fd[0]);
        //pause();
            
        //TODO gestione errori
        //leggo il pid del figlio così da poterlo togliere dalla lista di processi
        char* answer = malloc(30);
        int err = read(nodo->fd_reader,answer, 30);
        //pause();
        //TODO se il figlio ritorna 0 esso non è figlio e perciò non lo elimino dalla lista
        
        //printf("Lettura da pipe lato padre %d\n", ris);
        //Se la risposta tornata è diversa da "0" significa che è stato trovato il dispositivo con quell'id
        if(strcmp(answer, "0")!=0){
            pidCercato = atoi(answer);
            //Ritorno il pid
            //printf("PidCercato è %d", pidCercato);
            return pidCercato;
        }                  
        nodo = nodo->next;
    }        

    //Se scorrendo tutti i processi l'ID non è stato trovato ritorno -1
    return -1; //esci che sennò va avanti    

}

//Funzione per ottenre l'ID del dispositivo che è richiesto dal manuale 
//sul quale poi tramite info() si ottiene il PID che viene poi passato al manuale
void getManualPid(NodoPtr procList, NodoPtr dispList){
    //Se ci sono entrato significa che è arrivato sigusr2 da manuale
    char msg[20];
    char** args;
    int pidCercato;
    int fd;
    char *manCenFifo = "/tmp/manCenFifo";
    //Apro la fifo in lettura
    fd=open(manCenFifo, O_RDONLY);
    read(fd, msg, 20);
    //Chiudo lettura 
    close(fd);

    args = splitLine(msg);
    if(strcmp(args[0], "contpid")==0){
        //printf("Letto msg correto 'contpid'\n");
        //printf("Ho ricevuto il pid %s\n", args[1]);    

        //Tramite info(modificato) ricavo il pid corrispondente all'id passato
        pidCercato = manualCen_info(args[1], procList, dispList); 
        //printf("Questo è il pid cercato %d\n", pidCercato); 
    }
    //Comunico il pid cercato al manuale
    //Apro la pipe in scrittura
    fd=open(manCenFifo, O_WRONLY);
    sprintf(msg, "%d", pidCercato);
    int esito= write(fd, msg, strlen(msg)+1);
    //printf("Cen ha scritto a manuale con esito %d", esito);


    //printf("Sono nel getManualPid della centralina\n");
    //Chiudo la fifo in lettura
    
  
    return;
}
