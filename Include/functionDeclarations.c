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
int cen_single_list(char **args, NodoPtr list); // per ora la metto qua, poi andrà in funzioniDispositiviControllati
int cen_delete_generale(char **args, NodoPtr list);

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
void getManualPid();

int cen_numCommands();

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
        "link"
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
        &cen_link
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
    //Dealloco le liste
    freeList(procList);
    freeList(dispList);
<<<<<<< HEAD

    //Rimuovo la fifo usata dal manuale per ottenere il pid dei dispositivi
    remove("/tmp/manCenFifo");

=======
>>>>>>> 1f929fd2238f7f248055a60100357dff46328433
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

int cen_list(char **args, NodoPtr procList, NodoPtr dispList){ 
    //control_list(args, una list);
    printf("\n\tStampo la lista dei dispositivi COLLEGATI:");
    printf("\n\tCen %d accesa\n", procList->data);
    //Escludo la centralina dal while e la stampo SINGOLARMENTE
    cen_list_generale(args, procList);
    printf("\n\tStampo la lista dei dispositivi DISPONIBILI:\n");
    cen_list_generale(args, dispList);
    return 1;
}
/*
    Funzione: elenca tutti i dispositivi con <nome>, quelli attivi con <nome> <id> 
    Sintassi lato utente: list
    Sintassi comunicata dalla centralina ai figli: l
*/
int cen_list_generale(char **args, NodoPtr list){ 
    signal(SIGCONT, sign_cont_handler);
    char* comando = malloc(10);
    sprintf(comando, "l");
    char* answer = malloc(1000);//DA STANDARIZZARE --> è molto grande perché raccoglie le info di tutti i dispositivi in una stringa
    answer = broadcast_list(list, NULL, comando);
    printf("%s", answer);
    free(answer);     
    return 1;
}

/*
    Funzione: rimuove il dispositivo <id>, se di controllo elimina anche i dispositivi sottostanti
    Sintassi lato utente:                          delete <id>
    Sintassi comunicata dalla centralina ai figli: d <id>
*/
//delete funziona su dispList per ora
int cen_delete(char **args, NodoPtr procList, NodoPtr dispList){
    //TODO da modificare, pensavo che l'eliminazione avvenisse anche per tipo.
    if(args[1]==NULL){
        printf("Argomenti non validi\n");
        printf("Utilizzo: delete <id>\n");
        return 1;
    }else{  
        //tolgo la centralina
        int err = cen_delete_generale(args, procList);
        //controllo errori
        err = cen_delete_generale(args, dispList);
        return 1;//TODO      
    }
    printf("Device indicato non riconosciuto\n");
    printf("Utilizzo: delete <id>\n");
    
    return 1;
}
int cen_delete_generale(char **args, NodoPtr list){
    signal(SIGCONT, sign_cont_handler);
    //compongo il comando
    char* comando = malloc(6 + strlen(args[1]) + 3);//6 per il comando + lunghezza id (args[1]) + 2 per spazi e terminazione stringa
    //tipo di comando
    sprintf(comando, "d %s", args[1]); 
    char* answer = broadcast(list, NULL, comando);
    if(strcmp(answer, "0")!=0){//ha trovato il dispositivo
        removeNode(list, atoi(answer));
        free(comando);
        free(answer);
    }else{//non ho trovato nessun dispositivo con quell'id
        return -1;
    }
    return 1; //esci che sennò va avanti     
}


/*
    Funzione: aggiunge un device al sistema e ne mostra i dettagli
    Sintassi lato utente:                          add <tipo>
    TODO Sintassi comunicata dalla centralina :    add <tipo> (centralina comunica a processo specifico)
*/
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
    //Cambiare sintassi <label> <stato> //da generalizzare
