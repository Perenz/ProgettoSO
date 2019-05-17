#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


//ho spostato i metodi getLine e splitLine in una nuova libreria 
//TODO tale verrà linkata nel gestore generale dei processi di interazione



int dev_getinfo(char **args);
int dev_delete(char **args);
int dev_changestate(char **args);
int dev_manualControl(char **);
int dev_switch(char **args);
int dev_list(char **args);
int dev_info(char **args);
void get_info_string(char* ans);
void set_time();
int device_handle_command(char **args);
#include "../Include/funzioniDispositiviInterazione.c"

time_t tempoUltimaMisurazione;
double tempoSecondi;
int status;

//TODO : potrei usare un unico node
//File descriptor in cui il figlio legge e il padre scrive
int fd_read;
//File descriptor in cui il figlio scrive e il padre legge
int fd_write;

int fd_manuale=0;
char tipo = 'b';
pid_t idPar;
pid_t pid;
int id;


char *builtin_func[]={
        "l",//list
        "s",//changeState
        "i",//getInfo
        "d",//delete
        "im"//getInfoManual
};

void signhandle_quit(int sig){
    char fifo[30];
    if(sig==SIGQUIT){
        if(fd_manuale!=0){
            sprintf(fifo, "/tmp/fifoManComp%d", pid);
            remove(fifo);
        }
        _exit(0);
    }
}
void sighandle_usr1(int sig){
    sighandle1(sig, fd_read);
}
void sighandle_usr2(int sig){
    sighandle2(sig, fd_manuale);
}
//COMANDO   l
/*restituisce in pipe
    se comando è l: <informazioni>
*/
int dev_list(char **args){
    int err = dev_list_gen(args, idPar, fd_write);
    return err;
}
//COMANDO   info <id>
/*restituisce in pipe
    <info> := <tipo> <pid???> <id> <status> <time>
*/
int dev_info(char **args){
    int err = dev_info_gen(args, id, idPar, fd_write);
    return err;
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
//COMANDO d <pid>
/*restituisco in pipe:
    0 se NON sono il dispositivo da eliminare
    pid se sono il dispositivo da eliminare
*/
int dev_delete(char **args){
    int err = dev_delete_gen(args, pid, id, idPar, fd_write);
    return err;
}


int device_handle_command(char **args){
    //da fare come in functionDeclarations in file dispositivi
    if(strcmp(args[0],builtin_func[0])==0){//list
        return dev_list(args);
    }else if(strcmp(args[0],builtin_func[1])==0){//changestate
        return dev_switch(args);
    }else if(strcmp(args[0],builtin_func[2])==0){//info   
        return dev_info(args);   
    }else if(strcmp(args[0],builtin_func[3])==0){//delete
        return dev_delete(args);
    }else if(strcmp(args[0], builtin_func[4])==0){//manual
        return dev_manualControl(args);
    }
    return 1;
}


//COMANDO switch <id> <label> <stato:on/off>
/*restituisco in pipe:
    0 se NON sono il dispositivo in cui ho modificato lo stato
    1 se sono il dispositivo in cui ho modificato lo stato
*/
//MANCA CONTROLLO LABEL E STATO
int dev_switch(char **args){
    int id_change = atoi(args[1]);
    printf("FRATELLIIIOOOOOO");
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
        free(answer);
        
    }else{
        int esito = write(fd_write, "0\0", 2);//TODO
    }
    kill(idPar,SIGCONT);
    //famo ritornare l'errore poi
    return 1;
}


void get_info_string(char* ans){//TODO aggiungere timer
    set_time();
    memset(ans, 0, ANSWER);
    //<info> := <tipo> <pid???> <id> <status> <time>
    char* status_string = malloc(4);
    status_string = status==1? "on" : "off";
    sprintf(ans, "bulb %d %d %s %.2f\n", pid, id, status_string, tempoSecondi);//TODO aggiungere timer
    //free(status_string);
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
        if(strcmp(info_split[3], "on")==0){
            status = 1;
        }else{//spenta
            status = 0;
        }
        time(&tempoUltimaMisurazione);
        //tempoSecondi = atoi(info_split[4]);
    }
}

int dev_manualControl(char **args){
    int id_info = atoi(args[1]);
    char* msg = malloc(10);

    if(id == id_info){//guardo se il tipo e l'id coincidono
    //scrivo sulla pipe che sono io quello che deve essere ucciso e scrivo anche il mio pid, la centralina dovrà toglierlo dalla lista
    //TODO trovare un altro metodo
        //In caso di id coincidente devo aprire la fifo a cui poi si connette il controllo manuale
        char fifoManComp[30];
        
        sprintf(fifoManComp, "/tmp/fifoManComp%d", pid);
        mkfifo(fifoManComp, 0666);

        //Apro in lettura
        //Userò, dal manuale, il SIGUSR2 per questa fifo oppure il SIGUSR1 con controllo se lo switch è manuale o centralina
        //Non posso aprire prima in lettura
        //Posso usare il SIGUSR2 per aprire questa fifo
        fd_manuale = open(fifoManComp, O_RDONLY | O_NONBLOCK , 0644);
        //NON mi metto in ascolto, userò dei segnali da parte del manuale per dire al componente di leggere dalla pipe
        //Per essere chiusa devo scriverci qualcosa da manuale quando faccio il release

        sprintf(msg, "%d", pid);
        int esito = write(fd_write, msg, strlen(msg)+1);//Comunico alla centralina di aver trovato l'id cercato
        
        //free(msg);
    }
    else{
        //Se ID non coincide scrivo 0 sulla pipe
        sprintf(msg, "%d", 0);
        int esito = write(fd_write, msg, strlen(msg)+1);
        //printf("Non restituisce info dato che pid non coincide\n");
        
    }
    kill(idPar,SIGCONT);

    return 1;
}

int main(int argc, char *args[]){
    pid = getpid(); // chiedo il mio pid
    idPar = getppid(); //chiedo il pid di mio padre
    //leggo args per prendere gli argomenti passati(puntatore al lato di scrittura della pipe)
    fd_read = atoi(args[1]);
    fd_write = atoi(args[2]);  
    
    set_info(args[3]);

    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1
    signal(SIGUSR2, sighandle_usr2); //Alla ricezione di SIGUSR2 leggere il comanda sulla fifo direttamente connessa al manuale

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


