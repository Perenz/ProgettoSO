#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "../strutture/comandiH.h"
int id_gen = 0;

int add_device(char* execPath, NodoPtr magazzino_list, char* nome);
int add_device_generale(char* execPath, NodoPtr list, info info, char* nome);
//devices list
char *builtin_device[]={
        "bulb",
        "window",
        "fridge",
        "timer",
        "hub"
};
//path devices
char *builtin_dev_path[]={
        "./binaries/BULB",
        "./binaries/WINDOW",
        "./binaries/FRIDGE",
        "./binaries/TIMER",
        "./binaries/HUB"
};

int device_number(){
        return sizeof(builtin_device)/sizeof(char*);
}


//add device generale che viene chiamato dai dispositivi di controllo per "aggiungere"
//un dispositivo che già esisteva (ergo con informazioni non di default)
int add_device_generale(char* execPath, NodoPtr list, info info, char* nome){
    
    //info verrà gestito da ogni 
    pid_t pid, wpid;
    int fd_reader[2];
    int fd_writer[2];

    pipe(fd_reader); //creo la pipe
    pipe(fd_writer);
    pid=fork();
    if(pid == 0) {     

        char fd_writer_Tmp[10];
        char fd_reader_Tmp[10];
        //chiudo i fd delle pipe che non verranno utilizzate
        close(fd_reader[0]);
        close(fd_writer[1]);
        sprintf(fd_writer_Tmp,"%d", (fd_writer[0]));
        sprintf(fd_reader_Tmp,"%d", (fd_reader[1]));
        //creo il figlio passandogli i file descriptor delle pipe utilizzate per la comunicazione e il nome di tale dispositivo
        char *args[]={execPath,fd_writer_Tmp, fd_reader_Tmp, nome, NULL}; 
        execvp(args[0],args); //passo gli argomenti incluso il puntatore al lato di scrittura della pipe
    } else if (pid < 0) {
        perror("errore fork");
    } 
    else{
        //chiudo i fd delle pipe che non verranno utilizzate
        close(fd_reader[1]);
        close(fd_writer[0]);
        //comunico al figlio appena creato le informazioni (ricorda che siamo in add_generale e che quindi il dispositivo già esisteva precedentemente, nel caso in cui esso già esisteva ci 
        //sara informazioni.def == 1 per comunicare che esse sono le info di default)
        int err = write(fd_writer[1],&info,sizeof(info));
        if(err == -1) 
            printf("Errore scrittura pipe\n");
        if(err == -1) 
            printf("Errore scrittura pipe\n");
        //inserisco nella lista dei figli il dispositivo appena aggiuto
        list = insertLast(list, pid, fd_reader[0],fd_writer[1]);
        //Vado in pausa per permettere al figlio di generarsi
        pause();        
    }
    return 1;
}
//add device di default chiamato dalla centralina quando viene aggiunto un dispositivo
int add_device(char* execPath, NodoPtr magazzino_list, char* nome){
    //id_gen+=1;
    info infoD;
    //info default = 1, fa eseguire al figlio il codice iniziale di aggiunta, significa che un dispositivo nuovo e non già esistente e successivamente linkato
    infoD.def = 1; 
    infoD.id = id_gen;
    strcpy(infoD.nome,nome);
    
    add_device_generale(execPath, magazzino_list, infoD, nome);
    return 1;
}


