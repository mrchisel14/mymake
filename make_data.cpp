#include <make_data.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>


namespace make_data
{
  Data::~Data(){
    this->abort();
  }
  Data* parseFile(std::string fileName){
    if(fileName == ""){
      if(utilities::file_exists("mymake1.mk")){
	fileName = "mymake1.mk";
      }
      else if(utilities::file_exists("mymake2.mk")){
	fileName = "mymake2.mk";
      }
      else if(utilities::file_exists("mymake3.mk")){
	fileName = "mymake3.mk";
      }
      else if(utilities::file_exists("Makefile")){
	fileName = "Makefile";
      }
      else if(utilities::file_exists("makefile")){
	fileName = "makefile";
      }
      else{
	//error no file found
	return NULL;
      }
    }
    std::ifstream input(fileName);
    make_data::Data* result = new make_data::Data(fileName);
    Data::Target target;
    while(!input.eof()){
      std::string current_line;
      std::getline(input, current_line);
      removeComments(current_line);
      if(current_line == "") continue;
      if(isMacro(current_line)){
	current_line.replace(current_line.find("="), 1, " "); //Replace equal sign with space to tokenize
	std::istringstream iss(current_line);
	std::string macro;
	iss >> macro;
	std::string value;
	std::getline(iss, value);
	if(value.back() == ' '){
	  value.pop_back();
	}
	if(value.front() == ' '){
	  value = value.substr(1);
	}
	result->macros[macro] = value;
	std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
	    std::istream_iterator<std::string>{}};
	if(tokens.size() > 2){
	  	result->macros[tokens[0]] = tokens[1];
	}
      }
      else{
	if(isTarget(current_line)){
	  current_line.replace(current_line.find(":"), 1, " "); //Replace colon with space to tokenize
	  std::istringstream iss(current_line);
	  std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
	      std::istream_iterator<std::string>{}};

	  if(target.Name != "") result->targets.push_back(target);
	  target.Name = tokens[0];
	  if(utilities::file_exists(target.Name)){
	    target.setTimeStamp();
	  }
	  target.prerequisites.clear();
	  target.commands.clear();
	  tokens.erase(tokens.begin());
	  for(auto const &s : tokens){
	    Data::Target preReq;
	    preReq.Name = s;
	    if(utilities::file_exists(preReq.Name)){
	      //set time stamp
	      preReq.setTimeStamp();
	    }

	    target.prerequisites.push_back(preReq);
	  }
	}
	else if(isCommand(current_line)){
	  if(target.Name != ""){
	    ++(result->num_commands);
	    current_line = current_line.substr(1); //remove tab
	    target.commands.push_back(current_line);
	  }
	  else{
	    //unexpected command
	  }
	}
      }//end while
    }
    if(target.Name != "") result->targets.push_back(target);
    input.close();
    return result;
  }
  void removeComments(std::string &s){
    std::size_t pos;
    if((pos = s.find("#")) != std::string::npos){
      s.erase(pos);
    }
  }
  bool isMacro(std::string s){
    size_t result = s.find("=");
    if(result == std::string::npos){
      return false;
    }
    else{
      return true;
    }
  }
  bool isTarget(std::string s){
    size_t result = s.find(":");
    if(result == std::string::npos){
      return false;
    }
    else{
      return true;
    }
  }
  bool isCommand(std::string s){
    return (s[0] == '\t');
  }
  
  Data::Data(std::string fileName){
    this->fileName = fileName;
  }
  void Data::printFileName(){
    std::cout << fileName << std::endl;
  }
  void Data::printDump(){
    std::cout << macros.size() << " macros, " << targets.size() << " target rules, " << "0 .inf rules, " << num_names << " names, " << num_commands
	      << " cmds" << std::endl; 
    for(auto const &m : this->macros){
      std::cout << m.first << " = " << m.second << std::endl;
    }

    for(auto const &tt : this->targets){
      std::cout << tt.Name << ": ";
      for(auto const &t : tt.prerequisites){
	std::cout << t.Name << " ";
      }
      std::cout << std::endl;
      for(auto const &s : tt.commands){
	std::cout << "\t" << s << std::endl;
      }
    }
  }
  void Data::setDebug(){
    DEBUG = true;
  }
  bool Data::executeTargets(std::vector<std::string> targets){
    if(targets.size() == 0) targets.push_back("");
    
    for(auto const &t : targets){
      std::string target = t;
      if(target == ""){
	target = this->targets[0].Name;
      }
      Target* actual;
      actual = getTargetFromString(target);

      if(actual == NULL){
	std::cout << "No target " << target << " found." << std::endl;
	return false;
      }
      if(actual->Name[0] == '.'){
	//inference rule
	const char* actualPtr = actual->Name.c_str() + 1;
	std::vector<Target> inferredTargets;
	//find *.s1
	//create inferredTargets and prerequisites and commands
 	std::string sourceSuffix = "", targetSuffix = "";
	if(std::string(actualPtr).find('.') != std::string::npos){
	  //.s1.s2
	  sourceSuffix += ".";
	  while(*actualPtr != '.'){
	    sourceSuffix += *actualPtr;
	    ++actualPtr;
	  }
	  targetSuffix = std::string(actualPtr);
	}
	else{
	  //.s1
	  std::cout << "Actual string " << *actualPtr << std::endl;
	  sourceSuffix = std::string((actualPtr - 1));
	}

	DIR* dp;
	struct dirent* dirp;
	std::vector<std::string> files;
	std::string dir = "";
	char buf[1024];
	char* temp = getcwd(buf, 1024);

	if(temp == NULL){
	  std::cout << "Cannot handle paths larger than " << 1024 << " characters." << std::endl;
	  return false;
	}
	dir = std::string(temp);
	if((dp = opendir(dir.c_str())) == NULL){
	  std::cout << "Error \'" << errno << "\' opening directory: " << dir << std::endl;
	  return false;
	}
	else{
	  while((dirp = readdir(dp)) != NULL){
	    std::string file_name = std::string(dirp->d_name);
	    if(file_name.length() > sourceSuffix.length() &&
	       file_name.compare(file_name.length() - sourceSuffix.length(), sourceSuffix.length(),
				 sourceSuffix) == 0){
	      files.push_back(file_name);
	    }
	  }
	  for(auto const &f : files){
	    std::string target_str = f, prereq = f;
	    Target new_target;
	    size_t periodPos = f.rfind(".");
	    target_str.erase(periodPos, sourceSuffix.length());
	    std::string targetMinusSuffix = target_str;
	    target_str += targetSuffix;
	    new_target.Name = target_str;
	    Target temp;
	    temp.Name = prereq;
	    new_target.prerequisites.push_back(temp);
	    for(auto &s : actual->commands){
	      //edit commands
	      std::string t1 = s;
	      size_t loc;
	      while((loc = t1.find("$@")) != std::string::npos){
		std::string parsed_command = t1.replace(loc, 2, targetMinusSuffix);
	      }
	      while((loc = t1.find("$<")) != std::string::npos){
		std::string parsed_command = t1.replace(loc, 2, f);
	      }
	      new_target.commands.push_back(t1);
	    }
	    inferredTargets.push_back(new_target);
 	  }
	}
	//loop through each target
	/*	for(auto const &t : inferredTargets){
	  std::cout << t.Name << " : ";
	  for(auto const &p : t.prerequisites){
	    std::cout << p.Name << " ";
	  }
	  std::cout << std::endl;
	  for(auto const &c : t.commands){
	    std::cout << "\t" << c << std::endl;
	  }
	  }*/
	for(auto const &tar : inferredTargets){
	  
	  bool shouldExecute = false;
	  if(!utilities::file_exists(tar.Name)){
	    shouldExecute = true;
	  }
	  //check if needs updating
	  if(!shouldExecute && needsUpdate(tar)){
	    shouldExecute = true;
	  }
	  else if(!shouldExecute){
	    std::cout << "mymake: \'" << actual->Name << "\' is up to date." << std::endl;
	  }
	  if(shouldExecute){
	    std::vector<std::string> completed;
	    if(recursiveMake(tar, "", completed, "")){
	
	    }
	  }      
	}
      }
      else{
	bool shouldExecute = false;
	if(!utilities::file_exists(actual->Name)){
	  shouldExecute = true;
	}
	//check if needs updating
	if(!shouldExecute && needsUpdate(*actual)){
	  shouldExecute = true;
	}
	else if(!shouldExecute){
	  std::cout << "mymake: \'" << actual->Name << "\' is up to date." << std::endl;
	}
	if(shouldExecute){
	  std::vector<std::string> completed;
	  if(!recursiveMake(*actual, "", completed, "")){
	    std::cout << this->fileName << ": Recipe for target \'" << actual->Name << "\' failed."
		      << std::endl; 
	  }
	}      
      }
    }
    return false;
  }
  bool Data::recursiveMake(Target target, std::string spacing, std::vector<std::string> &completedTargets,
			   std::string previousTarget){
    if(DEBUG)
      std::cout << spacing << "making " << target.Name << std::endl;
    bool valid = true, shouldExecute = false;

    //check circular
    std::vector<std::string>::iterator it = std::find(completedTargets.begin(), completedTargets.end(),
						      target.Name);
    if(it != completedTargets.end()){
      std::cout << "mymake: Circular Target " << previousTarget << " <- " << target.Name
		<< " dependency dropped." << std::endl;
      return true;
    }
    completedTargets.push_back(target.Name);
    //check if exists
    if(!utilities::file_exists(target.Name)){
      shouldExecute = true;
    }
    //check if needs updating
    else if(needsUpdate(target)){
      shouldExecute = true;
    }
    if(shouldExecute){
      //recursive call for each prereq
      for(auto const &p : target.prerequisites){
	Target* actual = getTargetFromString(p.Name);
	if(actual == NULL){
	  //unable to find target
	  std::cout << "mymake: *** No rule to make target \'" << p.Name << "\', needed by \'" <<
	    target.Name << "\'. Stop." << std::endl;
	  valid = false;
	}
	else{
	  valid = recursiveMake(*actual, spacing + '\t', completedTargets, target.Name);
	}
      }
      if(valid){
	//execute commands
	for(auto &c : target.commands){
	  if(parseMacros(c)){
	    std::cout << c << std::endl;
	    this->commandHandler = new exec_handler(c);
	    int result = commandHandler->executeCommand();
	    delete(commandHandler);
	    commandHandler = NULL;
	    if(result == 1 && !this->ignoreFailedCommand){
	      valid = false;
	      break;
	    }
	    
	  }
	  else{
	    valid = false;
	  }
	}
      }
    }

    if(DEBUG)
      std::cout << spacing << "done making " << target.Name << std::endl;

    return valid;
  }
  Data::Target* Data::getTargetFromString(std::string target){
    for(auto &t : this->targets){
      if(t.Name == target)
	return &t;
    }
    return NULL;
  }
  bool Data::needsUpdate(const Target &t){
    bool result = false;
    for(auto const &p : t.prerequisites){
      if(p.time_since_epoch > t.time_since_epoch){
	result = true;
	break;
      }
      Target* actual = getTargetFromString(p.Name);
      if(actual == NULL){
	//already been checked ^^
      }
      else{
	result = needsUpdate(*actual);
	if(result) break;
      }
    }
    return result;
  }
  bool Data::parseMacros(std::string &s){
    bool success = true;
    if(this->macros.size() > 0){
      if(s.find("$") != std::string::npos){
	std::istringstream iss(s);
	std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
	s = "";
	for(auto &t : tokens){
	  if(t[0] == '$'){
	    std::string stripped_string = t;
	    stripped_string = stripped_string.substr(1); //remove $ sign

	    if(stripped_string[0] == '('){
	      stripped_string = stripped_string.substr(1);
	    }
	    std::size_t close;
	    close = stripped_string.find(")");
	    if(close != std::string::npos){
	      stripped_string.replace(close, 1, " ");
	    }
	    else{
	      close = stripped_string.find(".");
	      if(close != std::string::npos){
		stripped_string.replace(close, 1, " .");
	      }
	    }
	    std::istringstream isss(stripped_string);
	    std::vector<std::string> tokens2{std::istream_iterator<std::string>{isss}, std::istream_iterator<std::string>{}};
	    std::string macro = tokens2[0], suffix = "", prefix = "";
	    if(tokens2.size() > 1) suffix = tokens2[1];
	    if(this->macros.count(macro) > 0){
	      prefix = this->macros[macro];
	      t = prefix + suffix;
	    }
	    else{
	      std::cout << "Unable to find macro " << macro + suffix << "." << std::endl;
	      success = false;
	      break;
	    }
	  }
	  s += t + " ";
	}
      }
    }
    return success;
  }
  Data::Target::Target(){
    
  }
  void Data::Target::setTimeStamp(){
    struct stat st;
    int error = stat(this->Name.c_str(), &st);
    if(error != 0){
      //error reading file
    }
    else{
      this->time_since_epoch = st.st_mtime;
    }
  }
  void Data::abort(){
    if(this->commandHandler != NULL){
      delete(this->commandHandler);
    }
  }
  void Data::setIgnoreFailedCommand(){
    this->ignoreFailedCommand = true;
  }
}
