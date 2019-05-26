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
#define N_MAX_DISP 1024

#define CEN_DELIM " \t\r\n\a"
#define CEN_BUFSIZE 128
#define BUFSIZE 128
#define ANSWER 64

int broadcast_centralina(NodoPtr list, cmd comando, risp* array_risposte);
void printRisp(risp* array_risposte, int n, int indentazione);
void alloc_array(risp** array_risposte, int n);
void stampaRisp(risp);
void printColorato(char* string, char* color);

void sign_cont_handler(int sig){
    return;
}

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

int broadcast_centralina(NodoPtr list, cmd comando, risp* array_risposte){
    //Setto il gestore di SIGCONT, l'ho giò settato ma per sicurezza lo risetto
    //signal(SIGCONT, sign_cont_handler);
    int err_signal;//errore kill
    //Salto il primo nodo della lista dato che rappresenta la centralina
    NodoPtr nodo = list->next;
    risp answer_tmp;//Risposta che verrà inserita in un array di risposte
    //Imposto a zero la terminazione della comunicazione che fa continuare il ciclio di comunicazione con i figli
    answer_tmp.termina_comunicazione = 0;
    comando.profondita = 0;
    int i = 0;//indice array statico delle risposte PROVA

    //Finchè ho figli prova ad instaurare la comunicazione
    while(nodo != NULL){

        //mando il comando al figlio nodo 
        comando.id_padre = 0;
        write(nodo->fd_writer, &comando, sizeof(comando));
        //Comunico al figlio di gestire il comando appena inviato
        err_signal = kill(nodo->data, SIGUSR1); 
        if(err_signal != 0){
            perror("errore in invio segnale");
        }

        //Finchè il figlio non mi manda un messaggio con terminazione == 1 significa che 
        //ha ancora dei figli e perciò devo rimanere in ascolto e gestire le sue risposte
        while(1){
            //Leggo la risposta --> viene letta dopo che mi è arrivato un segnale SIGCONT
            //dato che la read è bloccante
            read(nodo->fd_reader, &answer_tmp, sizeof(risp));
            //se è un messaggio di terminazione devo uscire dal ciclo di ascolto e andare 
            //al nodo successivo
            if(answer_tmp.termina_comunicazione == 1){
                break;
            }else{
                //se non è un messaggio di terminazione significa che il figlio ha ancora risp da comunicare
                //nel caso dei dispositivi di interazione (o controllo senza figli) verrà mandato
                //1 messaggio contenente le informazioni e un successivo messaggio di terminazione

                //nel caso di dispositivi di controllo con figli continueranno a mandare messaggi
                //il primo messaggio sarà quello relativo al dispositivo di controllo in question
                //finchè tutti i figli non avranno mandato il messaggio di terminazione
                //continuerà a mandare risposte, quando tutti avranno mandato il messaggio di terminazione
                //egli manderà 1 messaggio di terminazione alla centralina (qui)

                //inserisco il messaggio nell'array e incremento l'indice
                if(answer_tmp.considera == 1){
                    array_risposte[i] = answer_tmp;
                    i++;
                }
                //mando un segnale al figlio per comunicare di continuare la comunicazione dato 
                //che ho letto la sua risposta
                err_signal = kill(nodo->data, SIGCONT); 
                if(err_signal != 0){
                    perror("errore in invio segnale");
                }
                //nel caso il comando sia quello di eliminare un dispositivo verrà settato un flag nella risposta in modo da eliminarlo dalla
                //lista nella centralina
                if(answer_tmp.eliminato == 1){
                    removeNode(list, answer_tmp.pid);
                }
                //andrò in pausa perché all'inizio del while c'è la read bloccante
            }
        }
        nodo = nodo->next;
    }
    //Rispondo con il numero di dispositivi inseriti
    return i;
}
int broadcast_controllo(NodoPtr list, cmd comando, info informazioni, int fd_papi, risp risposta_to_padre){
    //nodo rappresenta il figlio, nell'hub passo il successivo dato che il primo nodo 
    //è sè stesso
    NodoPtr nodo = list->next;
    //risposta che verrà mandata al padre (se ho figli)
    risp answer;
    int err_signal;//errore kill
    //scrivo al padre la risposta del dispositivo di controllo contenente le sue info
    //il padre potrebbe essere un dispositivo diverso dalla centralina ma comunque sarà in ascolto
    comando.profondita+=1;
    risposta_to_padre.profondita = comando.profondita;
    
    write(fd_papi, &risposta_to_padre, sizeof(risp));
    comando.id_padre = informazioni.id;
    
    //finchè ho figli
    while(nodo != NULL){
        //Mando il comando a mio figlio che lo gestirà
        write(nodo->fd_writer, &comando, sizeof(comando));
        int pid_figlio = nodo->data;
        err_signal = kill(nodo->data, SIGUSR1);
        //Mando un segnale per comunicare a mio figlio di gestire il comando
        if(err_signal != 0)
            perror("errore in invio segnale");
        while(1){
            //Leggo la risposta --> viene letta dopo che mi è arrivato un segnale SIGCONT
            //dato che è bloccante
            read(nodo->fd_reader, &answer, sizeof(risp));
            //se è un messaggio di terminazione devo uscire dal ciclo di ascolto e andare 
            //al nodo successivo
            if(answer.termina_comunicazione == 1){
                break;
            }else{
                //se non è un messaggio di terminazione significa che il figlio ha ancora risp da comunicare
                //nel caso dei dispositivi di interazione (o controllo senza figli) verrà mandato
                //1 messaggio contenente le informazioni e un successivo messaggio di terminazione
                
                //finchè tutti i figli non avranno mandato il messaggio di terminazione
                //continuerà a mandare risposte in su nell'albero verso la centralina, quando tutti avranno mandato il messaggio di terminazione
                //egli manderà 1 messaggio di terminazione al padre
                //scrivo a mio padre la risposta che ho appena letto
                if(answer.considera==1 ){
                    write(fd_papi, &answer, sizeof(risp));
                }
                
                //mando un segnale al figlio per comunicare di continuare la comunicazione                
                err_signal = kill(pid_figlio, SIGCONT);
                if(answer.eliminato == 1){//questo vale quando risalgo, se il dispositivo è da eliminare lo tolgo dalla lista dei processi
                    removeNode(list, answer.pid);
                    answer.eliminato = 0;//setto a 0 sennò lo toglie anche il padre che non lo ha nella lista 
                }
                if(err_signal != 0)
                    perror("errore in invio segnale");
                //messaggio di terminazione o
                //ulteriore risposta con terminazione = 0 
            }
        }
        nodo = nodo->next;
    }
    //comunico al padre di aver finito di comunicare mettendo il parametro termina_comunicazione = 1
    risposta_to_padre.termina_comunicazione = 1;
    write(fd_papi, &risposta_to_padre,sizeof(risp));
    return 1;
}

