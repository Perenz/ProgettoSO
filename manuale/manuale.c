#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "./handFunctionDeclarations.c"


#define CEN_DELIM " \t\r\n\a"
#define CEN_BUFSIZE 128

int cen_processCmd(char **command);
char* cen_getLine();
char** cen_splitLine(char *line);
//Variabili che indica il pid del dispositivo su cui si sta agendo manualmente
//Di default uguale a 0, cioè non ho ancora effettuato il "collegamento" con alcun dispositivo
int controlloPid=0;
int controlloId=0;
int cenPid;

//Lettura del comando identica a quanto fatto per la centralina in start.c


//DUBBIO: Come ottengo il pid della centralina?:
//Si potrebbe aprire una FIFO nella centralina in readonly anche se non l'altro estremo (manuale.c) non è ancora presente (Ciò e permesso)
// https://stackoverflow.com/questions/48697193/how-to-wait-for-the-other-end-of-a-named-pipe-to-be-open
//Poi quando avvio manuale.c apro la fifo sia write che read, scrivo a centralina che poi potrà rispondere perchè ora c'è l'altro estremo
//Mi serve comunque il pid della centralina per mandare il signal che dice ci leggere dalla fifo


/*Cambio il processCmd in questo modo:
    - Se controllo == 0
        Significa che nessun dispositivo è al momento controllato quindi si dovrà effettuare il collegamento digitando il nome,id o pid del 
        dispositivo su cui vogliamo agire.
        Tramite la centralina (ricercaNellAlbero) torniamo il pid del dispositivo con tale nome/id cosi da poter istanziare una
        FIFO tra manuale.c ed il dispositivo interessato.
        A questo punto la variabile pid cambia

        controllo rimane assegnata fino a quando non si effettua una exit/quit dal dispositivo "attivo"

    - Se controllo != 0
        Significa che stiamo gia controllando un dispositivo e possiamo quindi impartire i vari comandi come:
            - switch ...
            - status ...
            - ...
            - quit/exit

*/

#define myFIFO "/tmp/miaFifo"

int getCenPid(){
    int fd;
    //char *fifo="./test/newFifo";
    char msg[20] = "hand";

    //La apro per write only
    fd = open(myFIFO, O_WRONLY);

    //Invio msg "hand" sulla fifo
    int esito = write(fd, msg, strlen(msg)+1);

    //Chiudo il Write only
    close(fd);

    //Apro in read
    fd = open(myFIFO, O_RDONLY);
    read(fd, msg, 10);

    close (fd);

    return atoi(msg);
}

void sigquit_handler(int sig){
    exit(0);
}

int main(){
    char *command;
    char **params;

    size_t bufS = 0;
    int status =1;

    signal(SIGQUIT, sigquit_handler);
    //Passo 1: Prendere il pid della centralina grazie ad il processo di support
    //Creo la fifo
    //TODO, provare a non fare il mkfifo e fare solo l'open utilizzando la fifo aperta dal READ
    mkfifo(myFIFO, 0666);

    //printf("Ecco il pid %d", getCenPid());
    cenPid=getCenPid();

    //Ora mi comporto diversamente a secondo del valore di controllo
    //in entrambi i casi prendo i comandi esattamente come faccio in start.c, cambia l'insieme di comandi disponibili
    //Quindi cambia solo il processCmd con una serie di funzioni bultin diverse per i due casi
    do{
        if(controlloPid==0)
            printf("Inserisci il comando:>");
        else
            printf("(Controllo su dispositivo pid %d e id %d) Inserisci il comando :>", controlloPid, controlloId);
        

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

    /*
     * Non più necessario come lo splitLine
     *
    //Rimuovo newLine \n a fine stringa
    cmd = strtok(cmd, "\n");
    */

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
    int i=0;
    //se inserisco un comando vuoto richiedo di inserire un nuovo comando
    if(command[0] == NULL)
        return 1;


    for(i; i<cen_numCommands(controlloPid); i++){
        if(controlloPid==0){
            if(strcmp(command[0],noControl_builtin_cmd[i])==0){
                controlloPid=noControl_builtin_func[i](command, &cenPid);
                if(controlloPid==-1){
                    controlloPid=0;
                    return 1;
                }else{
                    controlloId=atoi(command[1]);
                    return controlloPid;
                }
            }
        }
        else{
            if(strcmp(command[0],control_builtin_cmd[i])==0)
                return control_builtin_func[i](command, &controlloPid);
        }
    }

    //Se comando inserito non esiste
    printf("Comando non riconosciuto, digitare \"help\" per la lista\n");
    return 1;
}