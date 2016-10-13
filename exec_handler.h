#ifndef _EXEC_HANDLER2_
#define _EXEC_HANDLER2_
#include <string>

namespace make_data{
  class exec_handler{
  public:
    exec_handler(std::string cmd);
    ~exec_handler();
    int executeCommand();
    void abort();
    
  private:
    char** command;
    char* original_line;
    void cleanup();
    char** parse(char** line);
    int execute();
  };
}

#endif
