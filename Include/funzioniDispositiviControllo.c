
#include "../strutture/listH.h"
//gestione comandi poi la porto qui
#include "gestioneComandi.c"
  
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
 


