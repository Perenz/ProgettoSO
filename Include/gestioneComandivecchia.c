#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "../strutture/list.c"
#include "../Include/gestioneComandi.c"
#include "../Include/addDevice.c"

//voglio usare le funzioni definite in functionDeclaration urca
#define CEN_BUFSIZE 128



NodoPtr dispList; //lista dei dispositivi collegati all'hub
pid_t idPar;
pid_t pid;
int id;
int status; //1 acceso, 0 spento
//File descriptor in cui il figlio legge e il padre scrive
int fd_read;
//File descriptor in cui il figlio scrive e il padre legge
int fd_write;

int dev_list(cmd);
int dev_switch(cmd);
int dev_info(cmd);
int dev_delete(cmd);
int dev_link(cmd);

void get_info_string(char*);//TODO aggiungere timer
void set_info(char*);

//int dev_link(char** args);
int device_handle_command(cmd);
void h_sigstop_handler ( int sig ) ;
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int dev_add(char*, char*);


void signhandle_quit(int sig){
    signal(SIGQUIT, signhandle_quit);
    _exit(0);
}




char *builtin_command[5]={
    "l",//list
    "s",//switch
    "i",//getInfo
    "d", //delete
    "a" //addDevice    da cambiare scegliendo lettera corrispondente
};
int (*builtin_func_hub[]) (cmd comando) = {
        &dev_list,
        &dev_switch,
        &dev_info,
        &dev_delete,
        &dev_link,
};
int cont_numCommands(){
    return (sizeof(builtin_command)/ sizeof(char*));
}

void sign_cont_handler_hub(int sig){
    signal(SIGCONT, sign_cont_handler_hub);
    return;
}
void h_sigstop_handler ( int sig ) {
  printf("Never happens (%d)\n",sig);
}
//SIGUSR1 usato per l'implementazione della lettura della pipe con il padre
void sighandle_usr1(int sig){
    signal(SIGCONT, sign_cont_handler);
    signal(SIGUSR1, sighandle_usr1);
    if(sig == SIGUSR1){
        cmd comando;
        int err_signal;
        read(fd_read, &comando, sizeof(cmd));
        int errnum = device_handle_command(comando);
        //printf("Termino %d\n", id);
        err_signal = kill(idPar, SIGCONT);
        if(err_signal != 0)
            perror("errore in invio segnale");

        //printf("Termina in modo adeguato.\n");
        return;
    }
}

//USATO PER SVEGLIARE IL PROCESSO
void sighandle_usr2(int sig){
    signal(SIGUSR2, sighandle_usr2);
    if(sig == SIGUSR2){
        return;
    }
}

int device_handle_command(cmd comando){
    //da fare come in functionDeclarations in file dispositivi
    int i;
    for(i=0; i<cont_numCommands(); i++){
        char tmp = *builtin_command[i];
        if(comando.tipo_comando==tmp)
            return builtin_func_hub[i](comando);
    }
    return 1;
}
int rispondi(risp risposta_controllore, cmd comando){
    risposta_controllore.termina_comunicazione = 0;
    risposta_controllore.pid = pid;
    //vado io in controllo e mando le varie risposte al papi
    //attenz, buono che salto il primo
    broadcast_controllo(dispList->next, comando, idPar, fd_write, risposta_controllore);
    return 1;
}

int dev_list(cmd comando){

    risp risposta_controllore;
    risposta_controllore.id = id;
    risposta_controllore.considera = 1;
    comando.profondita++;
    char* info = malloc(ANSWER);
    get_info_string(info);
    strcpy(risposta_controllore.info, info);

    rispondi(risposta_controllore, comando);


    return 1;
}
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int dev_switch(cmd comando){
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    return 1;
}
int dev_info(cmd comando){
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    return 1;
}
int dev_delete(cmd comando){
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    risp risposta_controllore;
    comando.profondita++;
    if(comando.forzato == 1 || comando.id == id){//comando --all 
        risposta_controllore.id = id;
        risposta_controllore.considera = 1;
        risposta_controllore.eliminato = 1;
        risposta_controllore.pid = pid;
        comando.forzato = 1;//indico ai miei figli di eliminarsi
        
        char* info = malloc(ANSWER);
        get_info_string(info);
        strcpy(risposta_controllore.info, info);
        
        rispondi(risposta_controllore, comando);
        exit(0);
    }else{
        risposta_controllore.considera = 0;//non considerarmi, non sono stato eliminato
        risposta_controllore.eliminato = 0;
        char* info = malloc(ANSWER);
        get_info_string(info);
        strcpy(risposta_controllore.info, info);

        rispondi(risposta_controllore, comando);
    }

    
    
    
    return 1;
}
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int dev_link(cmd comando){
    return 1;
}
void set_info(char* info){
    char** info_split = splitLine(info);
    //<infoDefault> := default <id>
    if(strcmp(info_split[0], "default")==0){
        id = atoi(info_split[1]);
    }else{
        //<info> := <id>
        id = atoi(info_split[0]);
    }
}

