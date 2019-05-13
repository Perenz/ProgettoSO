#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int add_device(char*, NodoPtr, NodoPtr);
int id = 2; //id che parte da 2, 1 = centralina
void sign_handler(int);
//device list
char *builtin_device[]={
        "bulb",
        "window",
        "fridge",
        "timer",
        "hub"
};

char *bultin_dev_path[]={
        "./componenti/BULB",
        "./componenti/WINDOW",
        "./componenti/FRIDGE",
        "./componenti/TIMER",
        "./componenti/HUB"
};


int device_number(){
        return sizeof(builtin_device)/sizeof(char*);
}

//Signal handler per gestire l'arrivo di segnali
void sign_handler(int sig){
    return;
}
int add_device(char* execPath, NodoPtr procList, NodoPtr dispList){
//TODO potrei passare solo i canali su cui scriver e non tutta la pipe
        pid_t pid, wpid;
        int fd_reader[2];
        int fd_writer[2];

        pipe(fd_reader); //creo la pipe
        pipe(fd_writer);
        pid=fork();
        
        if (pid == 0) {
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
        } else if (pid < 0) {
                // Errore nell'operazione di fork
                perror("errore fork");
        } 
        else{
                //Parent process
                //Indico al processgit po corrente (il padre) di gestire segnali in entrata di tipo SIGCONT con la funzione sign_handler
                signal(SIGCONT, sign_handler);
                //close(fd[1]); //chiudo il lato di scrittura;
                //Aggiungo alla lista dei processi quello appena generato, identificato dal suo pid
                //Devo aggiungere anche i fd per la pipe
                
                //DEBUG
                insertLast(dispList, pid, fd_reader[0],fd_writer[1]);
                
                //modifica Paolo --> Voglio far sì che il filedescriptor sia unico per la centralina --> qua devo fare il dup
                //insertLast(procList, pid, fd_reader[0], fd_writer[1]);
                /**
                 * TODO
                 * Nella funzione insertLast devo anche passare i FD cosi che vengano aggiunti al nodo
                 * */
                
                
                //Vado in pausa per permettere al figlio di generarsi
                pause();
                id+=1;



                //char msg[30];
                // int rev = read(fd[0],msg,512);
                // printf("\nesito: %d\n", rev);        
                // printf("\n\n\n\ndebug msg: %s\n\n\n",msg);
                
        }
        
        return 1;
}


