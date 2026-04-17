#if !defined(STRINGUTIL_H)
#define STRINGUTIL_H

#include <string>

const std::string recordSep = "*";
const std::string fieldSep = "^";

int find_instance(std::string candidate, int instance, std::string target);

std::string executing_directory(char *argv_0);
std::string token(std::string candidate,int instance, std::string separator);
std::string tostring(int c);

#endif
