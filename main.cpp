#include <static_utilities.h>
#include <make_data.h>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <signal.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <pthread.h>

void signalHandler(int param);
void timerHandler(int secs);
make_data::Data* data;
int main(int argc, char* argv []){
  int error = 0;
  std::string err_mesg;
  std::string fileName = "";
  std::list<std::string> options;
  std::vector<std::string> targets;
  std::thread* timer_thread = NULL;
  bool output_only = false, interruptable = true;
  int secs = -1;
  
  for(int i = 1; i < argc; ++i){
    std::string arg = argv[i];
    if(arg == "-f"){
      ++i; // skip arguument parameter
      fileName = argv[i];
      if(!utilities::file_exists(fileName)){
	error = 1;
	err_mesg = "Makefile " + fileName + " does not exist.";
      }
    }
    else if(arg == "-p"){
      output_only = true;
    }
    else if(arg == "-k"){
      options.push_front("k");
    }
    else if(arg == "-d"){
      options.push_front("d");
    }
    else if(arg == "-i"){
      interruptable = false;
    }
    else if (arg == "-t"){
      ++i;
      arg = argv[i];
      secs = std::stoi(arg);
      timer_thread = new std::thread(timerHandler, secs);
    }
    else{
      targets.push_back(arg);
    }
  }
  if(!error){
    //continue execution
    if(!interruptable) signal(SIGINT, SIG_IGN);
    else signal(SIGINT, signalHandler);

    data = make_data::parseFile(fileName);
    if(!output_only){
      for(auto const &s : options){
	std::cout << "Options: " << s << std::endl;
	if(s == "d") data->setDebug();
	else if(s == "k") data->setIgnoreFailedCommand();
      }
      data->executeTargets(targets);
    }
    else{
      data->printDump();
    }
  }
  else{
    std::cout << err_mesg << std::endl;
  }
  if(data != NULL){delete(data); data = NULL;}
  if(timer_thread != NULL) delete(timer_thread);
  return 0;
}
void timerHandler(int secs){
  std::this_thread::sleep_for(std::chrono::seconds(secs));
  std::cout << "Times up!" << std::endl;
  signalHandler(-234);  
}
void signalHandler(int param){
  if(param != -234)
    std::cout << "\nmymake: *** Interrupt" << std::endl;
  //call cleanup function here
  if(data != NULL){ delete(data); data = NULL;}
  exit(0);
}