int cen_switch(char **args, NodoPtr procList, NodoPtr dispList){
    //TODO cambiare controllo, potrei farlo nel dispositivo
    if(args[1]==NULL || args[2]==NULL || args[3]==NULL){         //primo argomento diverso da id (errori, bisogna verificar sia un numero)
        printf("Argomenti non validi\n");
        printf("Utilizzo: switch <id> <label> <pos>\n");
        printf("Comando 'device' per vedere la lista di quelli disponibili\n");
        return 1;
    }else{
        signal(SIGCONT, sign_cont_handler);
        char* comando = malloc(4 + strlen(args[1]) + strlen(args[2]) + strlen(args[3]));//7 per il comando, 4 per spazi di sep. e la terminazione
        //comando
        sprintf(comando, "s %s %s %s", args[1], args[2], args[3]);
        //printf("scrittura lato padre: %s\n", comando);
        char* answer = broadcast(procList, NULL, comando);
        if(strcmp(answer, "0")!=0){//ha trovato il dispositivo
            printf("%s\n", answer);//stampiamo una qualche risposta daje
            free(comando);
            free(answer);
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
*/  //da generalizzare 
int cen_info(char **args, NodoPtr procList, NodoPtr dispList){
    int err = cen_info_generale(args, dispList);
    int err2 = cen_info_generale(args, procList);
    if(err == -1 && err2 == -1)
        printf("Nessun dispositivo con questo id trovato.\n");
    return 1;
}
int cen_info_generale(char **args, NodoPtr list){
    signal(SIGCONT, sign_cont_handler);
    if(args[1] != NULL){
        char* comando = malloc(5 + strlen(args[1]) + 3);//1 per il comando + lunghezza id (args[1]) + 3 per spazi e terminazione stringa
        //tipo di comando
        sprintf(comando, "i %s", args[1]);
        //printf("scrittura lato padre: %s\n", comando);
        char* answer = broadcast(list, NULL, comando); 

        if(strcmp(answer, "0")!=0){//ha trovato il dispositivo
            printf("%s\n", answer);
            free(comando);
            free(answer);
        }else{//non ho trovato nessun dispositivo con quell'id
            printf("Nessun elemento ha questo id\n");        
        }
    }else{
        printf("Inserire: info <id> \n");
    }
    
    return -1; //esci che sennò va avanti   

}

/*
    Funzione: collega due dispositivi (uno deve essere di controllo) 
    Sintassi lato utente:                          link <id1> to <id2>
    TODO Sintassi comunicata dalla centralina : 
*/
int cen_link(char** args, NodoPtr procList, NodoPtr dispList){
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

    char* command_id1;
    char* command_id2;
    char* answer_id1;
    char* answer_id2;
    command_id1 = malloc(50);//TODO
    answer_id1 = malloc(50);
    //caso 1

    NodoPtr node = dispList;
    if(atoi(args[3]) == 2){ //uguaglianza con id centralina
        
        
        char** splitted_answer_id1;
        memset(command_id1, 0, 50);
        sprintf(command_id1, "i %s", args[1]);
        
        
        char* answer_id1 = broadcast(node, NULL, command_id1);//mando a dispList, non a procList
        printf("%s\n", answer_id1);
        //verifico la condizione a1 (vedi sopra)
        if(strcmp(answer_id1, "0")!=0){//non mi interessa se esso è un hub o una lampadina, basta che esista
            splitted_answer_id1 = splitLine(answer_id1);
            int pid_id1 = atoi(splitted_answer_id1[1]);
            //aT) se si trova in dispList sposto id1 in procList
            Nodo nodo_rimuovere;
            int err = getNode(dispList, pid_id1, &nodo_rimuovere);
            if(err!= -1){//non c'è stato errore
                spostaNode(dispList, procList, nodo_rimuovere);
                printf("\n\n");
            }else{
                printf("Porco cazzo\n");
            }
        }else{//significa che l'id1 non esiste in dispList
            //devo provare con procList
            printf("sono qui\n");
            
            //memset(answer_id1, 0, 50);
            
            node = procList->next; //salto la centralina
            printList(node);
            char* answer_id1 = broadcast(node, NULL, command_id1);//mando a procList
            printf("%s\n", answer_id1);
            if(strcmp(answer_id1, "0")!=0){//esiste
                //int pid_id1 = atoi(splitted_answer_id1[1]);
                //voglio ogni funzione chiamabile con parametro node, non con doppio fottuto parametro
                //in info ho un bel malloppo, se è un dispositivo di interazione niente paura, mi basta un add device generale 
                //se è un dispositivo di controllo esso si porterà dietro altrettanti dispositivi che dobbiamo ricreare
                
                //Passo l'id ed elimino il dispositivo con tale id
                cen_delete(&args[0], node, NULL);
                //splitted_answer_id1 = splitLine(answer_id1);

                char* tmp = strtok(answer_id1, "\n");
                printf(": %s\n", tmp);
                char** tmp_mat = splitLine(tmp);
                
                for(int i=0; i<device_number(); i++){
                    if(strcmp(tmp_mat[0], builtin_device[i])==0)
                        return add_device_generale(bultin_dev_path[i], procList, tmp);
                }
                
                //add_device_generale(tmp, procList, dispList);
                 
            }else{
                printf("Dispositivo %s non esiste\n", args[1]);
            }
            
        }   
    }else{//il dispositivo id2 non è la centralina


    }

    /*
    command_id1 = malloc(50);//TODO
    command_id2 = malloc(50);//TODO
    memset(command_id1, 0, 50);
    memset(command_id2, 0, 50);

    sprintf(command_id1, "i %s", args[1]);
    sprintf(command_id1, "i %s", args[3]);

    answer_id1 = broadcast(procList, NULL, command_id1);
    answer_id2 = broadcast(procList, NULL, command_id2);

    //Verifico che id2 sia un hub


    //Verifico che id1 esista


    memset(command_id1, 0, 20);
    sprintf(command_id1, "d %s", args[1]);
    cen_delete(procList, NULL, command_id1);

    memset(command_id2, 0, 50);
    //SINTASSI "link <id1> <id2> <infoid1>" 
    sprintf(command_id2, "link %s %s : %s", args[1], args[2], args[3]);

    */
    return 1;
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
    free(tmp);
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
    free(args);
  
    return;
}
