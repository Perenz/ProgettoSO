#ifndef __LIST_H__
#define __LIST_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>




typedef struct Nodo{
    int data;
    int linked;
    //File descriptor in cui il padre scrive e i figli leggono
    int fd_writer;
    //File descriptor in cui il padre legge e i figli scrivono 
    int fd_reader;

    struct Nodo* next;
}Nodo;


typedef Nodo* NodoPtr;

NodoPtr listInit(int);
void printList(NodoPtr);
NodoPtr insertLast(NodoPtr list, int data, int fd_reader,int fd_writer);
void freeList(NodoPtr);
void removeNode(NodoPtr list, int data);

#endif