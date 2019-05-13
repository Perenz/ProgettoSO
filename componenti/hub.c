#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "../strutture/list.c"
#include "../Include/gestioneComandi.c"

#define CEN_BUFSIZE 128

NodoPtr dispList; //lista dei dispositivi collegati all'hub
pid_t idPar;
pid_t pid;
int id;
int status;
//File descriptor in cui il figlio legge e il padre scrive
int fd_read;
//File descriptor in cui il figlio scrive e il padre legge
int fd_write;

int dev_getinfo(char **args);
int dev_delete(char **args);
int dev_changestate(char **args);
int device_handle_command(char **args);


void signhandle_quit(int sig){
    _exit(0);
}

char *builtin_func[]={
        "l",//list
        "s",//changeState
        "i",//getInfo
        "d", //delete
        "a" //addDevice
};
void sign_cont_handler(int sig){
        return;
}

//SIGUSR1 usato per l'implementazione della lettura della pipe con il padre
//
void sighandle_usr1(int sig){
    if(sig == SIGUSR1){

        printf("HUB in handler sigusr1\n");
        //proviamo a leggere
        //potrei passare anche la lunghezza del messaggio
        char str[CEN_BUFSIZE];
        read(fd_read, str, CEN_BUFSIZE);//uso 10 per intanto, vedi sopra poi
        //printf("\n\tLettura da pipe %s  \n", str);
        char** arg = splitLine(str);
        int errnum = device_handle_command(arg);
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
    }else if(strcmp(args[0],builtin_func[4])==0){//addDevice
        return add_device(args);
    }
    return 1;
}

//COMANDO s <pid> <label> <stato:1/0>
/*restituisco in pipe:
    0 se NON sono il dispositivo in cui ho modificato lo stato
    1 se sono il dispositivo in cui ho modificato lo stato
*/
int dev_changestate(char **args){
    int id_change = atoi(args[1]);
    printf("%d\n", id_change);
    if(id_change == id){//devo confrontare lo stato
        /* potremo scriver un messaggio del tipo: dispositivo <pid> acceso / spento
        char* msg = malloc(10);//potrei fare il log10 dell'pid per trovare il numero di cifre
        sprintf(msg, "d %d", pid);//pid inteso come pid
        
        int esito = write(fd_write, msg, strlen(msg)+1);
        
        per un'altra idea vedi functionDeclaration in metodo cen_switch
        */
        
        status = atoi(args[3]);
        printf("%s\n", args[3]);

        printf("Status dispositivo Hub %d : %d\n", id, status);  
        printf("\033[1;32m"); //scrivo in verde 
        printf("\tNon sono felice e non sono triste. È questo il dilemma della mia vita: non so come definire il mio stato d’animo, mi manca sempre qualcosa.");
        printf("\033[0m\n"); //resetto per scriver in bianco

        char* msg = malloc(10);
        int esito = write(fd_write, "1\0", 2);
        kill(idPar,SIGCONT);
    }else{
        int esito = write(fd_write, "0\0", 2);//TODO
        printf("here\n\n");
        kill(idPar,SIGCONT);
    }
    //famo ritornare l'errore poi
    return 1;
}

//COMANDO i <tipo> <pid> o
//        l
/*restituisce in pipe
    se comando è l: <informazioni>
    se comando è i:
        se <pid> == id_dispositivo: <informazioni>
        else 0; TODO segnale errore
*/
int dev_getinfo(char **args){
    char* str = malloc(128);
    if(args[0][0] == 'l'){//list
        
        sprintf(str, "%d Hub %d", pid, id);
        strcat(str ,(status==1?" accesa\n":" spenta\n"));
        NodoPtr Nodo = dispList;
        Nodo = Nodo->next; //escludo il mio pid!
        while(Nodo != NULL){
            //TODO gestire errori
            write(Nodo->fd_writer,"l\0",2);
            kill(Nodo->data, SIGUSR1);            
            //TODO
            //pause();
            int temp = read(Nodo->fd_reader,str,30);
            //memset(tmp,0,30);
            //strcat(msg,tmp);            printf("\t%s", str);
            strcat(str, temp);
            Nodo = Nodo->next;
        }

        //printf("Pipe su cui scrivo %d, pipe su cui leggo %d \n", fd_write, fd_read);

        int esito = write(fd_write, str, strlen(str)+1);
        kill(idPar,SIGCONT);
        //pause();
    }else if(args[0][0] == 'i'){//info
        int id_info = atoi(args[1]);
        char* msg = malloc(10);


        if(id == id_info){//guardo se il tipo e l'pid coincidono
        //scrivo sulla pipe che sono io quello che deve essere ucciso e scrivo anche il mio pid, la centralina dovrà toglierlo dalla lista
        //TODO trovare un altro metodo
            sprintf(msg, "%d Hub %d",pid, id);
            strcat(msg ,(status==1?" accesa\n":" spenta\n"));
            int esito = write(fd_write, msg, strlen(msg));
            
            kill(idPar,SIGCONT);
        }else{
            NodoPtr Nodo = dispList;
            Nodo = Nodo->next; //escludo il mio pid!
            while(Nodo != NULL){
                char* tmp = malloc(1 + strlen(args[1]) + 3);//1 per il comando + lunghezza id (args[1]) + 3 per spazi e terminazione stringa
                //tipo di comando
                strcat(tmp,"i ");
                //id dispositivo da spegnere
                strcat(tmp, args[1]);
                //delimitatore 
                strcat(tmp, "\0");
                //TODO gestire errori
                write(Nodo->fd_writer, tmp, strlen(tmp));
                kill(Nodo->data, SIGUSR1);            
                //TODO
                //pause();
                int temp = read(Nodo->fd_reader,str,30);
                //memset(tmp,0,30);
                //strcat(msg,tmp);            printf("\t%s", str);
                if(strcmp(temp, "0")!=0){
                    return 1;
                } 
                Nodo = Nodo->next;
            }
            sprintf(msg, "%d", 0);
            int esito = write(fd_write, msg, strlen(msg));
            printf("Non restituisce info dato che pid non coincide\n");
            kill(idPar,SIGCONT);
        }
    }
    //famo ritornare l'errore poi
    return 1;
}


