#include <signal.h>
#include <time.h>
#include "gestioneComandi.c"


#define CEN_BUFSIZE 128
#define ANSWER 32
//PER ORA LE DEFINISCO QUI, POI VERRA FATTA UNA LIBRARY
void sighandle1(int sig, int fd_read);
void sighandle2(int sig, int fd_manuale);
int dev_info_gen(char **args, int id, int idPar, int fd_write);
int dev_list_gen(char **args, int idPar, int fd_write);
int dev_delete_gen(char **args, int pid, int id, int idPar, int fd_write);


//TODO I due handler sono compattabili in uno unico con controllo del tipo di segnale appena prima del read
//SIGUSR1 usato per l'implementazione della lettura della pipe con il padre
void sighandle1(int sig, int fd_read){
    if(sig == SIGUSR1){
        //proviamo a leggere
        //potrei passare anche la lunghezza del messaggio
        char str[CEN_BUFSIZE];
        memset(str, 0, CEN_BUFSIZE);
        read(fd_read, str, CEN_BUFSIZE);//uso 10 per intanto, vedi sopra poi
        //printf("\n\tLettura da pipe sig1 %s  \n", str);
        
        char** arg = splitLine(str);
        int errnum = device_handle_command(arg);
        //free arg bro
    }
}
void sighandle2(int sig, int fd_manuale){
    if(sig == SIGUSR2){
        //proviamo a leggere
        //potrei passare anche la lunghezza del messaggio
        char str[CEN_BUFSIZE];
        //printf("Leggo dalla fifo manale\n");
        read(fd_manuale, str, CEN_BUFSIZE);//uso 10 per intanto, vedi sopra poi
        char** arg = splitLine(str);
        int errnum = device_handle_command(arg);
    }
}



//COMANDO   info <id>
/*restituisce in pipe
    <info> := <tipo> <pid???> <id> <status> <time>
*/
int dev_info_gen(char **args, int id, int idPar, int fd_write){
    int id_info = atoi(args[1]);
    if(id == id_info){
        char* ans = malloc(ANSWER);
        //MARCELLO SEI UN ZEBI, INFO ZEBI, INFO DEVI PASSARGLIELO COME PARAMETRO
        get_info_string(ans);
        int esito = write(fd_write, ans, strlen(ans)+1);
        //free(ans);
        kill(idPar,SIGCONT);
    }else{
        int esito = write(fd_write, "0\0", 2);
        //printf("Non restituisce info dato che id non coincide\n");
    }
    kill(idPar,SIGCONT);
    
    //famo ritornare l'errore poi
    return 1;
}

//COMANDO   l
/*restituisce in pipe
    se comando è l: <informazioni>
*/
int dev_list_gen(char **args, int idPar, int fd_write){
    char* ans = malloc(ANSWER);
    get_info_string(ans);
    int esito = write(fd_write, ans, strlen(ans)+1);

    //free(ans);
    kill(idPar,SIGCONT);
    return 1;
}



//COMANDO d <pid>
/*restituisco in pipe:
    0 se NON sono il dispositivo da eliminare
    pid se sono il dispositivo da eliminare
*/
int dev_delete_gen(char **args, int pid, int id, int idPar, int fd_write){
    //printf("pid: %d\n",pid);
    int id_delete = atoi(args[1]);
    char* msg = malloc(ANSWER);//potrei fare il log10 dell'pid per trovare il numero di cifre

    if(id == id_delete){//guardo se il tipo e l'pid coincidono
        //scrivo sulla pipe che sono io quello che deve essere ucciso e scrivo anche il mio pid, la centralina dovrà toglierlo dalla lista
        //TODO trovare un altro metodo
       
        sprintf(msg, "%d", pid);//pid inteso come pid
        int esito = write(fd_write, msg, strlen(msg)+1);
        
            printf("\033[1;31m"); //scrivo in rosso 
            printf("\x1b[ \n\t«Dio mio, Dio mio, perché mi hai abbandonato?»\n");
            printf("\033[0m\n"); //resetto per scriver in bianco
        free(msg);
        kill(idPar,SIGCONT);

        exit(0);

    }else{
        sprintf(msg, "%d", 0);
        int esito = write(fd_write, msg, strlen(msg)+1);
        //printf("\tNon eliminato dato che pid non coincide\n");
        free(msg);
        kill(idPar,SIGCONT);
    }
    
    //famo ritornare l'errore poi
    return 1;
}