#ifndef _MAKE_DATA_
#define _MAKE_DATA_
#include <string>
#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <static_utilities.h>
#include <sys/stat.h>

namespace make_data
{
  class Data{
  public:
    Data(std::string fileName);
    void printFileName();
    void printDump();
    void setDebug();
    bool executeTargets(std::vector<std::string> targets);
  private:
    std::string fileName;
    class Target{
    public:
      Target();
      void setTimeStamp();//he
    private:
      std::string Name;
      long time_since_epoch;
      std::vector<Target> prerequisites;
      std::vector<std::string> commands;
      friend class Data;
      friend make_data::Data* parseFile(std::string fileName);
    };
    std::map<std::string, std::string> macros;
    std::vector<Target> targets;
    friend make_data::Data* parseFile(std::string fileName);
    bool DEBUG = false;
    bool recursiveMake(Target target, std::string spacing);
    Target* getTargetFromString(std::string target);
    bool needsUpdate(Target &t);
    int num_commands = 0, num_names = 0;
    bool parseMacros(std::string &s);
  };
  Data* parseFile(std::string fileName);
  bool isMacro(std::string s);
  bool isTarget(std::string s);
  bool isCommand(std::string s);
  void removeComments(std::string &s);
}

#endif
