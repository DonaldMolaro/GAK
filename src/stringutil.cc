#include <stdio.h>
#include "stringutil.hh"

std::string tostring(int c)
{
  char buffer[128];
  bzero(buffer,128);
  snprintf(buffer,sizeof(buffer),"%d",c);
  return std::string(buffer);
}
