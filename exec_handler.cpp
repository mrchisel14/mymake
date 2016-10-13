#include <exec_handler.h>
#include <internal_commands.h>
#include <shell_tools.h>
#include <procQueue.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <vector>
#include <sstream>
#include <iterator>
#include <errno.h>

namespace make_data{
  exec_handler::exec_handler(std::string cmd){
    char* new_line = (char*)calloc(cmd.length() + 1 , sizeof(char));
    allocateQueue();
    cmd.copy(new_line, cmd.length(), 0);
    this->original_line = new_line;
    this->command = parse(&(this->original_line));
  }
  exec_handler::~exec_handler(){
    this->cleanup();
  }
  int exec_handler::executeCommand(){
    return this->execute();
  }
  void exec_handler::abort(){
    this->cleanup();
  }
  char** exec_handler::parse(char** line){
    char** args = 0;
    *line = parse_whitespace(*line);
    args = parse_arguments(*line);
    if(args == NULL || error_thrown){
      error_thrown = 1;
      return NULL;
    }
    args = expand_variables(args);
    if(args == NULL){
      error_thrown = 1;
      return NULL;
    }
    args = resolve_paths(args);

    return args;
    
  }
  int exec_handler::execute(){
    char** cmd = this->command;
    int status, i = 0, fdin = -1, fdout = -1, pipe_count = 0, j = 0, pipe_fd[2], result = 0;
    int old_in = STDIN_FILENO, pipe_index = 0, bg = 0, *bg_pids, 
      limit_or_etime = 0;
    pid_t pid;
    struct timeval tv[2];
    double seconds, microsec;
    char* temp = NULL, **pholder = cmd;

    /*count pipes*/
    while(cmd[i] != NULL){
      if(*cmd[i] == '|'){ 
	++pipe_count;
      }
      ++i;
    }
    i = 0;
    if(pipe_count > maxProc()){
      printf("You are attempting to run to many pipes\n");
      error_thrown = 1;
      return 0;
    }
    /*handle background processes*/
    while(cmd[i] != NULL){
      if(i == 0 && *cmd[i] == '&') ++cmd;
      if(cmd[i] != NULL && *cmd[i] == '&' && cmd[i+1] != NULL){
	printf("Invalid background process\n");
	error_thrown = 1;
	return result;
      }
      if(cmd[i+1] == NULL && *cmd[i] == '&'){
	/*set background process*/
	bg = 1;
	free(cmd[i]);
	cmd[i] = NULL;
	bg_pids = (int*)calloc(pipe_count + 1, sizeof(int));
      }
      ++i;
    }
    i = 0;
  
    for(j = 0; j <= pipe_count; ++j){
      pipe_index = 0;
      limit_or_etime = 0;
      if(strcmp(cmd[0], "cd") == 0){
	execute_internal(cmd, 0);
	break;
      }
      if(strcmp(cmd[0], "limits") == 0 || strcmp(cmd[0], "etime") == 0 ){
	gettimeofday(&tv[0], NULL);
	limit_or_etime = 1;
	++cmd;
      }
      setFDs(&fdin, &fdout, j, pipe_count, &pipe_index, cmd);

      if(pipe_index > 0){
	temp = cmd[pipe_index];
	cmd[pipe_index] = NULL;
      }
    
      /*handle pipe*/
      if(pipe_count > 0)
	pipe(pipe_fd);
      pid = fork();
      if(pid == -1){
	printf("%s: Failed executing command.\n", cmd[0]);
	exit(1);
      }
      else if(pid == 0){
	if(bg) setpgid(0, 0);
	if(fdin > -1){
	  close(STDIN_FILENO);
	  dup(fdin);
	  close(fdin);
	}
	if(fdout > -1){
	  close(STDOUT_FILENO);
	  dup(fdout);
	  close(fdout);
	}
	if(pipe_count > 0){        
	  /*read pipe input*/
	  if(j != 0){
	    close(0);
	    if(dup(old_in) < 0){
	      error_thrown = 1;
	      printf("%s\n", strerror(errno));
	    }
	    close(old_in);
	  }
	  if(j != pipe_count){
	    /*pipe output to next command*/
	    close(1);
	    if(dup(pipe_fd[1])){
	      error_thrown = 1;
	      printf("%s\n", strerror(errno));
	    }
	  }
	  close(pipe_fd[0]);
	  close(pipe_fd[1]);
	}
	if(bg && j == pipe_count && j != 0)waitpid(bg_pids[j-1], 0 ,0);
	if(isInternalCommand(cmd[0])){
	  execute_internal(cmd, pid);
	  exit(0);
	}
	else{
	  execv(cmd[0], cmd);
	}
	printf("make: execv: %s: %s\n", cmd[0], strerror(errno));
	exit(1);
      }
      else{
	if(limit_or_etime == 1){
	  --cmd;
	  execute_internal(cmd, pid);
	}
	if(bg){
	  bg_pids[j] = pid;
	  if(j == pipe_count)
	    {
	      queueInsert(bg_pids, pholder, pipe_count + 1);
	    }
	}
	else{
	  waitpid(0, &status, 0);
	  if(WIFEXITED(status)){
	    if(WEXITSTATUS(status) == 1){
	      result = 1;
	      break; 
	    }
	  }
	  if(strcmp(cmd[0], "etime") == 0){
	    gettimeofday(&tv[1], NULL);
	    seconds = tv[1].tv_sec - tv[0].tv_sec;
	    microsec = tv[1].tv_usec - tv[0].tv_usec;
	    if(microsec < 0) {
	      seconds -= 1;
	      microsec += 1 * 1000000;
	    }
	    printf("%.2fs %.2fu\n", seconds, microsec);
	  }
	}
	if(temp != NULL && pipe_index > 0)
	  cmd[pipe_index] = temp;
	while(*cmd != NULL && **cmd != '|') ++cmd;
	++cmd;
	if(pipe_count > 0){
	  close(pipe_fd[1]);
	  if(j!=0) close(old_in);
	  old_in = pipe_fd[0];
	}

      }
    }/*end for pipe count loop*/
    if(pipe_count > 0){
      close(pipe_fd[0]);
      close(pipe_fd[1]);
    }
    return result;
  }

  void exec_handler::cleanup(){
    char** line = &(this->original_line);
    char*** cmd = &(this->command);
    if(*cmd != NULL) freeArray(*cmd);
    *cmd = NULL;
    if(*line != NULL) free(*line);
    *line = NULL;
    error_thrown = 0;
    while(anyProcessRunning()) killZombies();
    freeQueue(procQueue);
  }
}
 