void get_info_string(char* ans){//TODO aggiungere timer
    //TODO 
    sprintf(ans, "hub %d %d", pid, id);
}


int main(int argc, char **args){
    dispList = listInit(getpid());

    //UGUALE A BULB 
    pid = getpid(); // chiedo il mio pid
    idPar = getppid(); //chiedo il pid di mio padre

    fd_read = atoi(args[1]);
    fd_write = atoi(args[2]);
    //MANCA IL SET_INFO, sbaglia l'id
    set_info(args[3]);

    signal(SIGCONT, sign_cont_handler_hub);//Segnale per riprendere il controllo 
    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1
    signal(SIGSTOP,h_sigstop_handler);

    printf("\nHub creato: id: %d\n", id);
    printf("Id: %d\n", id);
    printf("Pid: %d\nPid padre: %d\n", pid, idPar);
    int i=0;

    //AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

    //PROVA
    /*
    
    add_device_generale("./componenti/BULB", dispList, info);
    sprintf(info, "default %d", id+1);
    add_device_generale("./componenti/BULB", dispList, info);
    sprintf(info, "default %d", id+1);
    add_device_generale("./componenti/HUB", dispList, info);
    */
   char* info = malloc(ANSWER);
    sprintf(info, "default %d", id+1);
    while(i++ < 2 && id==3){
        sprintf(info, "%d", id+1);
        add_device_generale("./componenti/HUB", dispList, info);
        sprintf(info, "default %d", id+1);
        add_device_generale("./componenti/BULB", dispList, info);
    }
    i  = 0;
     while(i++ < 10 && id==4){
         sprintf(info, "default %d", id+1);
        add_device_generale("./componenti/BULB", dispList, info);
        sprintf(info, "%d", id+1);
        add_device_generale("./componenti/HUB", dispList, info);
        
    }
    
   
    
    if(id == 4){
        char* info = malloc(ANSWER);
        sprintf(info, "%d", id+2);
        add_device_generale("./componenti/BULB", dispList, info);
        sprintf(info, "%d", id+3);
        sprintf(info, "default %d", id+1);
        add_device_generale("./componenti/BULB", dispList, info);
        add_device_generale("./componenti/BULB", dispList, info);
        sprintf(info, "%d", id+3);
        sprintf(info, "default %d", id+1);
        add_device_generale("./componenti/BULB", dispList, info);
        add_device_generale("./componenti/BULB", dispList, info);
        sprintf(info, "%d", id+3);
        sprintf(info, "default %d", id+1);
        add_device_generale("./componenti/BULB", dispList, info);
        add_device_generale("./componenti/BULB", dispList, info);
        sprintf(info, "%d", id+3);
        sprintf(info, "default %d", id+1);
        add_device_generale("./componenti/BULB", dispList, info);
        
    }
    

    

       
    
    

    

    //Invio segnale al padre
    int ris = kill(idPar, SIGCONT);

    //Child va in pausa
    while(1){
        //printf("Sono in pausa\n");
        pause();
    }

    printf("Child ora termina\n");   
    exit(0);
}
//i vari parametri potrebbero essere levati se messo in hub
int broadcast_controllo(NodoPtr list, cmd comando, int pid_papi, int fd_papi, risp risposta_to_padre){
    signal(SIGCONT, sign_cont_handler_hub);//Segnale per riprendere il controllo 
    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1
    signal(SIGSTOP,h_sigstop_handler);
    //nodo rappresenta il figlio, nell'hub passo il successivo dato che il primo nodo 
    //è sè stesso
    NodoPtr nodo = list;
    //risposta che verrà mandata al padre (se ho figli)
    risp answer;
    int err_signal;//errore kill
    //debug printf("Mando il messaggio di me stesso alla centralina %d\n", id);
    //scrivo al padre la risposta del dispositivo di controllo contenente le sue info
    //il padre potrebbe essere un dispositivo diverso dalla centralina
    write(fd_papi, &risposta_to_padre, sizeof(risp));
    //mando un segnale al padre per comunicargli di leggere dalla pipe
    
    //err_signal = kill(pid_papi, SIGCONT);
    if(err_signal != 0)
        perror("errore in invio segnale");
    //GESTIONE SE L'HUB è DA ELIMINARE
    /*
    //da eliminare quando finisco con i figli
    if(risposta_to_padre.eliminato == 1){
        to_delete = 1;
        risposta_to_padre.eliminato = 0;
    }
    */
    //finchè ho figli
    while(nodo != NULL){
        //imposto il read dalla pipe come non bloccante
        /*
        int flags = fcntl(nodo->fd_reader, F_GETFL, 0);
        fcntl(nodo->fd_reader, F_SETFL, flags | O_NONBLOCK);
        */
        //Mando il comando a mio figlio che lo gestirà
        write(nodo->fd_writer, &comando, sizeof(comando));
        err_signal = kill(nodo->data, SIGUSR1); 
        //Mando un segnale per comunicare a mio figlio di gestire il comando
        if(err_signal != 0)
            perror("errore in invio segnale");
        while(1){
            //Leggo la risposta --> viene letta dopo che mi è arrivato un segnale SIGCONT
            //dato che è bloccante
            read(nodo->fd_reader, &answer, sizeof(risp));
                //debug printf("Leggo la risposta in %d, %s\n", id, answer.info);
            //se è un messaggio di terminazione devo uscire dal ciclo di ascolto e andare 
            //al nodo successivo
            if(answer.termina_comunicazione == 1){
                break;
            }else{
                
                //se non è un messaggio di terminazione significa che il figlio ha ancora risp da comunicare
                //nel caso dei dispositivi di interazione (o controllo senza figli) verrà mandato
                //1 messaggio contenente le informazioni e un successivo messaggio di terminazione

                //finchè tutti i figli non avranno mandato il messaggio di terminazione
                //continuerà a mandare risposte, quando tutti avranno mandato il messaggio di terminazione
                //egli manderà 1 messaggio di terminazione alla centralina (qui)

                //scrivo a mio padre la risposta che ho appena letto
                write(fd_papi, &answer, sizeof(risp));
                //err_signal = kill(pid_papi, SIGCONT); 
                if(err_signal != 0)
                    perror("errore in invio segnale");
                //Mi metto in pausa per permettere a mio padre di leggere la risposta
                //sono sicuro che non rimango in pausa dato che ho verificato che 
                //answer.terminazione fosse diverso da 1
                //kill(getpid(), SIGSTOP);
                //pause();
                //mando un segnale al figlio per comunicare di continuare la comunicazione
                err_signal = kill(nodo->data, SIGCONT); 
                if(err_signal != 0)
                    perror("errore in invio segnale");
                //mi metto in pausa così da permettere al figlio di comunicare: 
                    //messaggio di terminazione o
                    //ulteriore risposta con terminazione = 0 
                
            }
        }
        nodo = nodo->next;
    }
    //comunico al padre di aver finito di comunicare mettendo il parametro termina comunicazione = 1
    risposta_to_padre.termina_comunicazione = 1;
    write(fd_papi, &risposta_to_padre,sizeof(risp));
    //do il controllo a mio padre quando ritorno nel signal
    return 1;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include "../strutture/listH.h"
#include "../strutture/comandiH.h"
#include "../strutture/comandinonvacazz.c"


#define CEN_DELIM " \t\r\n\a"
#define CEN_BUFSIZE 128
#define BUFSIZE 128
#define ANSWER 64


//////////NON MI COMPILA SE LI METTO IN COMANDI.C OIBO

void initArray(array_risposte *a, size_t initialSize) {
    a->array = (risp*)malloc(initialSize * sizeof(risp));
    a->used = 0;
    a->size = initialSize;
}

void insertArray(array_risposte *a, risp element) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = (risp *)realloc(a->array, a->size * sizeof(risp));
    }
    a->array[a->used++] = element;
}

