#include <signal.h>
#include <time.h>
#include "../strutture/comandiH.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#define ANSWER 64
#define CEN_DELIM " \t\r\n\a"

void sighandle1(int sig, int fd_read, int);
void sighandle2(int sig);
int dev_info_gen(cmd comando, int id, int idPar, int fd_write, int pid, info);
int dev_list_gen(cmd comando, int idPar, int fd_write, info);
int dev_delete_gen(cmd comando, int pid, int id, int idPar, int fd_write, info);


int rispondi(risp answer, cmd comando, int fd_write);
int dev_manual_info_gen(cmd comando, int id, int idPar, int fd_write, int pid, info informazioni);

//SIGUSR1 usato per l'implementazione della lettura della pipe con il padre
void sighandle1(int sig, int fd_read, int fd_write){
    if(sig == SIGUSR1){
        cmd comando;
        read(fd_read, &comando, sizeof(cmd));
        comando.manuale=0;
        int errnum = device_handle_command(comando);
        // se il comando non è tra quelli inseriti comunico un errore nella mia risposta
        if(errnum == -1){
            risp answer;  
            answer.considera = 0;
            answer.eliminato = 0;
            answer.errore = 1;
            rispondi(answer, comando, fd_write);
        }
    }
}
//SIGURS2 usato per gestire il comando manuale
void sighandle2(int sig){
    if(sig == SIGUSR2){
        char fifoManComp[30];
        sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
        int fd_manuale = open(fifoManComp, O_RDONLY);
        if(fd_manuale<0){
            printf("Errore nell'apertura della Fifo in READONLY %s\n", strerror(errno));
        }
        cmd comando;
        read(fd_manuale, &comando, sizeof(cmd));
        close(fd_manuale);
        comando.manuale=1;
        int errnum = device_handle_command(comando);
    }
}

//funzione utilizzata per comunicare il padre la propria risposta,
//vedi gestioneComandi per l'implementazione dell'attesa della risposta e la lettura da parte del padre e
int rispondi(risp answer, cmd comando, int fd_write){
    if(answer.considera == 1){
        comando.profondita += 1;
        answer.profondita = comando.profondita;
    }
    
    answer.errore = 0;
    answer.id_padre = comando.id_padre;
    answer.termina_comunicazione = 0;

    write(fd_write, &answer, sizeof(risp));

    answer.termina_comunicazione = 1;
    write(fd_write, &answer, sizeof(risp));
    return 1;
}

//COMANDO   info <id>
/*restituisce in pipe
    <info> := <tipo> <pid???> <id> <status> <time>
*/
int dev_info_gen(cmd comando, int id, int idPar, int fd_write, int pid, info informazioni){
    risp answer;
    if(id == comando.id || comando.forzato == 1){//comando forzato per avere le info di dispositivi situati nel sott'albero di un processo che ha id 
        answer.pid = pid;
        answer.considera = 1;
        answer.id = id;
        answer.dispositivo_interazione = 1;
        answer.info_disp.def = 0;
        answer.info_disp = informazioni;
    }else{
        answer.considera = 0;
        answer.dispositivo_interazione = 1;
    }
    rispondi(answer, comando, fd_write);

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
    rispondi(answer, comando, fd_write);

    return 1;
}



//COMANDO d <pid>
/*restituisco in pipe:
    considera=0 e eliminato=0 se NON sono il dispositivo da eliminare
    considera=1 e eliminato10, pid se sono il dispositivo da eliminare
*/
int dev_delete_gen(cmd comando, int pid, int id, int idPar, int fd_write, info informazioni){
    //printf("pid: %d\n",pid);
    risp answer;
    if(id == comando.id || comando.forzato == 1){//comando forzato = 1 per avere le info di dispositivi situati nel sott'albero di un processo che ha id 
        answer.pid = pid;
        answer.id = id;
        answer.considera = 1;
        answer.eliminato = 1;

        answer.info_disp.def = 0;
        answer.info_disp = informazioni;
        rispondi(answer, comando, fd_write);
        //ho risposto al padre con le mie info per farmi eliminare dalla lista dei figli
        //ora posso eliminarmi 
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
    if(id == comando.id){
        answer.pid = pid;
        answer.considera = 1;
        answer.id = id;
        answer.dispositivo_interazione = 1;
        answer.info_disp.def = 0;
        answer.info_disp = informazioni;
        //Devo creare la fifo per il collegamento diretto
        char fifoManComp[30];
        
        sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
        mkfifo(fifoManComp, 0666);
    
    }else{
        answer.considera = 0;
        answer.dispositivo_interazione = 1;
    }
    rispondi(answer, comando,fd_write);
    
    return 1;
}