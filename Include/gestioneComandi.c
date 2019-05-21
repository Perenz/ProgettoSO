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


<<<<<<< HEAD
//////////NON MI COMPILA SE LI METTO IN COMANDI.C OIBO
void stampaDisp(info infoDisp);
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

=======
>>>>>>> 6cd491fab1c1814c63a2a08fb924be0cb1463c72


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
            }else{
                if(answer_tmp.considera == 1){
                    //Print da spostare
                    array_risposte[i] = answer_tmp;
                    int j;
                    for(j=0;j<array_risposte[i].profondita; j++){
                        printf("  ");  
                    }printf("|__"); 
                    stampaDisp(array_risposte[i].info_disp);
                    
                    i++;
                }
                if(answer_tmp.eliminato == 1){
                    removeNode(list, answer_tmp.pid);
                }
                //se non è un messaggio di terminazione significa che il figlio ha ancora risp da comunicare
                //nel caso dei dispositivi di interazione (o controllo senza figli) verrà mandato
                //1 messaggio contenente le informazioni e un successivo messaggio di terminazione

                //nel caso di dispositivi di controllo con figli continueranno a mandare messaggi
                //il primo messaggio sarà quello relativo al dispositivo di controllo in question
                //finchè tutti i figli non avranno mandato il messaggio di terminazione
                //continuerà a mandare risposte, quando tutti avranno mandato il messaggio di terminazione
                //egli manderà 1 messaggio di terminazione alla centralina (qui)

                //inserisco il messaggio nell'array e incremento l'indice
                
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
    return 1;
}
 


void stampaDisp(info infoDisp){
    if(strcmp(infoDisp.tipo, "Bulb") == 0){
        printf("%d Bulb %d %s time: %.2f \n", infoDisp.pid, infoDisp.id, infoDisp.stato, infoDisp.time);
    }else if(strcmp(infoDisp.tipo, "Hub") == 0){
        printf("%d Hub %d %s time: %.2f \n", infoDisp.pid, infoDisp.id, infoDisp.stato, infoDisp.time);
    }else if(strcmp(infoDisp.tipo, "Fridge") == 0){
        printf("%d Fridge %d %s time: %.2f  delay: %.2f  percentualeRiempimento: %d  temperatura: %d \n", infoDisp.pid, infoDisp.id, infoDisp.stato, infoDisp.time,
        infoDisp.delay, infoDisp.percentuale, infoDisp.temperatura);
    }else if(strcmp(infoDisp.tipo, "Window") == 0){
        printf("%d Window %d %s time: %.2f \n", infoDisp.pid, infoDisp.id, infoDisp.stato, infoDisp.time);
    }
}