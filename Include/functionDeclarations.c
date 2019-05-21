#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "../strutture/comandiH.h"
#include "gestioneComandi.c"
#include "addDevice.c"
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
int cen_link(char **args, NodoPtr procList, NodoPtr dispList);
int cen_power(char **args, NodoPtr procList, NodoPtr dispList);
void getManualPid();

int cen_numCommands();
int checkPower();

void printRisp(risp* array_risposte){
    int i = 0;
    //Per ora lo famo così
    while(array_risposte[i].id != -1){
        //stampa nulla, marcello
        printf("%d\n", i);
    }
}



//Variabile intera che memorizza lo stato di accensione della centralina
//1 accesa
//0 spenta
int powerOn=1;

//l'id della centralina è uguale a 2 --> potremo generalizzare e fare che in start.c creo il processo cmd che comunica con la centralina tramite pipe il
//comando che deve essere eseguito --> centralina ha id = 2 o 1, è più fico id == 1

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
        "info",
        "link",
        "power"
        //CREA NUOVO COMANDO INFOMANUALE 
        //TODO
};

//Puntatore alle funzioni associati alla lista dei comandi builtin_cmd
int (*builtin_func[]) (char **, NodoPtr, NodoPtr) = {
        &cen_prova,
        &cen_clear,
        &cen_help,
        &cen_exit,
        &cen_add,
        &cen_list,
        &cen_delete,
        &cen_switch,
        &cen_info,
        &cen_link,
        &cen_power
};
int cen_numCommands(){
    return (sizeof(builtin_cmd)/ sizeof(char*));
}


int cen_prova(char **args, NodoPtr procList, NodoPtr dispList){
    printf("Hai inserito il comando %s\n", args[0]);
    return 1;
}
int cen_clear(char **args, NodoPtr procList, NodoPtr dispList){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    system("clear");
    return 1;
}
int cen_exit(char **args, NodoPtr procList, NodoPtr dispList){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    signal(SIGQUIT, SIG_IGN);
    while(procList!=NULL){
            kill(procList->data, SIGQUIT);
            procList=procList->next;
    }
    while(dispList!=NULL){
            kill(dispList->data, SIGQUIT);
            dispList=dispList->next;
    }
    //Dealloco le liste
    freeList(procList);
    freeList(dispList);

    //Rimuovo la fifo usata dal manuale per ottenere il pid dei dispositivi
    remove("/tmp/manCenFifo");

    return 0;
}
int cen_help(char **args, NodoPtr procList, NodoPtr dispList){
    printf("Progetto SO realizzato da: Paolo Tasin, "
           "Stefano Perenzoni, Marcello Rigotti\n");
    printf("Centralina per controllo domotico\n");
    printf("Digitare i seguenti comandi:\n");

    int i;
    for(i=0; i<cen_numCommands();i++){
        printf("> %s\n", builtin_cmd[i]);
    }
    return 1;
}





