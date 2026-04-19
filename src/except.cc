#include <iostream>

#include "except.hh"
#include "stringutil.hh"

GAException::GAException(const std::string& file, 
			 const int line, 
			 const std::string& reason):
  std::runtime_error(reason),
  file_(file),
  line_(line)
{
  std::cerr << "Exception at " << file_ << ":" << tostring(line_)
            << ":" << what()
            << std::endl;
}
      
std::ostream& 
GAException::operator<<(std::ostream& ostr) const
{
   ostr << what(); 
   return ostr; 
}
