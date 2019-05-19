#include "listH.h"

NodoPtr listInitStart(){
    NodoPtr toRtn = malloc(sizeof(Nodo));
    toRtn->data = 0;
    toRtn->next = NULL;
    return toRtn;
}

NodoPtr listInit(int data){
    NodoPtr toRtn = malloc(sizeof(Nodo));
    toRtn->data=data;
    toRtn->next=NULL;

    return toRtn;
}
void printList(NodoPtr list){
    while(list != NULL){
        printf("%d ", list->data);   
        list=list->next;
    }
    printf("\n");
}
//Dealloco tutta ma meoria allocata dinamicamente per i nodi della lista
void freeList(NodoPtr list){
    NodoPtr node;
    while(list!=NULL){
        node=list;
        list=list->next;
        free(node);
    }
}

NodoPtr insertLast(NodoPtr list, int data, int fd_reader,int fd_writer){
    if(list==NULL){
        NodoPtr toRtn = listInit(data);
        toRtn->fd_reader=fd_reader;
        toRtn->fd_writer=fd_writer;
        toRtn->linked = 0;
        return toRtn;
    }
    
    //Scorro la lista fino all'ultima posizione
    NodoPtr n = list;
    while (n->next != NULL)
    {
        n = n->next;
    }
    
    //Inserisco il nuovo nodo in ultima posizione
    NodoPtr tmp = listInit(data);
    tmp->fd_reader=fd_reader;
    tmp->fd_writer=fd_writer;
    tmp->linked = 0;
    n->next=tmp;

    return list;
}


//da sistemare
void removeNode(NodoPtr list, int data){

        ////////da sistemare, non toglie il primo, per ora risolvo facendo che il primo ha 0 e o bona


    if(list != NULL){//controllo che la lista non sia vuota
        NodoPtr nodo = list;
        NodoPtr tmp = nodo->next;
        if(tmp == NULL){//La lista ha un elemento
            /*
            if(list->data == data){
                list->data = 0;
                return;
            }
            */
        }else{//la lista ha più di un elemento
        /*
            if(list->data == data){//il primo elemento è quello da togliere

            }*/
            while(tmp != NULL){
                if(tmp->data == data){
                    
                    nodo->next = tmp->next;
                    free(tmp);
                    return;
                }
                nodo = nodo->next;
                tmp = tmp->next;
            }
            
        }

    }

}




void spostaNode(NodoPtr listSrc, NodoPtr listDest, Nodo obj){
    removeNode(listSrc, obj.data);
    insertLast(listDest, obj.data, obj.fd_reader, obj.fd_writer);
}

int getNode(NodoPtr list, int pid, Nodo* nodo_return){
    NodoPtr toRtn = NULL;
    NodoPtr tmp = list;
    while(tmp != NULL){
        if(pid == tmp->data){
            nodo_return->data = tmp->data;
            nodo_return->fd_reader = tmp->fd_reader;
            nodo_return->fd_writer = tmp->fd_writer;
            return 1;
        }
        tmp = tmp->next;
    }
    return -1;

}
