/*
  shell_tools.h
  Shawn Stone
  Date: August 31, 2015

  Description: Header file for Shell program

*/
#ifndef _SHELL_TOOLS_H_
#define _SHELL_TOOLS_H_
//#define _POSIX_SOURCE /*Required for kill()*/
//#define _DEFAULT_SOURCE /*Required for setenv()*/
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>/*for strerror*/
#include <stdio.h>
#include <stdlib.h>

extern unsigned int LINE_SIZE;
extern int error_thrown;

void freeArray(char** a); /*'Free char**' */
void throwError();/*set error_thrown to true. Not currently in use*/
int isspecial(char c);/*Used in parse_whitespace to tokenize special chars*/
char * parse_whitespace(char* line);/*Remove Trailing/Leading/To much space*/
char** parse_arguments(char* line);/*Create command and arg array*/
/*Used to parse env var for use in getenv()*/
char* getEnvVar(char* arg, int* index);
char** expand_variables(char** args);
int isInternalCommand(char * cmd);
int is_rfile(char* path);
int is_dir(const char* path);
char* resolve_command(char* cmd);/*Used in resolve_paths*/
char** resolve_paths(char** args);
void setFDs(int* fdin, int* fdout, int cmdNum, int pipe_count, int* pipe_index,
	    char** cmd);/*Used in execute to set I/O FDs*/
int maxProc();

#endif
