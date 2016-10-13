/*
  procQueue.h
  Shawn Stone
  Date: September 26, 2015

  Description: Header file for background process queue.

  The Background process queue contains an array of type pentry. Which is
  defined in this file.

*/
#ifndef PROCQUEUE_H
#define PROCQUEUE_H

typedef struct myProc{
  int size; /*size of pid int array*/
  int* pid;
  char* cmd;
}pentry;

/*Global queue for shell program*/
extern pentry* procQueue[100]; 

/*Declarations*/
void allocateQueue();
void printProcEntryStarted(int index);
void printProcEntryExited(int index);
int queueInsert(int* pid, char** cmd, int numPids);
int getProcIndex(int pid, int* is_last);
void updateProcQueue();
void removeProcFromQueue(int index);
int anyProcessRunning();
void killZombies();
void freeQueue(pentry** q);
#endif