//COMANDO d <pid>
/*restituisco in pipe:
    0 se NON sono il dispositivo da eliminare (anche se il dispositivo era uno dei miei figli ed è stato correttamente eliminato)
    pid se sono il dispositivo da eliminare
*/
int dev_delete(char **args){
    //printf("pid: %d\n",pid);
    int id_delete = atoi(args[1]);
    char* msg = malloc(10);//potrei fare il log10 dell'pid per trovare il numero di cifre
    signal(SIGCONT, sign_cont_handler);


    if(id == id_delete){//guardo se il tipo e l'pid coincidono
        //scrivo sulla pipe che sono io quello che deve essere ucciso e scrivo anche il mio pid, la centralina dovrà toglierlo dalla lista
        //TODO trovare un altro metodo
       
        sprintf(msg, "%d", pid);//pid inteso come pid
        printf("pid in messaggio: %s\n",msg);
        int esito = write(fd_write, msg, strlen(msg));
        
        printf("\033[1;31m"); //scrivo in rosso 
        printf("\x1b[ \n\t«Dio mio, Dio mio, perché mi hai abbandonato?»\n");
        printf("\033[0m"); //resetto per scriver in bianco

        NodoPtr Nodo = dispList;
        //Escludo il mio pid dal while
        Nodo = Nodo->next;

        while(Nodo != NULL){ // se avevo dispositivi collegati mando un messaggio di delete ad ognuno di essei con il loro pid per farli eliminare!
            char* tmp = malloc(1 + strlen(args[1]) + 3);//1 per il comando + lunghezza id (args[1]) + 2 per spazi e terminazione stringa
            //tipo di comando
            strcat(tmp,"d ");
            //id dispositivo da spegnere
            strcat(tmp, Nodo->data); 
            //delimitatore 
            strcat(tmp, "\0");
            //scrivo il comando sulla pipe
            write(Nodo->fd_writer, tmp, strlen(tmp));
            //mando un segnale al figlio così si risveglia e legge il contenuto della pipe
            kill(Nodo->data, SIGUSR1);
            //printf("Mi metto in read dal figlio %d sul canale %d\n", Nodo->data, Nodo->fd[0]);
            //pause();
                
            //TODO gestione errori
            //leggo il pid del figlio così da poterlo togliere dalla lista di processi
            char* answer = malloc(30);
            int err = read(Nodo->fd_reader,answer, 30);
            //pause();
            //TODO se il figlio ritorna 0 esso non è figlio e perciò non lo elimino dalla lista
            int ris = atoi(answer);

            //printf("Lettura da pipe lato padre %d\n", ris);
            if(ris!=0){
                removeNode(dispList, ris);
            }                
            Nodo = Nodo->next;
            //risveglio il figlio così può eliminarsi
            //DOVREBBE FUNZIONAR ANCHE COSì, mi sa de no
            //kill(Nodo->data, SIGUSR1);
            //memset(tmp,0,30);
            //strcat(msg,tmp);
        }
        printf("Eliminazione avvenuta con successo\n\n");
        kill(idPar,SIGCONT);

        exit(0);

    }else{ // nel caso il pid non sia il mio (HUB) inoltro il messaggio ai miei filgi così che controllino se devono essere eliminati
        NodoPtr Nodo = dispList;
        //Escludo la centralina dal while
        Nodo = Nodo->next;

        while(Nodo != NULL){ 
            char* tmp = malloc(1 + strlen(args[1]) + 3);//1 per il comando + lunghezza id (args[1]) + 2 per spazi e terminazione stringa
            //tipo di comando
            strcat(tmp,"d ");
            //id dispositivo da spegnere
            strcat(tmp, args[1]); 
            //delimitatore 
            strcat(tmp, "\0");
            //scrivo il comando sulla pipe
            write(Nodo->fd_writer, tmp, strlen(tmp));
            //mando un segnale al figlio così si risveglia e legge il contenuto della pipe
            kill(Nodo->data, SIGUSR1);
            //printf("Mi metto in read dal figlio %d sul canale %d\n", Nodo->data, Nodo->fd[0]);
            //pause();
                
            //TODO gestione errori
            //leggo il pid del figlio così da poterlo togliere dalla lista di processi
            char* answer = malloc(30);
            int err = read(Nodo->fd_reader,answer, 30);
            //pause();
            //TODO se il figlio ritorna 0 esso non è figlio e perciò non lo elimino dalla lista
            int ris = atoi(answer);

            //printf("Lettura da pipe lato padre %d\n", ris);
            if(ris!=0){
                removeNode(dispList, ris);
                return 1;
            }                
            Nodo = Nodo->next;
            //risveglio il figlio così può eliminarsi
            //DOVREBBE FUNZIONAR ANCHE COSì, mi sa de no
            //kill(Nodo->data, SIGUSR1);
            //memset(tmp,0,30);
            //strcat(msg,tmp);
        }
        sprintf(msg, "%d", 0);
        int esito = write(fd_write, msg, strlen(msg));
        kill(idPar,SIGCONT);
    }
    //famo ritornare l'errore poi
    return 1;
}

