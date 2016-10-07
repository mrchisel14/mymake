
#include <iostream>
#include <fstream>

int main(int argc, char* argv []){
  if(argc == 1){
    //check for makefile or Makefile and build first target
  }
  else{
    for(int i = 1; i < argc; ++i){
      if(argv[i] == "-f"){
	++i; // skip arguument parameter
	//read file
	std::ifstream makeFile(argv[i])); 
      }
      else{
	printf("Time to build target %s\n", argv[i]);
      }
    }
  }
  return 0;
}
