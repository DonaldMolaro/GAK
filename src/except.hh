#pragma once


#include <iosfwd>
#include <string>
#include <stdexcept>

class GAException : public std::runtime_error
{
 public:
  GAException(const std::string& file, 
	      const int line, 
	      const std::string& reason);
  std::ostream& operator<<(std::ostream& ostr) const;
  const std::string& file() const noexcept { return file_; }
  int line() const noexcept { return line_; }
 private:
  std::string file_;
  int line_;
};

class GAComplete : public GAException
{
 public:
  GAComplete(const std::string& file, 
	     const int line, 
	     const std::string& reason):
    GAException(file,line,reason) {}
};

class GAFatalException : public GAException 
{
 public:
  GAFatalException(const std::string& file, 
		   const int line, 
		   const std::string& reason):
    GAException(file,line,reason) {}
};

class GANonFatalException : public GAException 
{
 public:
  GANonFatalException(const std::string& file, 
		      const int line, 
		      const std::string& reason):
    GAException(file,line,reason) {}
};

