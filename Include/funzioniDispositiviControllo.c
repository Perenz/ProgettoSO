//ci mettiamo gestioneComandi ed add_device
//inoltre faccio che qui prendo le funzioni dichiarate in functionDeclaration ma le faccio con un argomento solo così da poterle usare
//faccio anche il compact_line che dato un char** restituisce un char*

//per risolvere ad esempio il fatto che hub con info usa broadcast_list basta che chiamo list al posto che info banana
//le altre sono uguali e perciò le ho già fatte

//devo togliere il -1 di marcello e far si che il remove sulla lista funzioni anche sul primo elemento

//#include "../strutture/listH.h"
//gestione comandi poi la porto qui
#include "gestioneComandi.c"

int dev_manual_info_gen(cmd comando, int id, int idPar, int fd_write, int pid){
    /*
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
    */
    //famo ritornare l'errore poi
    return 1;
}