//come addDevice della centralina, solo che è interno all'hub e quindi sarà l'hub a generare i processi
//TODO sistemare gli argomenti passati in modo che ci sia il path corretto per l'eseguibile e quindi sistemare la gestione degli argomenti passati
//va anche aggiunto un controllo per verificare che, se non è il primo dispositivo aggiunto all'hub, sia dello stesso dipo di quelli già presenti
//possibile passarlo come parametro(?)
int add_device(char* execPath){
//TODO potrei passare solo i canali su cui scriver e non tutta la pipe
        pid_t pidChild;
        int fd_reader[2];
        int fd_writer[2];

        pipe(fd_reader); //creo la pipe
        pipe(fd_writer);
        pid=fork();
        
        if (pidChild == 0) {
                // Child process
                //TODO modifica fdTmp
                char fd_writer_Tmp[10];
                char fd_reader_Tmp[10];
                char idTmp[10]; //id che verrà passato in args al processo creato
                sprintf(idTmp,"%d",id); //salvo in idTmp l'id attuale e poi lo incremento
                //TODO close(fd[0]);
                sprintf(fd_writer_Tmp,"%d", (fd_writer[0]));
                sprintf(fd_reader_Tmp,"%d", (fd_reader[1]));
                char *args[]={execPath,fd_writer_Tmp, fd_reader_Tmp , idTmp, NULL}; 
                execvp(args[0],args); //passo gli argomenti incluso il puntatore al lato di scrittura della pipe
        } else if (pidChild < 0) {
                // Errore nell'operazione di fork
                perror("errore fork");
        } 
        else{
                //Parent process
                //Indico al processgit po corrente (il padre) di gestire segnali in entrata di tipo SIGCONT con la funzione sign_handler
                //close(fd[1]); //chiudo il lato di scrittura;
                //Aggiungo alla lista dei processi quello appena generato, identificato dal suo pid
                //Devo aggiungere anche i fd per la pipe
                
                //DEBUG
                insertLast(dispList, pidChild, fd_reader[0],fd_writer[1]);
                
                //modifica Paolo --> Voglio far sì che il filedescriptor sia unico per la centralina --> qua devo fare il dup
                //insertLast(procList, pid, fd_reader[0], fd_writer[1]);
                /**
                 * TODO
                 * Nella funzione insertLast devo anche passare i FD cosi che vengano aggiunti al nodo
                 * */
                
                
                //Vado in pausa per permettere al figlio di generarsi
                pause();



                //char msg[30];
                // int rev = read(fd[0],msg,512);
                // printf("\nesito: %d\n", rev);        
                // printf("\n\n\n\ndebug msg: %s\n\n\n",msg);
                
        }
        
        return 1;
}





int main(int argc, char **args){
    pid = getpid(); // chiedo il mio pid
    idPar = getppid(); //chiedo il pid di mio padre
    dispList = listInit(getpid());
    id = atoi(args[3]);
    //0 spenta
    //1 accesa
    status = 0; //equivalente a quello dei dispositivi collegati

    //leggo args per prendere gli argomenti passati(puntatore al lato di scrittura della pipe)
    fd_read = atoi(args[1]);
    fd_write = atoi(args[2]);

    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1


    printf("\nHub creato\n");
    printf("Pid: %d\nPid padre: %d\n", pid, idPar);

    //Invio segnale al padre
    int ris = kill(idPar, SIGCONT); 

    //Child va in pausa
    while(1){
        //printf("Child va in pausa\n");
        pause();
    }


    printf("Child ora termina\n");   
    exit(0);
}