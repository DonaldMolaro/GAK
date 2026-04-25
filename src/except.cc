#include <iostream>
#include <string>

#include "except.hh"

GAException::GAException(const std::string& file, 
			 const int line, 
			 const std::string& reason):
  std::runtime_error(reason),
  file_(file),
  line_(line)
{
  std::cerr << "Exception at " << file_ << ":" << std::to_string(line_)
            << ":" << what()
            << std::endl;
}
      
std::ostream& 
GAException::operator<<(std::ostream& ostr) const
{
   ostr << what(); 
   return ostr; 
}
