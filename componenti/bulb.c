#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <assert.h>
#include <sched.h>
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
        "c",//changeState
        "g",//getInfo
        "d" //delete
};

//TODO
//al posto che un sighandle per ogni comando dato dalla centralina potremo utilizzare un metodo svegliami da metter in una funzione a parte
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
        char str[50];

        read(fd_read, str, 10);//uso 10 per intanto, vedi sopra poi
        printf("\n\t Lettura da pipe %s  \n", str);

        //TODO tutto questo dovrebbe farlo una funzione split command e tipo e comando potrebbero esser un singolo carattere
        char* comando = malloc(2);
        char* tipo = malloc(2);
        char* id = malloc(2);//2 come prova, vedi commento sopra
        
        comando[0] = str[0];
        comando[1] = '\0';
        //printf("comando: %s\n", comando);
        tipo[0] = str[2];
        tipo[1] = '\0';
        //printf("tipo: %s\n", tipo);
        id[0] = str[4];
        id[1] = '\0';
        //printf("id: %s \n", id);
        
        
        char** arg = malloc(3*(sizeof(char)));
        arg[0] = comando;
        arg[1] = tipo;
        arg[2] = id;


        //solo per switch prova, TODO cambiare
        arg[3] = "1\0";

        int errnum = device_handle_command(arg);
        //printf("%d\n", strlen(str));
        
        //printf("siamo ancora qua...\n");


        //sprintf(str, "Bulb %d", id);
        //printf("Figlio ora scrive\n");
        //strcat(str ,(status==1?" accesa\n":" spenta\n"));

        //printf("Pipe su cui scrivo %d, pipe su cui leggo %d \n", fd_write, fd_read);

        //int esito = write(fd_write, str, strlen(str)+1);
        
        
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


//COMANDO c <tipo> <id> <stato:1/0>
int dev_changestate(char **args){
    if(args[1][0]== tipo /*&& strcmp(args[2], id)==0*/){//guardo se il tipo e l'id coincidono
        /* potremo scriver un messaggio del tipo: dispositivo <id> acceso / spento
        char* msg = malloc(10);//potrei fare il log10 dell'id per trovare il numero di cifre
        sprintf(msg, "d %d", id);//id inteso come pid
        
        int esito = write(fd_write, msg, strlen(msg)+1);
        */
        status = atoi(args[3]);
        
        
        char* str = malloc(30);
        printf("Status %d\n", status);//non funziona ancora perché non gestisco lo split dei comandi
        str = "Ha funzionato\0";

        //printf("Pipe su cui scrivo %d, pipe su cui leggo %d \n", fd_write, fd_read);

        //int esito = write(fd_write, str, strlen(str)+1);
        
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

//COMANDO d <tipo> <id>
int dev_delete(char **args){
    printf("%s\n",args[1]);
    printf("%c\n",tipo);
    if(args[1][0]== tipo /*&& strcmp(args[2], id)==0*/){//guardo se il tipo e l'id coincidono
        //scrivo sulla pipe che sono io quello che deve essere ucciso e scrivo anche il mio pid, la centralina dovrà toglierlo dalla lista
        //TODO trovare un altro metodo
        char* msg = malloc(10);//potrei fare il log10 dell'id per trovare il numero di cifre
        sprintf(msg, "d %d", id);//id inteso come pid
        int esito = write(fd_write, msg, strlen(msg)+1);
        printf("questo è il pid: %s\n", msg);
        printf("Eliminazione avvenuta con successo\n");
        kill(idPar,SIGCONT);
        exit(0);//questo exit 0 avviene? forse sarebbe meglio terminarlo dal padre

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


