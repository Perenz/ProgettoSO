#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//Variabili che indica il pid del dispositivo su cui si sta agendo manualmente
//Di default uguale a 0, cioè non ho ancora effettuato il "collegamento" con alcun dispositivo
int controllo=0;

//Lettura del comando identica a quanto fatto per la centralina in start.c


//DUBBIO: Come ottengo il pid della centralina?:
//Si potrebbe aprire una FIFO nella centralina in readonly anche se non l'altro estremo (manuale.c) non è ancora presente (Ciò e permesso)
// https://stackoverflow.com/questions/48697193/how-to-wait-for-the-other-end-of-a-named-pipe-to-be-open
//Poi quando avvio manuale.c apro la fifo sia write che read, scrivo a centralina che poi potrà rispondere perchè ora c'è l'altro estremo
//Mi serve comunque il pid della centralina per mandare il signal che dice ci leggere dalla fifo


/*Cambio il processCmd in questo modo:
    - Se controllo == 0
        Significa che nessun dispositivo è al momento controllato quindi si dovrà effettuare il collegamento digitando il nome,id o pid del 
        dispositivo su cui vogliamo agire.
        Tramite la centralina (ricercaNellAlbero) torniamo il pid del dispositivo con tale nome/id cosi da poter istanziare una
        FIFO tra manuale.c ed il dispositivo interessato.
        A questo punto la variabile pid cambia

        controllo rimane assegnata fino a quando non si effettua una exit/quit dal dispositivo "attivo"

    - Se controllo != 0
        Significa che stiamo gia controllando un dispositivo e possiamo quindi impartire i vari comandi come:
            - switch ...
            - status ...
            - ...
            - quit/exit

*/

#define myFIFO "/tmp/miaFifo"

int getCenPid(){
    int fd;
    //char *fifo="./test/newFifo";
    char msg[20] = "hand";

    //La apro per write only
    fd = open(myFIFO, O_WRONLY);

    //Invio msg "hand" sulla fifo
    int esito = write(fd, msg, strlen(msg)+1);

    //Chiudo il Write only
    close(fd);

    //Apro in read
    fd = open(myFIFO, O_RDONLY);
    read(fd, msg, 10);

    close (fd);

    return atoi(msg);
}

int main(){
    //Passo 1: Prendere il pid della centralina grazie ad il processo di support
    //Creo la fifo
    mkfifo(myFIFO, 0666);

    printf("Ecco il pid %d", getCenPid());

    //Ora mi comporto diversamente a secondo del valore di controllo
    //in entrambi i casi prendo i comandi esattamente come faccio in start.c, cambia l'insieme di comandi disponibili
    //Quindi cambia solo il processCmd con una serie di funzioni bultin diverse per i due casi

    exit(0);
}