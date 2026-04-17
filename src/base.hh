//
// Genectic Algoritim Implementation.
//
// Author:
//        Donald Molaro
//
// Genesis:        Sept 1994
//
// Changed from a bitstring class to an symbolic
// class.
//
#ifndef __GA_Base_hh__
#define __GA_Base_hh__
#include <iosfwd>
#include <vector>

class BaseString
{
private:
  std::vector<unsigned char> bits;
  std::vector<int> startPos;
  std::vector<int> endPos;
  int stringLength;
  int numBytes;
  int baseLength;
  int bitLength;
  int testBit(int i) const;
  void setBit(int i);
  void clearBit(int i);
public:
  BaseString(int length,int numStates);
  ~BaseString() = default;
  int length() const               { return stringLength; }
  int test(int baseNo) const;
  void set(int baseNo,int value = 1);
  void clear(int baseNo);
  void assign(int i,int value) { if (value) set(i,value); else clear(i); }
  void printBits(std::ostream &ostr) const;           // For debuging purposes only.
  void print(char **value,std::ostream &ostr) const;  // For debuging purposes only.
#ifdef GAK_TESTING
  int testBitForTesting(int i) const { return testBit(i); }
  void setBitForTesting(int i) { setBit(i); }
  void clearBitForTesting(int i) { clearBit(i); }
#endif
};
#endif
