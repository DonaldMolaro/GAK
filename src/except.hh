#if !defined(EXCEPT_H)
#define EXCEPT_H

#include <iosfwd>
#include <string>
#include <stdexcept>

// class GAException : public runtime_error
class GAException
{
 public:
  GAException(const std::string& file, 
	      const int line, 
	      const std::string& reason);
  virtual std::ostream& operator<<(std::ostream& ostr);
  virtual const char* what () const;
 private:
  const std::string _file;
  const int _line;
  const std::string _reason;
};

class GAComplete : public GAException
{
 public:
  GAComplete(const std::string& file, 
	     const int line, 
	     const std::string& reason):
    GAException(file,line,reason) {} ;
};

class GAFatalException : public GAException 
{
 public:
  GAFatalException(const std::string& file, 
		   const int line, 
		   const std::string& reason):
    GAException(file,line,reason) {};
};

class GANonFatalException : public GAException 
{
 public:
  GANonFatalException(const std::string& file, 
		      const int line, 
		      const std::string& reason):
    GAException(file,line,reason) {};
};

#endif
