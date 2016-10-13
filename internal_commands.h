/*
  internal_commands.h
  Shawn Stone
  September 26, 2015

  Description: This file contains definitions for internal commands

*/
#ifndef INTERNAL_COMMANDS_H
#define INTERNAL_COMMANDS_H

void echo(char** cmd);
void cd(char** cmd);
void limits(char** cmd, pid_t pid);
void execute_internal(char** cmd, pid_t pid);
#endif
