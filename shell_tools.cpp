/*
  shell_tools.c
  Shawn Stone
  Date: August 31, 2015

  Description: Implementation of shell_tools.h

*/
#include <shell_tools.h>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>

/*Global Variables*/
unsigned int LINE_SIZE = 50;/*Initial Line buffer Size*/
/*Sends error to main telling it to cleanup and then reset*/
int error_thrown = 0;

void freeArray(char** a){
  int i = 0;
  while(a[i] != NULL)
    {
      free(a[i]);
      ++i;
    }
  free(a);
  a = NULL;
}
void throwError(){
  error_thrown = 1;
}
int isspecial(char c){  
  return (c == '|' || c == '<' || c == '>' || c == '&');
}
char * parse_whitespace(char* line){
  char* pholder = line, *temp; /*Pholder shall point to beginning of line at
				 all times*/
  
  /*Remove to much whitespace between args and Add space for special chars*/
  while(*line != '\0'){
    if(isspace(*line)){
      if(isspace(*(line + 1))){
	*(line + 1) = '\0';
	line += 2;
	while(isspace(*line)){ 
	  ++line;
	}
	temp = (char*)calloc(LINE_SIZE + 1, sizeof(char));
	strcpy(temp, pholder);
	strcat(temp, line);
	free(pholder);
	pholder = temp;
	line = pholder;
      }
    }
    if(isspecial(*line)){  /*Add whitespace next to special characters*/
      char* nline;
      char c;/*used to save special char to copy into new line*/

      if((!isspace(*(line + 1)) && *(line + 1)!= '\0') || 
	 !isspace(*(line - 1))){
	/*add space after and before special char*/
	c = *line;
	nline = (char*)calloc(LINE_SIZE + 4, sizeof(char));
	*line = '\0';
	sprintf(nline, "%s %c %s", pholder, c, line + 1);
	*line = c;
	free(pholder);
	line = pholder = nline;
      }
    }
    ++line;
  }/*end while*/
  line = pholder;;

  /*Remove Lead White Space*/
  if(isspace(*line)){ 
    ++line;
    while(isspace(*line)) ++line;
    temp = (char*)calloc(strlen(line)+1, sizeof(char));
    strcpy(temp, line);
    free(pholder);
    line = temp;
    temp = 0;
  }
  pholder = line;

  /*Remove Trailing Whitespace*/
  line = pholder;
  line = line + strlen(line) - 1;
  while(line > pholder && isspace(*line)) --line;
  *(line + 1) = '\0';
  line = pholder;
  return line;
}
char** parse_arguments(char* line){
  char** args = NULL;
  std::string temp = std::string(line);
  std::istringstream iss(temp);
  std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
      std::istream_iterator<std::string>{}};
  unsigned int j = 0;
  args = (char**)calloc(tokens.size() + 1, sizeof(char*));
  for(j = 0; j < tokens.size(); ++j){
    args[j] = (char*)calloc(tokens[j].length() + 1, sizeof(char));
    tokens[j].copy(args[j], tokens[j].length(), 0);
  }
  args[j] = NULL;
  
  return args;
}
char* getEnvVar(char* arg, int* index){
  int i = 0, size = 10;
  char* env_var = (char*)calloc(size, sizeof(char)), *temp;
  
  while(arg[*index] != '\0' && arg[*index] != '$' && arg[*index] != '/'){
    if(i == size - 2){
      /*need to allocate more memory*/
      temp = (char*) calloc((size + 10), sizeof(char));
      size += 10;
      strncpy(temp, env_var, size - 1);
      free(env_var);
      env_var = temp;
      temp = 0;
    }
    env_var[i] = toupper(arg[*index]);
    ++(*index);
    ++i;
  }
  env_var[i] = '\0';
  
  return env_var;
}
char** expand_variables(char** args){
  int index = 0, i = 0, prefix_size;
  char* env_var, *suffix_beg;
  char* temp;

  while(args[i] != NULL){    
    while(args[i][index] != '\0'){
      if(args[i][index] == '$'){
	if(args[i][index + 1] != '\0'){
	  prefix_size = index;
	  ++index; /*Go to one after $*/
	  temp = getEnvVar( args[i], &index);
	  env_var = getenv((const char*) temp);

	  if(env_var != NULL){
	    /*expand Variable*/
	    suffix_beg = args[i] + index;
	    if(temp != 0){ 
	      free(temp);
	      temp = 0;
	    }
	    temp = (char*)calloc(strlen(args[i]) + strlen(env_var) + 1, 
				 sizeof(char));
	    /*remove forward slash in prefix*/
	    if (prefix_size > 0 && args[i][prefix_size - 1] == '/' && 
		env_var[0] == '/') 
	      prefix_size -= 1;
	    strncpy(temp, args[i], prefix_size);            
            
	    strcat(temp, env_var);
	    strcat(temp, suffix_beg);

	    free(args[i]);
	    args[i] = temp;
	    temp = 0;
	  }
	  else{           
	    printf("Undefined environmental variable $%s.\n", temp);
	    free(temp);
	    temp = 0;
	    freeArray(args);
	    return NULL;
	  }
	}
      }
      ++index;
    }
    index = 0;
    ++i;
  }
  i = 0;
  return args;
}
int isInternalCommand(char * cmd){
  return (strcmp(cmd, "cd") == 0 || strcmp(cmd, "exit") == 0 || 
	  strcmp(cmd, "echo") == 0 ||strcmp(cmd, "etime") == 0 ||
	  strcmp(cmd, "limits") == 0);
}
int is_rfile(char* path){/*Is path regular file?*/
  struct stat s;
  if(stat(path, &s) == 0){
    if(S_ISREG(s.st_mode)){
      return 1;
    }
  }
  return 0;
}
int is_dir(const char* path){
  struct stat s;
  if(stat(path, &s) == 0){
    if(S_ISDIR(s.st_mode)){
      return 1;
    }
  }
  return 0;
}
char* resolve_command(char* cmd){
  /*
    USED FOR EXTERNAL COMMANDS ONLY
    This function aside from providing checks. Creates a char array containing
    each path in the $PATH variable with the command appended to it. These are
    then checked until a regular file is found indicating the command
    exists. The command is then returned with it's full path name.

    Return Value: command string is returned on success NULL on failure.
  */
  char** paths;
  char* env_var = getenv("PATH"), *beg;
  unsigned int i, count_paths = 1, count_chars = 0, j = 0;

  if(is_rfile(cmd)){  
    return cmd;
  }
  if(cmd == NULL || *cmd == '\0' || *cmd == '|'){
    printf("Invalid null command\n");
    return NULL;
  }
  for(i = 0; i < strlen(env_var); ++i){
    if(env_var[i] == ':') ++count_paths;
  }
  paths = (char**)calloc(count_paths + 1, sizeof(char*));

  beg = env_var;
  for(i = 0; i < strlen(env_var) + 1; ++i){
    if(env_var[i] == ':' || env_var[i] == '\0'){
      paths[j] = (char*)calloc(count_chars + strlen(cmd)+ 2, sizeof(char));
      strncpy(paths[j], beg, count_chars);
      if(paths[j][strlen(paths[j])] != '/')
	strcat(paths[j], "/");
      strcat(paths[j], cmd);
      count_chars = 0;
      ++j;
      beg = &env_var[i+1];
    }
    else
      ++count_chars;
  }

  /*At this point we have the paths in $path in a char array with cmd appended*/
  for(i = 0; i < count_paths; ++ i){
    /*check if it is regular file or directory*/

    if(is_rfile(paths[i])){ /*is regular file?*/
      char* result = (char*)calloc(strlen(paths[i]) + 1, sizeof(char));
      strncpy(result, paths[i], strlen(paths[i]));
      free(cmd);
      freeArray(paths);
      return result;
    }
    else if(is_dir(paths[i])){/*This is used to make sure we do not execute a Dir*/
      printf("%s: Permission denied.\n", paths[i]);
      freeArray(paths);
      free(cmd);
      return NULL;
    }
  }

  /*If we make it to this point the external command was not found*/
  printf("%s: Command not found.\n", cmd);
  freeArray(paths);
  return NULL;
}
char** resolve_paths(char** args){
  int i = 0, j = 0, k = 0;
  char* expansion = 0, *temp = 0, *pholder;

  while(args[i] != NULL){
    /*expand pathnames*/

    /*Special instance do not resolve paths of args*/
    if(strcmp(args[i], "echo") == 0){
      while(args[i] != NULL && *args[i] != '|') ++i; /*go to next command*/
      continue;
    }

    /*Resolve ~ if needed*/
    if(args[i][0] == '~' && (args[i][j+1] == '/' || 
			     args[i][j+1] == '\0')){
      expansion = getenv("HOME");
      temp =(char*) calloc(strlen(expansion) + strlen(args[i]) + 2, 
			   sizeof(char));
      strcpy(temp, expansion);
      strcat(temp, args[i] + 1);
      free(args[i]);
      args[i] = temp;
      temp = 0;
    }
    pholder = args[i];
    
    /*Commands*/
    /*Internal commands leave as is. Will be handled in execute()*/
    if(i == 0 && strcmp(args[i], "echo") == 0){
      ++i;
      continue;        
    }
    if((i == 0 || args[i][0] == '|') && strcmp(args[i], "cd") != 0){
      /*resolve command in $PATH*/
      if(isInternalCommand(args[i])) ++i;
      if(args[i] == NULL) break;
      if(args[i] != NULL && *args[i] == '&') ++i;
      if(i != 0 && args[i][0] == '|') ++i;
      temp = resolve_command(args[i]);
      if(temp == NULL){
	throwError();
	return args;
      }
      else{
	args[i] = temp;
	temp = 0;
      }
    }
    while(args[i][j] != '\0'){
      if(args[i][j] == '.'){
	if(j == 0 && (args[i][j+1] == '/' || args[i][j+1] == '\0' || 
		      args[i][j+1] == '.')){
	  /*evaluate PWD*/
	  expansion = getenv("PWD");
	  temp = (char*)calloc(strlen(expansion) + 1, sizeof(char));
	  strcpy(temp, expansion);
	  expansion = temp;
	}
	else{
	  /*special case do not evaluate PWD*/
	  expansion = NULL;
	}
	if(j > 0 && args[i][j-1] != '/') {++j; continue;}
	if(args[i][j+1] == '.'){
	  /*go to parent*/
	  if(expansion == NULL){
	    expansion = (char*) calloc(j + 1, sizeof(char));
	    strncpy(expansion, args[i], j - 1);
	    expansion[j+1] = '\0';
	  }
	  temp = expansion;
	  temp += strlen(expansion) - 1;
	  while(temp[k] != '/' && temp[k] != '\0') --temp;
	  if(temp == expansion || temp == args[i]) temp[k+1] = '\0';
	  else temp[k] = '\0';
	  temp = 0;

	  args[i] += j + 2;
	}
	else if(expansion == NULL && args[i][j+1] == '/'){
	  /*remove ./*/
	  expansion = (char*) calloc(j, sizeof(char));
	  strncpy(expansion, args[i], j-1);
	  args[i] += j + 2;
	}
	else if(args[i][j+1] == '/'){
	  /*remove ./*/
	  args[i] += j + 1;
	}
	else if(expansion == NULL && args[i][j+1] == '\0'){
	  /*remove .*/
	  expansion = (char*) calloc(j, sizeof(char));
	  strncpy(expansion, args[i], j-1);
	  args[i] += j + 1;
	}
	if(expansion != NULL)
	  temp = (char*) calloc(strlen(args[i]) + strlen(expansion) + 1, 
				sizeof(char));
	else
	  temp = (char*) calloc(strlen(args[i]) + 1, sizeof(char));
	if(expansion != NULL)
	  strcpy(temp, expansion);
	strcat(temp, args[i]);
	args[i] = pholder;
	free(args[i]);
	if(expansion != NULL)
	  free(expansion);
	args[i] = temp;
	temp = 0;
	pholder = 0;
      }
      ++j;
    }/*end while*/
    j = 0;
    ++i;
  }/*end while*/
  return args;
}
void clean_command(char** cmd, int* index){
  int i = *index + 2;
  free(cmd[*index]);
  free(cmd[*index+1]);
  
  while(cmd[i] != NULL){
    cmd[i - 2] = cmd[i];
    ++i;
  }
  cmd[i - 2] = NULL;
  i = 0;
  *index -= 1;
}