void freeArray(array_risposte *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}



//char* getLine();
int broadcast_controllo(NodoPtr list, cmd comando, int pid_papi, int fd_papi, risp risposta_to_padre);
//char** splitLine(char* line);

int broadcast_centralina(NodoPtr list, cmd comando, array_risposte* answer);
char* getLine(){
    char *cmd=NULL;
    //Dimensione buffer per riallocazione
    size_t  bufS = 0;
    getline(&cmd, &bufS, stdin);
    return cmd;
}

//Restituita una stringa la funzione la splitta in
//diverse stringhe secondo i delimitatori specificati
//nella macro CEN_DELIM
char** splitLine(char* line){
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
void sign_cont_handler(int sig){
    signal(SIGCONT, sign_cont_handler);
    //printf("Arrivato segnale\n");
    return;
}
int broadcast_centralina(NodoPtr list, cmd comando, array_risposte* answertoltoperora){
    //Setto il gestore di SIGCONT, l'ho giò settato ma per sicurezza lo risetto
    signal(SIGCONT, sign_cont_handler);
    int err_signal;//errore kill
    //Salto il primo nodo della lista dato che appartiene alla centralina
    NodoPtr nodo = list->next;
    //TODO restabilire l'array dinamico di risposte
    risp answer_tmp;//Risposta che verrà inserita in un array di risposte
    //Imposto a zero la terminazione della comunicazione che fa continuare il ciclio di comunicazione con i figli
    answer_tmp.termina_comunicazione = 0;

    risp array_risposte[1000];//array statico di risposte PROVA
    int i = 0;//indice array statico delle risposte PROVA


    //Finchè ho figli prova ad instaurare la comunicazione
    while(nodo != NULL){
        //Setto la lettura in pipe come non bloccante
        /*
        int flags = fcntl(nodo->fd_reader, F_GETFL, 0);
        fcntl(nodo->fd_reader, F_SETFL, flags | O_NONBLOCK);
        */

        //mando il comando al figlio nodo 
        write(nodo->fd_writer, &comando, sizeof(comando));
        //Comunico al figlio di gestire il comando appena inviato
        err_signal = kill(nodo->data, SIGUSR1); 
        if(err_signal != 0)
            perror("errore in invio segnale");

        //Finchè il figlio non mi manda un messaggio con terminazione == 1 significa che 
        //ha ancora dei figli e perciò devo rimanere in ascolto e gestire le sue risposte
        while(1){
            //Leggo la risposta --> viene letta dopo che mi è arrivato un segnale SIGCONT
            //dato che la read è bloccante
            //debug printf("\tMi metto in ascolto in centralina\n");
            read(nodo->fd_reader, &answer_tmp, sizeof(risp));
            //se è un messaggio di terminazione devo uscire dal ciclo di ascolto e andare 
            //al nodo successivo
            if(answer_tmp.termina_comunicazione == 1){
                //debug printf("Il messaggio è di terminazione\n");

                break;
            }else{//se non è un messaggio di terminazione significa che il figlio ha ancora risp da comunicare
                //nel caso dei dispositivi di interazione (o controllo senza figli) verrà mandato
                //1 messaggio contenente le informazioni e un successivo messaggio di terminazione

                //nel caso di dispositivi di controllo con figli continueranno a mandare messaggi
                //il primo messaggio sarà quello relativo al dispositivo di controllo in question
                //finchè tutti i figli non avranno mandato il messaggio di terminazione
                //continuerà a mandare risposte, quando tutti avranno mandato il messaggio di terminazione
                //egli manderà 1 messaggio di terminazione alla centralina (qui)

                //inserisco il messaggio nell'array e incremento l'indice
                array_risposte[i] = answer_tmp;
                printf("%s\n", array_risposte[i].info);
                i++;
                //printf("\t\t\t%s\n", answer_tmp.info);

                //mando un segnale al figlio per comunicare di continuare la comunicazione dato 
                //che ho letto la sua risposta
                err_signal = kill(nodo->data, SIGCONT); 
                if(err_signal != 0)
                    perror("errore in invio segnale");
                //andrò in pausa perché all'inizio del while c'è la read bloccante
            }
        }
        nodo = nodo->next;
    }
    printf("Numero dispositivi: %d\n", i);
    return 1;
}






