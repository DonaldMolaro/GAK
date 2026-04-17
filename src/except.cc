#include <string.h>
#include <iostream>
#include "except.hh"
#include "stringutil.hh"

GAException::GAException(const std::string& file, 
			 const int line, 
			 const std::string& reason):
  _reason(reason),
  _file(file),
  _line(line)
{
  std::cerr << "Exception at " << _file << ":" << tostring(_line) 
            << ":" << _reason 
            << std::endl;
};
      
std::ostream& 
GAException::operator<<(std::ostream& ostr) 
{
   ostr << what(); 
   return ostr; 
}
const char* 
GAException::what () const 
{
  return _reason.c_str();
}