void stampaRisp(risp answer){
    if(strcmp(answer.info_disp.tipo, "bulb") == 0){
        printf("%d Bulb %d %s time: %.2f \n",answer.info_disp.pid,answer.info_disp.id, answer.info_disp.stato, answer.info_disp.time);
    }else if(strcmp(answer.info_disp.tipo, "fridge") == 0){
        printf("%d Fridge %d %s time: %.2f  delay: %.2f  percentualeRiempimento: %d  temperatura: %d \n",answer.info_disp.pid,answer.info_disp.id,answer.info_disp.stato,answer.info_disp.time,
       answer.info_disp.frigo.delay,answer.info_disp.frigo.percentuale,answer.info_disp.frigo.temperatura);
    }else if(strcmp(answer.info_disp.tipo, "window") == 0){
        printf("%d Window %d %s time: %.2f \n",answer.info_disp.pid,answer.info_disp.id,answer.info_disp.stato,answer.info_disp.time);
    }else if(strcmp(answer.info_disp.tipo, "timer") == 0){
        ////////////////da cambiare
        printf("%d Timer %d %s time: %.2f \n",answer.info_disp.pid,answer.info_disp.id,answer.info_disp.stato,answer.info_disp.time);//aggiungere override 1 / 0
    }else if(strcmp(answer.info_disp.tipo, "hub") == 0){        
        printf("%d Hub %d ",answer.info_disp.pid,answer.info_disp.id);

        if(answer.info_disp.lampadina.maxTime != -1){     
            printColorato("\n\t\tTempo max lampadine: ", "yellow" );
            printf("%.2f ",answer.info_disp.lampadina.maxTime);
            printColorato("\tInterruttore lampadine (in hub): ", "yellow" );
            printf("%s",answer.info_disp.lampadina.accensione.stato);
            if(answer.info_disp.lampadina.override_hub == '1')
                printColorato("\tOverrode ", "red" );
        }
        if(answer.info_disp.finestra.maxTime != -1){
            printColorato("\n\t\tTempo max finestre: ", "cyan" );
            printf("%.2f ",answer.info_disp.finestra.maxTime);
            printColorato("\tInterruttore finestre (in hub): ", "cyan" );
            printf("%s",answer.info_disp.finestra.apertura.stato);
            if(answer.info_disp.finestra.override_hub == '1')
                printColorato("\tOverrode ", "red" );
        }
        if(answer.info_disp.frigo.maxTime != -1){
            printColorato("\n\t\tTempo max frigoriferi: ", "green" );
            printf("%.2f ",answer.info_disp.frigo.maxTime);
            printColorato("\tInterruttore frigoriferi (in hub): ", "green" );
            printf("%s",answer.info_disp.frigo.apertura.stato);
            if(answer.info_disp.frigo.override_hub == '1')
                printColorato("\tOverrode ", "red" );
        }
        printf("\n");
    }
}
void printRisp(risp* array_risposte, int n, int indentazione){
    //Se indentazione = 1 indento la stampa
    int i = 0;
    for(; i < n; i++){
        if(indentazione == 1){
            int j = 0;
            for(; j<array_risposte[i].profondita; j++){
                printf("\t");
            }
        }
        stampaRisp(array_risposte[i]);
    }
}
void printColorato(char* string, char* color){
    if(strcmp(color, "red") == 0){
        printf("\033[0;31m");
    }else if(strcmp(color, "green") == 0){
        printf("\033[0;32m");
    }else if(strcmp(color, "cyan") == 0){
        printf("\033[0;36m");
    }else if(strcmp(color, "yellow") == 0){
        printf("\033[0;33m");
    }
    printf("%s", string);
    printf("\033[0m");
}

void malloc_array(risp** array_risposte, int n){
    *array_risposte = malloc(n * sizeof(risp));
    if(array_risposte == NULL)
        perror("Errore allocazione memoria");
}

void calloc_array(risp** array_risposte, int n){
    *array_risposte = calloc(0, n * sizeof(risp));
    if(array_risposte == NULL)
        perror("Errore allocazione memoria");
}

