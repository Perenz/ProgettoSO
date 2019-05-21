#include <signal.h>
#include <time.h>
#include "../strutture/comandiH.h"
//#include "../strutture/comandinonvacazz.c"
//#include "gestioneComandi.c"

#define ANSWER 64
#define CEN_BUFSIZE 128
#define CEN_DELIM " \t\r\n\a"


//PER ORA LE DEFINISCO QUI, POI VERRA FATTA UNA LIBRARY
void sighandle1(int sig, int fd_read, int);
void sighandle2(int sig, int fd_manuale);
int dev_info_gen(cmd comando, int id, int idPar, int fd_write);
int dev_list_gen(cmd comando, int idPar, int fd_write);
int dev_delete_gen(cmd comando, int pid, int id, int idPar, int fd_write);
int rispondi(risp answer, cmd comando, int fd_write, int pidPapi);
char** splitLine(char* line);
//NON HO VOGLIA DI RISOLV L'ERRORE

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


//TODO I due handler sono compattabili in uno unico con controllo del tipo di segnale appena prima del read
//SIGUSR1 usato per l'implementazione della lettura della pipe con il padre
void sighandle1(int sig, int fd_read, int pid_padre){
    if(sig == SIGUSR1){
        cmd comando;
        read(fd_read, &comando, sizeof(cmd));
        int errnum = device_handle_command(comando);
        //    kill(pid_padre, SIGCONT);
    
    }
}
void sighandle2(int sig, int fd_manuale){
    if(sig == SIGUSR2){
        //PERCHÈ NON USI SIG1??????????????????'
        cmd comando;
        read(fd_manuale, &comando, sizeof(cmd));//uso 10 per intanto, vedi sopra poi
        //printf("\n\tLettura da pipe sig1 %s  \n", str);
        int errnum = device_handle_command(comando);
    }
}

int rispondi(risp answer, cmd comando, int fd_write, int pidPapi){
    answer.profondita = comando.profondita+1;
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
int dev_info_gen(cmd comando, int id, int idPar, int fd_write){
     risp answer;
    if(id == comando.id || comando.forzato == 1){//comando forzato per avere le info di dispositivi situati nel sott'albero di un processo che ha id 
        
        answer.considera = 1;
        answer.id = id;
        //get_info_string(answer.info);
        
        
    }else{
        answer.considera = 0;
    }
    rispondi(answer, comando,fd_write, idPar);
    
    //famo ritornare l'errore poi
    return 1;
}

//COMANDO   l
/*restituisce in pipe
    se comando è l: <informazioni>
*/
int dev_list_gen(cmd comando, int idPar, int fd_write){
    risp answer;
    get_info_string(&(answer.info_disp));
    answer.considera = 1;
    /*
    answer.foglia = 1;
    answer.termina_comunicazione = 1;
    */
    rispondi(answer, comando, fd_write, idPar);

    return 1;
}



//COMANDO d <pid>
/*restituisco in pipe:
    0 se NON sono il dispositivo da eliminare
    pid se sono il dispositivo da eliminare
*/
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

int dev_delete_gen(cmd comando, int pid, int id, int idPar, int fd_write){
    //printf("pid: %d\n",pid);
    risp answer;
    if(id == comando.id || comando.forzato){
        answer.pid = pid;
        answer.id = id;
        answer.considera = 1;
        answer.eliminato = 1;
        char* info = malloc(ANSWER);
        get_info_string(&(answer.info_disp));
        rispondi(answer, comando, fd_write, pid);
        exit(0);
    }else{
        answer.pid = pid;
        answer.id = id;
        answer.considera = 1;
        answer.eliminato = 0;
        rispondi(answer, comando, fd_write, pid);
    }
    return 1;
}