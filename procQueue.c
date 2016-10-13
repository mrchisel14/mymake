/*
  procQueue.h
  Shawn Stone
  Date: September 26, 2015

  Description: Header file for background process queue.

  The Background process queue contains an array of type pentry. Which is
  defined in this file.

*/

#include <shell_tools.h>
#include <procQueue.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/*Global queue for shell program*/
pentry* procQueue[100]; 
/**/

void allocateQueue(){
  int i = 0;
  for(; i < 99; ++i){
    procQueue[i] = (pentry*)calloc(1, sizeof(pentry));
    procQueue[i]->size = 0;
    procQueue[i]->pid = NULL;
    procQueue[i]->cmd = NULL;
  }
  procQueue[99] = NULL;
}
void printProcEntryStarted(int index){
  int size = procQueue[index]->size, i = 0;
  fflush(stdout);
  printf("[%i]\t", index);
  for(i = 0; i < size; ++i){
    printf("[%i]", procQueue[index]->pid[i]);    
  }
  printf("\n");
  
}
void printProcEntryExited(int index){
  fflush(stdout);
  printf("[%i]+\t[%s]\n", index, procQueue[index]->cmd);
}
int queueInsert(int* pid, char** cmd, int numPids){
  unsigned int index = -1, i = 0, size = 100, ncmd_size = 0;
  char* ncmd = (char*)calloc(size, sizeof(char)), *temp;
  while(cmd != NULL && *cmd != '\0'){
    ncmd_size = (*ncmd == '\0') ? 0 : strlen(ncmd);
    if(strlen(*cmd) + 1 + ncmd_size > size){
      temp = (char*)calloc(size*2, sizeof(char));
      size *= 2;
      strcpy(temp, ncmd);
      free(ncmd);
      ncmd = temp;
    } 
    strcat(ncmd, " ");
    strcat(ncmd, *cmd);
    ++cmd;
  }
  while(procQueue[i] != NULL){
    if(procQueue[i]->pid != NULL) ++i;
    else{
      index = i;
      procQueue[i]->size = numPids;
      procQueue[i]->pid = pid;
      procQueue[i]->cmd = ncmd;
      printProcEntryStarted(i);
      break;
    }
    ++i;
  }
  return index;
}
int getProcIndex(int pid, int* is_last){
  int i = 0, index = -1, j = 0, size = 0;
  while(procQueue[i] != NULL){
    size = procQueue[i]->size;
    for(j = 0; j < size; ++j){
      if(procQueue[i]->pid[j] == pid){
        if(j == size - 1)
          *is_last = 1;
        index = i;
      }
    }
    ++i;
  }
  return index;
}
void updateProcQueue(){
  int index = 0, pid, is_last = 0;
  while((pid = waitpid(-1, 0, WNOHANG)) != 0){
    if(pid == -1) break;
    index = getProcIndex(pid, &is_last);
    if(index == -1) break;
    if(is_last)
      printProcEntryExited(index);
      removeProcFromQueue(index);
  }
}
void removeProcFromQueue(int index){
  procQueue[index]->size = 0;
  free(procQueue[index]->pid);
  procQueue[index]->pid = NULL;
  free(procQueue[index]->cmd);
  procQueue[index]->cmd = NULL;
}
int anyProcessRunning(){
  int running = 0, i = 0;
  updateProcQueue();
  while(procQueue[i] != NULL){
    if(procQueue[i]->pid != NULL) running = 1;
    ++i;
  }
  return running;
}
void killZombies(){
  int i = 0, j;
  while(procQueue[i] != NULL){
    if(procQueue[i]->pid != NULL){
      for(j = 0; j < procQueue[i]->size; ++j){
        if(kill(procQueue[i]->pid[j], SIGKILL)){
          printf("%d: %s\n", procQueue[i]->pid[j], strerror(errno));
        }
        else
          updateProcQueue();
      }
    }
    ++i;
  }
}
void freeQueue(pentry** q){
  int i = 0;
  while(q[i] != NULL)
  {
    free(q[i]->pid);
    free(q[i]->cmd);
    free(q[i]);
    ++i;
  }
}
