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
  numBytes     = bitLength / 8;
  if (bitLength % 8 ) numBytes++;

  bits.assign(numBytes, 0);
  startPos.resize(stringLength);
  endPos.resize(stringLength);

  for ( int baseNo = 0 ; baseNo < stringLength ; baseNo++ )
    {
      int startBit = baseNo * baseLength;
      int endBit   = (baseNo * baseLength) + baseLength;
      startPos[baseNo] = startBit;
      endPos[baseNo]   = endBit;
    }
}

int BaseString::testBit(int i) const
{
  return ( ( i >= 0)&&(i < bitLength )
	   ? ( ( bits[i >> 3] & ( 0x80 >> (i & 0x07) )) ? 1 : 0)
	   : ( throw GAFatalException(__FILE__,__LINE__,"Attempted to TEST out of range bit"),0 ) );
}

void BaseString::setBit(int i)
{
  if ( ( i >= 0)&&(i < bitLength) )
    {
      int byteoffset = i >> 3;
      int bitoffset  = i & 0x07;
      bits[byteoffset] |= ( 0x80 >> bitoffset );
    }
  else
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to SET out of range bit");
    }
}

void BaseString::clearBit(int i)
{
  if ( ( i >= 0)&&(i < bitLength) )
    {
      int byteoffset = i >> 3;
      int bitoffset  = i & 0x07;
      bits[byteoffset] &= ~( 0x80 >> bitoffset );
    }
  else
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to CLEAR out of range bit");
    }
}

int BaseString::valueAt(int index) const
{
   int result = 0;
   if  ( (index >= 0)&&(index < stringLength) )
     {
       int startBit = startPos[index];
       int endBit   = endPos[index];
       for ( int i = startBit ; i < endBit ; i++ )
	 {
	   result += testBit(i) << (i - startBit);
	 }
     }
   else
     {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to TEST out of range base");
     }
   return result;
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
  if (( index >= 0)&&(index < stringLength))
    {
      int startBit = startPos[index];
      int endBit   = endPos[index];
      for ( int i = startBit ; i < endBit ; i++ )
	{
	  if (value & (0x01 << (i - startBit)))
	    {
	      setBit(i);
	    }
	  else
	    {
	      clearBit(i);
	    }
	}
    }
  else
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to SET out of range base");
    }
}

void BaseString::clearValue(int index)
{
  if ( ( index >= 0)&&(index < stringLength) )
    {
      int startBit = startPos[index];
      int endBit   = endPos[index];

      for ( int i = startBit ; i < endBit ; i++ )
	{
	  clearBit(i);
	}
    }
  else
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to CLEAR out of range base");
    }
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
