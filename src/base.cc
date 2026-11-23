#include <iostream>
#include <string>

#include "base.hh"
#include "except.hh"

namespace
{
int bitsRequiredForStates(int stateCount)
{
   int bits = 0;
   int maxValue = stateCount - 1;
   while (maxValue > 0)
   {
      ++bits;
      maxValue >>= 1;
   }
   return bits > 0 ? bits : 1;
}
}

BaseString::BaseString(int length, int numStates)
{
  if (length <= 0)
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to create nonsensical BaseString");
    }
  if (numStates <= 1)
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to create nonsensical BaseString");
    }

  stateCount   = numStates;
  baseLength   = bitsRequiredForStates(numStates);
  stringLength = length;
  bitLength    = stringLength * baseLength;
  values.assign(stringLength, 0);
}

void BaseString::outOfRange(const char* what)
{
  throw GAFatalException(__FILE__,__LINE__,what);
}

int BaseString::testBit(int i) const
{
  if (i < 0 || i >= bitLength)
    {
      outOfRange("Attempted to TEST out of range bit");
    }
  return (values[i / baseLength] >> (i % baseLength)) & 1;
}

void BaseString::setBit(int i)
{
  if (i < 0 || i >= bitLength)
    {
      outOfRange("Attempted to SET out of range bit");
    }
  values[i / baseLength] |= 1 << (i % baseLength);
}

void BaseString::clearBit(int i)
{
  if (i < 0 || i >= bitLength)
    {
      outOfRange("Attempted to CLEAR out of range bit");
    }
  values[i / baseLength] &= ~(1 << (i % baseLength));
}

int BaseString::decodeBits(int startBit, int endBit) const
{
  if (startBit < 0 || endBit < startBit || endBit > bitLength)
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to decode out of range bits");
    }

  int result = 0;
  for (int bit = startBit ; bit < endBit ; bit++)
    {
      result <<= 1;
      if (testBit(bit))
        {
          result += 1;
        }
    }

  return result;
}

void BaseString::setValue(int index, int value)
{
  if (value < 0 || value >= stateCount)
    {
      outOfRange("Attempted to SET a value outside the state range");
    }
  if (index < 0 || index >= stringLength)
    {
      outOfRange("Attempted to SET out of range base");
    }
  values[index] = value;
}

void BaseString::clearValue(int index)
{
  if (index < 0 || index >= stringLength)
    {
      outOfRange("Attempted to CLEAR out of range base");
    }
  values[index] = 0;
}

void BaseString::swapValues(int first, int second)
{
  const int firstValue = valueAt(first);
  const int secondValue = valueAt(second);
  setValue(first, secondValue);
  setValue(second, firstValue);
}

std::string BaseString::bitString() const
{
  std::string printableBits;
  printableBits.reserve(bitLength);
  for ( int i = 0 ; i < bitLength; i++ )
    {
      printableBits += testBit(i) ? '1' : '0';
    }
  return printableBits;
}

void BaseString::printBits(std::ostream &ostr) const
{
  ostr << bitString();
}

void BaseString::printSymbols(const std::vector<std::string>& symbols, std::ostream &ostr) const
{
  if (static_cast<int>(symbols.size()) < stateCount)
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to print BaseString with insufficient symbols");
    }

  for ( int i = 0 ; i < stringLength; i++ )
    {
      ostr << symbols[valueAt(i)];
    }
}
