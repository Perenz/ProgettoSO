#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include "../strutture/comandiH.h"
#include "./handFunctionDeclarations.c"

#define CEN_DELIM " \t\r\n\a"
#define CEN_BUFSIZE 128
#define myFIFO "/tmp/getPidCenFIFO"

int cen_processCmd(char **command);
char* cen_getLine();
char** cen_splitLine(char *line);
//Variabili che indicano il pid e l'id del dispositivo su cui si sta agendo manualmente
//Di default uguali a 0, cioè non ho ancora effettuato il "collegamento" con alcun dispositivo
int controlloPid;
int controlloId;
char controlloTipo;


//Variabile per memorizzare il pid della centralina
int cenPid;

/*Cambio la funzione processCmd processCmd in questo modo:
    - Se controllo == 0
        Significa che nessun dispositivo è al momento controllato quindi si dovrà effettuare il collegamento digitando l'id del 
        dispositivo su cui vogliamo agire.
        Tramite la centralina (ricercaNellAlbero) torniamo il pid del dispositivo con tale id cosi da poter istanziare una
        FIFO tra manuale.c ed il dispositivo interessato.
        A questo punto la variabile pid cambia

        controllo rimane assegnata fino a quando non si effettua una release dal dispositivo "attivo"

    - Se controllo != 0
        Significa che stiamo gia controllando un dispositivo e possiamo quindi impartire i vari comandi come:
            - switch ...
            - release
            - ...
            - quit/exit

*/



int getCenPid(){
    int fd;
    char msg[10] = "hand";

    //La apro per write only
    fd = open(myFIFO, O_WRONLY);
    
    //Gestisco l'errore
    if(fd<0){
        fprintf(stderr, "Errore nell'apertura della fifo %s: %s", myFIFO, strerror(errno));
    }

    //Invio msg "hand" sulla fifo
    if(write(fd, msg, strlen(msg)+1)<0){
        fprintf(stderr, "Errore nella scrittura su fifo %s: %s", myFIFO, strerror(errno));

    }

    //Chiudo il Write only
    close(fd);

    //Apro in read
    fd = open(myFIFO, O_RDONLY);
    if(fd<0){
        fprintf(stderr, "Errore nell'apertura della fifo %s: %s", myFIFO, strerror(errno));
    }

    
    if(read(fd, msg, 10)<0){
        fprintf(stderr, "Errore nella lettura da fifo %s: %s", myFIFO, strerror(errno));

    }

    close (fd);

    //Ritorno il pid della centralina
    return atoi(msg);
}

void sigquit_handler(int sig){
    exit(0);
}

void sigint_handler(int sig){
    printf("Programma interrotto a causa di un errore improvviso\n");
    exit(0);
}

int main(){
    char *command;
    char **params;

    size_t bufS = 0;
    int status =1;

    signal(SIGINT, sigint_handler);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGQUIT, sigquit_handler);
    //Passo 1: Prendere il pid della centralina grazie ad il processo di support

    //printf("Ecco il pid %d", getCenPid());
    cenPid=getCenPid();
    printf("Pid centralian %d\n", cenPid);

    //Ora mi comporto diversamente a secondo del valore di controllo
    //in entrambi i casi prendo i comandi esattamente come faccio in start.c, cambia l'insieme di comandi disponibili
    //Quindi cambia solo il processCmd con una serie di funzioni bultin diverse per i due casi
    do{
        printf("\033[0;34m"); //Set the text to the color blue
        if(controlloPid==0)
            printf("Inserisci il comando:>");
        else
            printf("(Controllo su: pid %d, id %d, tipo %c) Inserisci il comando :>", controlloPid, controlloId, controlloTipo);
        printf("\033[0m");

        //Prendo il comando dall'utente
        command = cen_getLine();

        //Splitta la linea in singoli parametri/argomenti
        params = cen_splitLine(command);

        //Esegue il comando
        status = cen_processCmd(params);
    }while(status);

    exit(0);
}

char* cen_getLine(){
    char *cmd=NULL;
    //Dimensione buffer per riallocazione
    size_t  bufS = 0;
    getline(&cmd, &bufS, stdin);
    return cmd;
}

//Restituita una stringa la funzione la splitta in
//diverse stringhe secondo i delimitatori specificati
//nella macro CEN_DELIM
char** cen_splitLine(char *line){
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


int cen_processCmd(char **command){
    //se inserisco un comando vuoto richiedo di inserire un nuovo comando
    if(command[0] == NULL)
        return 1;


    //In base alla situazione in cui mi trovo (controllo dispositivo o no) devo controllare su un numero possibilmente diverso di comandi
    int i;
    for(i=0; i<cen_numCommands(controlloPid); i++){
        if(controlloPid==0){
            //Controllo sui comandi disponibili quando non controllo alcun dispositivo
            if(strcmp(command[0],noControl_builtin_cmd[i])==0){
                controlloPid=noControl_builtin_func[i](command, &cenPid, &controlloTipo);
                if(controlloPid==-1){
                    //Il -1 indica "errore" e quindi resetto il Pid del dipositivo controllato e ritorno 1 per proseguire
                    controlloPid=0;
                    return 1;
                }else if (controlloPid==0)
                {
                    //Devo uscire
                    return 0;
                }
                {
                    //Pid trovato e diverso da -1
                    //Setto correttamente anche la variabile contenente l'id del dispositivo controllato
                    controlloId=atoi(command[1]);
                    inizializzaFifo(controlloPid);
                    return controlloPid;
                }
            }
        }
        else{
            //Controllo sui comandi disponibili quando controllo un dispositivo
            if(strcmp(command[0],control_builtin_cmd[i])==0)
                return control_builtin_func[i](command, &controlloPid, controlloId, controlloTipo);
        }
    }

    //Se comando inserito non esiste
    printf("Comando non riconosciuto, digitare \"help\" per la lista\n");
    return 1;
}