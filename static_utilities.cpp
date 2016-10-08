#include <static_utilities.h>

namespace utilities
{
  bool file_exists (const std::string& name){
    if(std::ifstream(name)){
      return true;
    }
    else{
      return false;
    }
  }
}
