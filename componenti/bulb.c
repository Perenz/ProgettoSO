#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <assert.h>
#include <sched.h>
#include <time.h>
//ho spostato i metodi getLine e splitLine in una nuova libreria 
//TODO tale verrà linkata nel gestore generale dei processi di interazione
#include "../Include/gestioneComandi.c"


int dev_getinfo(char **args);
int dev_delete(char **args);
int dev_switch(char **args);
int dev_list(char **args);
int dev_info(char **args);
void get_info_string(char* ans);
void set_time();

time_t tempoUltimaMisurazione;
double tempoSecondi;
int status;

//TODO : potrei usare un unico node
//File descriptor in cui il figlio legge e il padre scrive
int fd_read;
//File descriptor in cui il figlio scrive e il padre legge
int fd_write;
char tipo = 'b';
pid_t idPar;
pid_t pid;
int id;

int device_handle_command(char **args);
char *builtin_func[]={
        "info",//info
        "switch",//changeState
        "list",//list
        "delete" //delete
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
        memset(str, 0, CEN_BUFSIZE);
        read(fd_read, str, CEN_BUFSIZE);//uso 10 per intanto, vedi sopra poi
        //printf("\n\tLettura da pipe %s  \n", str);
        
        char** arg = splitLine(str);
        int errnum = device_handle_command(arg);
    }
}
int device_handle_command(char **args){
    //da fare come in functionDeclarations in file dispositivi
    if(strcmp(args[0],builtin_func[0])==0){//getinfo
        return dev_info(args);
    }else if(strcmp(args[0],builtin_func[1])==0){//changestate
        return dev_switch(args);
    }else if(strcmp(args[0],builtin_func[2])==0){//list
        
        return dev_list(args);   
    }else if(strcmp(args[0],builtin_func[3])==0){//delete
        return dev_delete(args);
    }
    return 1;
}
void signhandle_quit(int sig){
    _exit(0);
}

//COMANDO switch <id> <label> <stato:on/off>
/*restituisco in pipe:
    0 se NON sono il dispositivo in cui ho modificato lo stato
    1 se sono il dispositivo in cui ho modificato lo stato
*/
//MANCA CONTROLLO LABEL E STATO
int dev_switch(char **args){
    int id_change = atoi(args[1]);
    printf("%d", id_change);
    if(id_change == id && 
        strcmp(args[2], "accensione")==0 &&
        (strcmp(args[3], "on")==0 ||  strcmp(args[3], "off")==0)){
        
        set_time();
        char* answer = malloc(ANSWER);
        if(strcmp(args[3], "on")==0){
            status = 1;
        }else{//uguale a off dato dal controllo precedente
            status = 0;
        }
        //aggiorna il time prima di cambiare lo status
        get_info_string(answer);
            printf("%s", answer); //debug 
            printf("\033[1;32m"); //scrivo in verde 
            printf("\tNon sono felice e non sono triste. È questo il dilemma della mia vita: non so come definire il mio stato d’animo, mi manca sempre qualcosa.");
            printf("\033[0m\n"); //resetto per scriver in bianco



        int esito = write(fd_write, answer, ANSWER);
        kill(idPar,SIGCONT);
    }else{
        int esito = write(fd_write, "0\0", 2);//TODO
        kill(idPar,SIGCONT);
    }
    //famo ritornare l'errore poi
    return 1;
}

//COMANDO   l
/*restituisce in pipe
    se comando è l: <informazioni>
*/
int dev_list(char **args){
    char* ans = malloc(ANSWER);
    get_info_string(ans);
    int esito = write(fd_write, ans, strlen(ans));
    kill(idPar,SIGCONT);
    //pause();
    return 1;
}


