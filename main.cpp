#include <static_utilities.h>
#include <make_data.h>
#include <iostream>
#include <fstream>
#include <string>
#include <list>

int main(int argc, char* argv []){
  int error = 0;
  std::string err_mesg;
  std::string fileName = "";
  std::list<std::string> options;
  std::vector<std::string> targets;
  bool output_only = false;
  int seconds = -1;
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
      options.push_front("i");
    }
    else if (arg == "-t"){
      ++i;
      arg = argv[i];
      seconds = std::stoi(arg);
    }
    else{
      targets.push_back(arg);
    }
  }
  if(!error){
    //continue execution
    make_data::Data* data;
    data = make_data::parseFile(fileName);
    if(!output_only){
      for(auto const &s : options){
	std::cout << "Options: " << s << std::endl;
	if(s == "d") data->setDebug();
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

  return 0;
}
