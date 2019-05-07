#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <assert.h>
#include <sched.h>
//ho spostato i metodi getLine e splitLine in una nuova libreria 
//TODO tale verrà linkata nel gestore generale dei processi di interazione
#include "../Include/gestioneComandi.c"


int status;
//TODO : potrei usare un unico node
//File descriptor in cui il figlio legge e il padre scrive
int fd_read;
//File descriptor in cui il figlio scrive e il padre legge
int fd_write;
char tipo = 'b';
pid_t idPar;
pid_t id;

int device_handle_command(char **args);
char *builtin_func[]={
        "l",//list
        "s",//changeState
        "g",//getInfo
        "d" //delete
};

void sighandle_int(int sig) {
    if(sig==SIGQUIT)
        _exit(0);      
    //pause();
}

//SIGUSR1 usato per l'implementazione della lettura della pipe con il padre
void sighandle_usr1(int sig){
    if(sig == SIGUSR1){
        //proviamo a leggere
        //potrei passare anche la lunghezza del messaggio
        char str[CEN_BUFSIZE];
        read(fd_read, str, CEN_BUFSIZE);//uso 10 per intanto, vedi sopra poi
        //printf("\n\tLettura da pipe %s  \n", str);
        char** arg = splitLine(str);
        int errnum = device_handle_command(arg);
    }
}
int device_handle_command(char **args){

    //da fare come in functionDeclarations in file dispositivi
    if(strcmp(args[0],builtin_func[0])==0 || strcmp(args[0],builtin_func[2])==0){//list o getinfo
        return dev_getinfo(args);
    }else if(strcmp(args[0],builtin_func[1])==0){//changestate
        return dev_changestate(args);
    }else if(strcmp(args[0],builtin_func[3])==0){//delete
        return dev_delete(args);
    }
    return 1;
}


//COMANDO s <id> <label> <stato:1/0>
/*restituisco in pipe:
    0 se NON sono il dispositivo in cui ho modificato lo stato
    1 se sono il dispositivo in cui ho modificato lo stato
*/
int dev_changestate(char **args){
    int id_change = atoi(args[1]);
    printf("%d\n", id_change);
    if(id_change == id){//devo confrontare lo stato
        /* potremo scriver un messaggio del tipo: dispositivo <id> acceso / spento
        char* msg = malloc(10);//potrei fare il log10 dell'id per trovare il numero di cifre
        sprintf(msg, "d %d", id);//id inteso come pid
        
        int esito = write(fd_write, msg, strlen(msg)+1);
        
        per un'altra idea vedi functionDeclaration in metodo cen_switch
        */
        
        status = atoi(args[3]);
        printf("%s\n", args[3]);

        printf("Status dispositivo %d : %d\n", id, status);  
        printf("\033[1;32m"); //scrivo in verde 
        printf("\tNon sono felice e non sono triste. È questo il dilemma della mia vita: non so come definire il mio stato d’animo, mi manca sempre qualcosa.");
        printf("\033[0m\n"); //resetto per scriver in bianco

        char* msg = malloc(10);
        int esito = write(fd_write, "1\0", 2);
        kill(idPar,SIGCONT);
    }else{
        int esito = write(fd_write, "0\0", 2);//TODO
        printf("here\n\n");
        kill(idPar,SIGCONT);
    }
    //famo ritornare l'errore poi
    return 1;
}

//COMANDO i <tipo> <id> o
//        l
int dev_getinfo(char **args){
    if(args[0][0] == 'l'){//list
        char* str = malloc(30);
        sprintf(str, "Bulb %d", id);
        strcat(str ,(status==1?" accesa\n":" spenta\n"));

        //printf("Pipe su cui scrivo %d, pipe su cui leggo %d \n", fd_write, fd_read);

        int esito = write(fd_write, str, strlen(str)+1);
        kill(idPar,SIGCONT);
        //pause();
    }
    //famo ritornare l'errore poi
    return 1;
}

//COMANDO d <id>
/*restituisco in pipe:
    0 se NON sono il dispositivo da eliminare
    pid se sono il dispositivo da eliminare
*/
int dev_delete(char **args){
    //printf("id: %d\n",id);
    int id_delete = atoi(args[1]);
    char* msg = malloc(10);//potrei fare il log10 dell'id per trovare il numero di cifre

    if(id == id_delete){//guardo se il tipo e l'id coincidono
        //scrivo sulla pipe che sono io quello che deve essere ucciso e scrivo anche il mio pid, la centralina dovrà toglierlo dalla lista
        //TODO trovare un altro metodo
       
        sprintf(msg, "%d\0", id);//id inteso come pid
        printf("id in messaggio: %s\n",msg);
        int esito = write(fd_write, msg, strlen(msg));
        
        printf("\033[1;31m"); //scrivo in rosso 
        printf("\x1b[ \n\t«Dio mio, Dio mio, perché mi hai abbandonato?»\n");
        printf("\033[0m"); //resetto per scriver in bianco
       
        printf("Eliminazione avvenuta con successo\n\n");
        kill(idPar,SIGCONT);

        exit(0);

    }else{
        sprintf(msg, "%d\0", 0);
        int esito = write(fd_write, msg, strlen(msg));
        printf("Non eliminato dato che id non coincide\n");
        kill(idPar,SIGCONT);
    }
    //famo ritornare l'errore poi
    return 1;
}




void signhandle_quit(int sig){
    _exit(0);
}


int main(int argc, char *args[]){
    id = getpid(); // chiedo il mio pid
    idPar = getppid(); //chiedo il pid di mio padre

    //0 spenta
    //1 accesa
    status = 0; 

    //leggo args per prendere gli argomenti passati(puntatore al lato di scrittura della pipe)
    fd_read = atoi(args[1]);
    fd_write = atoi(args[2]);

    /*
    //Mando il msg sul fd e ne stampo l'esito
    char msg[50];
    sprintf(msg, "Bulb %d", id);
    //printf("Figlio ora scrive su canale %d\n", fd);
    strcat(msg ,(status==1?" accesa\n":" spenta\n"));
    int rtn = write(fd,msg,strlen(msg)+1);
    //printf("Esito invio %d\nmsg inviato: %s\n", rtn, msg);
    */
    
    signal(SIGINT, sighandle_int);
    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1

    printf("\nLampadina creata\n");
    printf("Pid: %d\nPid padre: %d\n", id, idPar);

    //Invio segnale al padre
    int ris = kill(idPar, SIGCONT); 

    //Child va in pausa
    
    
    while(1){
        //printf("Child va in pausa\n");
        pause();
    }

    //printf("Child ora termina\n");   
    exit(0);
}


