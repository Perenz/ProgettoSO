
#include "../strutture/listH.h"
//gestione comandi poi la porto qui
//#include "gestioneComandi.c"
  /*
int dev_manual_info_gen(cmd comando, int id, int idPar, int fd_write, int pid){
  
    risp answer;
    if(id == comando.id){//comando forzato per avere le info di dispositivi situati nel sott'albero di un processo che ha id 

        answer.considera = 1;
        answer.id = id;
        get_info_string(&(answer.info_disp));

        //Devo creare la fifo per il collegamento diretto
        char fifoManComp[30];
        
        sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
        mkfifo(fifoManComp, 0666);

        //Apro in lettura
        //*fd_manuale = open(fifoManComp, O_RDONLY | O_NONBLOCK , 0644);

        //Userò, dal manuale, il SIGUSR2 per questa fifo oppure il SIGUSR1 con controllo se lo switch è manuale o centralina
        //Non posso aprire prima in lettura
        //Posso usare il SIGUSR2 per aprire questa fifo
        
        //NON mi metto in ascolto, userò dei segnali da parte del manuale per dire al componente di leggere dalla pipe
        //Per essere chiusa devo scriverci qualcosa da manuale quando faccio il release       
    }else{
        answer.considera = 0;
    }
    rispondi(answer, comando,fd_write, idPar);
   
    //famo ritornare l'errore poi
    return 1;
}
 */


