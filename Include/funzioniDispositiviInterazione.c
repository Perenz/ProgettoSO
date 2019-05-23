#include <signal.h>
#include <time.h>
#include "../strutture/comandiH.h"
#include <fcntl.h>
#include <unistd.h>

//#include "../strutture/comandinonvacazz.c"
//#include "gestioneComandi.c"

#define ANSWER 64
#define CEN_BUFSIZE 128
#define CEN_DELIM " \t\r\n\a"


//PER ORA LE DEFINISCO QUI, POI VERRA FATTA UNA LIBRARY
void sighandle1(int sig, int fd_read, int);
void sighandle2(int sig);
int dev_info_gen(cmd comando, int id, int idPar, int fd_write, int pid, info);
int dev_list_gen(cmd comando, int idPar, int fd_write, info);
int dev_delete_gen(cmd comando, int pid, int id, int idPar, int fd_write, info);
int dev_add_gen(cmd comando, int id, int pid, int fd_write);

int rispondi(risp answer, cmd comando, int fd_write);
int dev_manual_info_gen(cmd comando, int id, int idPar, int fd_write, int pid, info informazioni);



//TODO I due handler sono compattabili in uno unico con controllo del tipo di segnale appena prima del read
//SIGUSR1 usato per l'implementazione della lettura della pipe con il padre
void sighandle1(int sig, int fd_read, int fd_write){
    if(sig == SIGUSR1){
        cmd comando;
        read(fd_read, &comando, sizeof(cmd));
        comando.manuale=0;
        int errnum = device_handle_command(comando);
        if(errnum == -1){
            risp answer;  
            answer.considera = 0;
            answer.eliminato = 0;
            rispondi(answer, comando, fd_write);
        }
    }
}
void sighandle2(int sig){
    if(sig == SIGUSR2){
        char fifoManComp[30];
        sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
        int fd_manuale = open(fifoManComp, O_RDONLY);
        cmd comando;
        read(fd_manuale, &comando, sizeof(cmd));//uso 10 per intanto, vedi sopra poi
        close(fd_manuale);
        comando.manuale=1;
        int errnum = device_handle_command(comando);
    }
}

int rispondi(risp answer, cmd comando, int fd_write){
    answer.profondita = comando.profondita+1;
    answer.id_padre = comando.id_padre;
    comando.profondita+=1;
    answer.termina_comunicazione = 0;
    write(fd_write, &answer, sizeof(answer));


    answer.termina_comunicazione = 1;
    write(fd_write, &answer, sizeof(answer));
    return 1;
}

//COMANDO   info <id>
/*restituisce in pipe
    <info> := <tipo> <pid???> <id> <status> <time>
*/
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
//GUARDA LISTGUARDA LISTGUARDA LISTGUARDA LISTGUARDA LISTGUARDA LISTGUARDA LISTGUARDA LISTGUARDA LISTGUARDA LIST
int dev_info_gen(cmd comando, int id, int idPar, int fd_write, int pid, info informazioni){
    risp answer;
    if(id == comando.id || comando.forzato == 1){//comando forzato per avere le info di dispositivi situati nel sott'albero di un processo che ha id 
        answer.pid = pid;
        answer.considera = 1;
        answer.id = id;
        answer.dispositivo_interazione = 1;
        answer.info_disp.def = 0;
        answer.info_disp = informazioni;
        
        //get_info_string(&(answer.info_disp));
    }else{
        answer.considera = 0;
        answer.dispositivo_interazione = 1;
    }
    rispondi(answer, comando, fd_write);
    
    //famo ritornare l'errore poi
    return 1;
}

//COMANDO   l
/*restituisce in pipe
    se comando è l: <informazioni>
*/
int dev_list_gen(cmd comando, int idPar, int fd_write, info informazioni){
    risp answer;
    answer.info_disp = informazioni;
    answer.considera = 1;
    answer.info_disp.def = 0;
    answer.info_disp = informazioni;
    
    rispondi(answer, comando, fd_write);

    return 1;
}



//COMANDO d <pid>
/*restituisco in pipe:
    0 se NON sono il dispositivo da eliminare
    pid se sono il dispositivo da eliminare
*/
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

int dev_delete_gen(cmd comando, int pid, int id, int idPar, int fd_write, info informazioni){
    //printf("pid: %d\n",pid);
    risp answer;
    if(id == comando.id || comando.forzato){
        answer.pid = pid;
        answer.id = id;
        answer.considera = 1;
        answer.eliminato = 1;

        answer.info_disp.def = 0;
        answer.info_disp = informazioni;

        rispondi(answer, comando, fd_write);
        exit(0);
    }else{
        answer.pid = pid;
        answer.id = id;
        answer.considera = 0;
        answer.eliminato = 0;
        rispondi(answer, comando, fd_write);
    }
    return 1;
}


int dev_manual_info_gen(cmd comando, int id, int idPar, int fd_write, int pid, info informazioni){
    risp answer;
    if(id == comando.id){//comando forzato per avere le info di dispositivi situati nel sott'albero di un processo che ha id 

        answer.pid = pid;
        answer.considera = 1;
        answer.id = id;
        answer.dispositivo_interazione = 1;
        answer.info_disp.def = 0;
        answer.info_disp = informazioni;
        //get_info_string(&(answer.info_disp));

        //Devo creare la fifo per il collegamento diretto
        char fifoManComp[30];
        
        sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
        mkfifo(fifoManComp, 0666);
    
    }else{
        answer.considera = 0;
        answer.dispositivo_interazione = 1;
    }
    rispondi(answer, comando,fd_write);
    
    //famo ritornare l'errore poi
    return 1;
}