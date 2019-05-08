#ifndef __LIST_H__
#define __LIST_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Nodo{
    int data;
    int fd[2];
    struct Nodo* next;
}Nodo;


typedef Nodo* NodoPtr;

NodoPtr listInit(int);
void printList(NodoPtr);
NodoPtr insertLast(NodoPtr, int, int[2]);
void freeList(NodoPtr);

#endif