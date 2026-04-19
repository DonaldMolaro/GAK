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
  // Packed gene storage. Each logical base occupies enough bits to represent
  // values in [0, numStates), so symbolic chromosomes can share the same
  // storage class as binary chromosomes.
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
  // Build a logical string with `length` gene positions and `numStates`
  // representable values per position. `numStates == 2` gives classic bits;
  // larger values produce a symbolic chromosome.
  BaseString(int length,int numStates);
  ~BaseString() = default;
  int length() const               { return stringLength; }
  // Read the symbolic value stored at a gene position.
  int test(int baseNo) const;
  // Store a symbolic value at a gene position.
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