/*
    Funzione: elenca tutti i dispositivi con <nome>, quelli attivi con <nome> <id> 
    Sintassi lato utente: list
    Sintassi comunicata dalla centralina ai figli: l
*/
int cen_list(char **args, NodoPtr procList, NodoPtr dispList){ 
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    signal(SIGCONT, sign_cont_handler);

    //RICEZIONE RISPOSTE
    risp* array_risposte_proc_list = malloc(1000 * sizeof(risp));
    risp* array_risposte_disp_list = malloc(1000 * sizeof(risp));

    cmd comando;
    comando.tipo_comando = 'l';
    comando.profondita = 0;
    int err = 0;
    printf("\n\tStampo la lista dei dispositivi COLLEGATI:\n");
    printf("\nCENTRALINA VAGINA\n");
    err = broadcast_centralina(procList, comando, array_risposte_proc_list);  
    //printRisp(array_risposte_proc_list);


    printf("\n\tStampo la lista dei dispositivi DISPONIBILI:\n");
    //gestione eerr
    err = broadcast_centralina(dispList, comando, array_risposte_disp_list);
    
    printRisp(array_risposte_disp_list);


    free(array_risposte_proc_list);
    free(array_risposte_disp_list);
    return 1;
}
/*
    Funzione: rimuove il dispositivo <id>, se di controllo elimina anche i dispositivi sottostanti
    Sintassi lato utente:                          delete <id>
    Sintassi comunicata dalla centralina ai figli: d <id>
*/
//delete funziona su dispList per ora
int cen_delete(char **args, NodoPtr procList, NodoPtr dispList){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    //TODO da modificare, pensavo che l'eliminazione avvenisse anche per tipo.
    if(args[1]==NULL /*|| atoi(args[1]) < 2*/){
        printf("Argomenti non validi\n");
        printf("Utilizzo: delete <id>\n");
        return 1;
    }else{
        //RICEZIONE RISPOSTE
        risp* array_risposte_proc_list = malloc(1000 * sizeof(risp));
        risp* array_risposte_disp_list = malloc(1000 * sizeof(risp));
        int err;
        signal(SIGCONT, sign_cont_handler);
        cmd comando;
        comando.tipo_comando = 'd';
        if(strcmp(args[1], "--all")==0){
            comando.forzato = 1;
        }else{
            comando.forzato = 0;
            comando.id = atoi(args[1]);
        }

        err = broadcast_centralina(dispList, comando, array_risposte_disp_list);
        //gestione err
        err = broadcast_centralina(procList, comando, array_risposte_proc_list);
        
        free(array_risposte_proc_list);
        free(array_risposte_disp_list);
    }
    /*GESTIONE ID non esistente
    printf("Device indicato non riconosciuto\n");
    printf("Utilizzo: delete <id>\n");
    */
    
    return 1;
}
/*
    Funzione: aggiunge un device al sistema e ne mostra i dettagli
    Sintassi lato utente:                          add <tipo>
    TODO Sintassi comunicata dalla centralina :    add <tipo> (centralina comunica a processo specifico)
*/
int cen_add(char **args, NodoPtr procList, NodoPtr dispList){
    char nome[20];
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    if(args[1]==NULL){
            printf("Argomenti non validi\n");
            printf("Utilizzo: add <device>\n");
            printf("Comando 'device' per vedere la lista di quelli disponibili\n");
            return 1;
    }
    //3 device disponibili: bulb, window, fridge
    else{
        id_gen+=1;
        if(args[2]==NULL){
            //Devo concatenare nome del device piu id 
            //Es: bulb3
            
            sprintf(nome, "%s%d", args[1], id_gen);
        }
        else{
            //strcpy(nome,args[2]);
            //strcpy(nome, "Prova");
            strcpy(nome, args[2]); 
        }
        int i;
        for(i=0; i<device_number(); i++){
            if(strcmp(args[1], builtin_device[i])==0)
                    return add_device(bultin_dev_path[i], procList, dispList, nome);
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
    //Cambiare sintassi <label> <stato> //da generalizzare AAAAAAèAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int cen_switch(char **args, NodoPtr procList, NodoPtr dispList){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    //TODO cambiare controllo, potrei farlo nel dispositivo
    if(args[1]==NULL || args[2]==NULL || args[3]==NULL){         //primo argomento diverso da id (errori, bisogna verificar sia un numero)
        printf("Argomenti non validi\n");
        printf("Utilizzo: switch <id> <label> <pos>\n");
        printf("Comando 'device' per vedere la lista di quelli disponibili\n");
        return 1;
    }else{
        /*
        signal(SIGCONT, sign_cont_handler);
        char* comando = malloc(4 + strlen(args[1]) + strlen(args[2]) + strlen(args[3]));//7 per il comando, 4 per spazi di sep. e la terminazione
        //comando
        sprintf(comando, "s %s %s", args[1], args[2]);
        strcat(comando,(isdigit(args[3])?(args[3]):(args[3])));
        //printf("scrittura lato padre: %s\n", comando);
        char* answer = broadcast(procList, NULL, comando);
        if(strcmp(answer, "0")!=0){//ha trovato il dispositivo
            printf("%s\n", answer);//stampiamo una qualche risposta daje
            free(comando);
            free(answer);
        }else{//non ho trovato nessun dispositivo con quell'id
            printf("Nessun elemento ha questo id o errore nel comando\n");    
        }          
        */
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
*/  //da generalizzare 
int cen_info(char **args, NodoPtr procList, NodoPtr dispList){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    risp* array_risposte_proc_list = malloc(1 * sizeof(risp));
    risp* array_risposte_disp_list = malloc(1 * sizeof(risp));

    signal(SIGCONT, sign_cont_handler);
    cmd comando;
    int err;
    comando.tipo_comando = 'i';
    comando.id = atoi(args[1]);
    err = broadcast_centralina(procList, comando, array_risposte_proc_list);
    err = broadcast_centralina(dispList, comando, array_risposte_disp_list);
    
    free(array_risposte_proc_list);
    free(array_risposte_disp_list);
    //gestione non c'è nessun dispositivo con questo id
    return 1;
}

/*
    Funzione: collega due dispositivi (uno deve essere di controllo) 
    Sintassi lato utente:                          link <id1> to <id2>
    TODO Sintassi comunicata dalla centralina : 
*///AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int cen_link(char** args, NodoPtr procList, NodoPtr dispList){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    if(args[1]==NULL || args[2]==NULL || args[3]==NULL )//gestione errori alla Carlina
        return -1;
    //2 casi da gestire: 
        //caso 1--> l'id2 è uguale a 2
            //a1) chiedo le info di id1 per vedere se si trova in dispList
                //aT) se si trova in dispList sposto id1 in procList
                //aF) else delete di id1 e add alla centralina in procList
        //caso 2-->else
            //chiedo le info di id1 per vedere se è dispositivo di controllo
            //chiedo le info di id2 per vedere se è un dispositivo di interazione collegabile (stesso tipo) ad id1
            //delete id1
            //link id1 to id2
    risp* array_risposte_proc_list = malloc(1000 * sizeof(risp));
    risp* array_risposte_disp_list = malloc(1000 * sizeof(risp));

    signal(SIGCONT, sign_cont_handler);
    cmd comando;
    int err;
    comando.tipo_comando = 'i';
    comando.info_forzate = 1;
    comando.id = atoi(args[1]);
    err = broadcast_centralina(procList, comando, array_risposte_proc_list);
    err = broadcast_centralina(dispList, comando, array_risposte_disp_list);

    
    
    free(array_risposte_proc_list);
    free(array_risposte_disp_list);
    return 1;
}



int manualCen_info(char *arg, NodoPtr procList, NodoPtr dispList){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
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
    free(tmp);
    //Se scorrendo tutti i processi l'ID non è stato trovato ritorno -1
    return -1; //esci che sennò va avanti    

}

//Per gestire l'accensione/spegnimento generale della centralina
int cen_power(char **args, NodoPtr procList, NodoPtr dispList){
    NodoPtr nodo;
    if(powerOn==1){ //è acessa allora la spengo
        //TODO
        //Devo mandare in pausa tutti i dispositivi
        //Su entrambe le liste
    
        nodo=procList;
        while(nodo!=NULL){
            kill(SIGINT, nodo->data);
            nodo=nodo->next;
        }
        nodo=dispList;
        while(nodo!=NULL){
            kill(SIGINT, nodo->data);
            nodo=nodo->next;
        }
        powerOn=0;
        printf("Centralina spenta\n");
    }
    else if(powerOn==0){ //è spenta allora la accendo
        //TODO
        //Slocco dalla puasa tutti i dispositivi 
        //Su entrambe le liste
        nodo=procList;
        while(nodo!=NULL){
            kill(SIGCONT, nodo->data);
            nodo=nodo->next;
        }
        nodo=dispList;
        while(nodo!=NULL){
            kill(SIGCONT, nodo->data);
            nodo=nodo->next;
        }
        powerOn=1;
        printf("Centralina accesa\n");
    }
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

        //CAMBIA SOLO QUI
        //CON il nuovo info devo prendere pidCercato
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
    free(args);
  
    return;
}
