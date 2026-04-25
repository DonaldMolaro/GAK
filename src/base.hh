#pragma once

#include <iosfwd>
#include <string>
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
  int stateCount;
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
  int states() const               { return stateCount; }
  int bitsPerValue() const         { return baseLength; }
  int bitCount() const             { return bitLength; }

  // Read or write the symbolic value stored at a gene position.
  int valueAt(int index) const;
  void setValue(int index,int value = 1);
  void clearValue(int index);
  void swapValues(int first, int second);

  // Convenience formatting helpers for debugging and tests.
  std::string bitString() const;
  void printBits(std::ostream &ostr) const;
  void printSymbols(const std::vector<std::string>& symbols, std::ostream &ostr) const;

#ifdef GAK_TESTING
  int testBitForTesting(int i) const { return testBit(i); }
  void setBitForTesting(int i) { setBit(i); }
  void clearBitForTesting(int i) { clearBit(i); }
#endif
};