//COMANDO   info <id>
/*restituisce in pipe
    <info> := <tipo> <pid???> <id> <status> <time>
*/
int dev_info(char **args){
    int id_info = atoi(args[1]);
    if(id == id_info){
        char* ans = malloc(ANSWER);
        get_info_string(ans);
        int esito = write(fd_write, ans, strlen(ans));
        kill(idPar,SIGCONT);
    }else{
        int esito = write(fd_write, "0\0", 2);
        //printf("Non restituisce info dato che id non coincide\n");
        kill(idPar,SIGCONT);
    }
    
    //famo ritornare l'errore poi
    return 1;
}
void set_time(){
     if(status==1){
        time_t tmp;
        time(&tmp);
        tempoSecondi += (difftime(tmp, tempoUltimaMisurazione));
        //la lampadina è accesa
        tempoUltimaMisurazione = tmp;
    }else{//utilizzato per gestire il caso di cambio di stato
        time(&tempoUltimaMisurazione);
    }
}
void get_info_string(char* ans){//TODO aggiungere timer
    set_time();
    memset(ans, 0, ANSWER);
    //<info> := <tipo> <pid???> <id> <status> <time>
    char* status_string = malloc(4);
    status_string = status==1? "on" : "off";
    sprintf(ans, "bulb %d %d %s %.2f\n", pid, id, status_string, tempoSecondi);//TODO aggiungere timer
    //sprintf(ans, "Bulb %d %d", pid, id);
    //strcat(ans ,(status==1? " accesa\0":" spenta\0"));
    //printf(": %s", ans);
}
//DETERMINATO DAL COMANDO CHE VIENE MANDATO IN GET_INFO_STRING
void set_info(char* info){
    char** info_split = splitLine(info);
    //<infoDefault> := default <id>
    if(strcmp(info_split[0], "default")==0){
        id = atoi(info_split[1]);
        status = 0; 
        tempoSecondi = 0; 
    }else{
        //<info> := <tipo> <pid???> <id> <status> <time>
        tipo = info_split[0][0];
        id = atoi(info_split[2]);
        if(strcmp(info_split[3], "accesa")){
            status = 1;
        }else{//spenta
            status = 0;
        }
        time(&tempoUltimaMisurazione);
        //tempoSecondi = atoi(info_split[4]);
    }
}

//COMANDO delete <id>
/*restituisco in pipe:
    0 se NON sono il dispositivo da eliminare
    pid se sono il dispositivo da eliminare
*/
int dev_delete(char **args){
    //printf("pid: %d\n",pid);
    int id_delete = atoi(args[1]);
    char* msg = malloc(ANSWER);//potrei fare il log10 dell'pid per trovare il numero di cifre

    if(id == id_delete){//guardo se il tipo e l'pid coincidono
        //scrivo sulla pipe che sono io quello che deve essere ucciso e scrivo anche il mio pid, la centralina dovrà toglierlo dalla lista
        //TODO trovare un altro metodo
       
        sprintf(msg, "%d", pid);//pid inteso come pid
        int esito = write(fd_write, msg, strlen(msg));
        
            printf("\033[1;31m"); //scrivo in rosso 
            printf("\x1b[ \n\t«Dio mio, Dio mio, perché mi hai abbandonato?»\n");
            printf("\033[0m"); //resetto per scriver in bianco
       
        printf("Eliminazione avvenuta con successo\n\n");
        kill(idPar,SIGCONT);

        exit(0);

    }else{
        sprintf(msg, "%d", 0);
        int esito = write(fd_write, msg, strlen(msg));
        printf("\tNon eliminato dato che pid non coincide\n");
        kill(idPar,SIGCONT);
    }
    //famo ritornare l'errore poi
    return 1;
}







int main(int argc, char *args[]){
    pid = getpid(); // chiedo il mio pid
    idPar = getppid(); //chiedo il pid di mio padre
    //leggo args per prendere gli argomenti passati(puntatore al lato di scrittura della pipe)
    fd_read = atoi(args[1]);
    fd_write = atoi(args[2]);  
    
    set_info(args[3]);


    signal(SIGINT, sighandle_int);
    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1

    printf("\nLampadina creata\n");
    printf("Id: %d\n", id);
    printf("Pid: %d\nPid padre: %d\n\n", pid, idPar);

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