void setFDs(int* fdin, int* fdout, int cmdNum, int pipe_count, int* pipe_index, 
	    char** cmd){
  int i = 0;
  *fdin = *fdout = -1;
  if(cmdNum == 0 && pipe_count > 0) *fdout = -2; /*do not allow output redirect*/
  else if(cmdNum > 0 && pipe_count > 0){
    if(cmdNum == pipe_count) *fdin = -2;
    else{
      *fdout = -2;
      *fdin = -2;
    }
  }
  if(cmdNum == 0 && pipe_count > 0) *fdout = -2; /*do not allow output redirect*/
  else if(cmdNum > 0 && pipe_count > 0){
    if(cmdNum == pipe_count) *fdin = -2;
    else{
      *fdout = -2;
      *fdin = -2;        
    }
  }
  i = 0;
  while(cmd[i] != NULL && *cmd[i] != '|'){
    if(*cmd[i] == '<'){
      /*handle input and output redirection*/
      if(*fdin != -1){
	error_thrown = 1;
	printf("Ambigous input redirect\n");
	return;
      }
      if(is_rfile(cmd[i+1])){
	*fdin = open(cmd[i + 1], O_RDONLY);
	if(*fdin == -1){
	  error_thrown = 1;
	  printf("%s:%s\n",cmd[i+1], strerror(errno));
	  return;          
	}
	clean_command(cmd, &i);
      }
      else{
	printf("%s: No such file\n", cmd[i+1]);
	error_thrown = 1;
	return;
      }
    }
    else if(*cmd[i] == '>'){
      if(*fdout != -1){
	error_thrown = 1;
	printf("Ambigous output redirect\n");
	return;
      }
      *fdout = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR 
		    | S_IRUSR);
      if(*fdout == -1){
	error_thrown = 1;
	printf("%s:%s\n",cmd[i+1], strerror(errno));
	return;          
      }
      clean_command(cmd, &i);
    }
    ++i;
  }
  if(cmd[i] != NULL && *cmd[i] == '|') *pipe_index = i;
}
int maxProc(){
  FILE* fp = fopen("/proc/sys/kernel/pid_max", "r");
  int max = 3;
  if(fp != NULL) fscanf(fp, "%d", &max);
  fclose(fp);;
  return max;
}

