/*
  internal_commands.c
  Shawn Stone
  September 26, 2015

  Description: This file contains implementations of internal_commands.h 


*/

#include <shell_tools.h>
#include <stdio.h> /*used for snprintf*/

void echo(char** cmd){
  int i = 0;
  while(cmd[i] != NULL && *cmd[i] != '|'){
    printf("%s ", cmd[i]);
    ++i;
  }
  printf("\n");
}
void cd(char** cmd){
  int i = 0;
  char* env_var = getenv("HOME"), *newValue;
  while(cmd[i] != NULL){
    ++i;
  }
  if(i > 1){
    printf("cd: To Many Arguments\n");
  }
  else if(i == 1){
    if(is_dir(cmd[0])){
      newValue = (char*)calloc(strlen(cmd[0]) + 1,sizeof(char));
      strcpy(newValue, cmd[0]);
      setenv("PWD", newValue, 1);
      chdir(newValue);
    }
    else{
      printf("%s: Directory not found.\n", env_var);
    }
  }
  else if(i == 0){
    if(is_dir(env_var)){
      setenv("PWD", env_var, 1);
      chdir(env_var);
    }
    else{
      printf("%s: Directory not found.\n", env_var);
    }
  }
  else{
    printf("cd : failed to execute command\n");
  }
}
void limits(char** cmd, pid_t pid){
  FILE* fp;
  char file[100], buffer[100];
  int pid_i = (int) pid;

  if(cmd == NULL || cmd[0] == NULL || *cmd[0] == '\0'){
    printf("No argument supplied to limits.\n");
    return;
  }
  sprintf(file, "/proc/%i/limits", pid_i);
  if(is_rfile(file)){
    fp = fopen(file, "r");
    sleep(1);
    printf("\n");
    while(fgets(buffer, 100, fp) != NULL){
      fflush(fp);
      printf("%s", buffer);
    }
    fclose(fp);
  }
  else{
    printf("limits: %s is not a regular file\n", file);
  }
  /*print info*/
}
void execute_internal(char** cmd, pid_t pid){
  if(strcmp(cmd[0], "echo") == 0){
    echo(cmd + 1);
  }
  else if(strcmp(cmd[0], "cd") == 0){
    cd(cmd + 1);
  }
  else if(strcmp(cmd[0], "limits") == 0){
    limits(cmd + 1, pid);
  }
  else if(strcmp(cmd[0], "etime") == 0){
    
  }
  else{
    printf("%s: Internal command not found", cmd[0]);
  }
}

