//
// Genectic Algoritim Implementation.
//
// Author:
//        Donald Molaro
//
// Genesis:        Sept 1994
//
// Changed bitstrings to a base string.
//                 Dec 26 1994.
//
// Code Clean up & moved to lookup tables for 
// base locations. Moved to exceptions for
// Error handling. 
//                 January 2001
//

#include <cmath>
#include <iostream>
#include <string>

#include "base.hh"
#include "except.hh"

namespace
{
double IntLog2(int x)
{
   return (std::log(static_cast<double>(x))/std::log(2.0));
}
}

BaseString::BaseString(int Len,int numStates)
{
  if (Len <= 0)
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to create non-sencical BaseString");
    }
  if (numStates <= 1)
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to create non-sencical BaseString");
    }

  baseLength   = static_cast<int>(std::ceil(IntLog2(numStates))); // # of bits required to represent base
  stringLength = Len;                         // Length of string in bases.
  bitLength    = stringLength * baseLength;   // Length of string in bits.
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
  /*
   * Dragons here...
   */
  return ( ( i >= 0)&&(i < bitLength ) 
	   ? ( ( bits[i >> 3] & ( 0x80 >> (i & 0x07) )) ? 1 : 0) 
	   : ( throw GAFatalException(__FILE__,__LINE__,"Attempted to TEST out of range bit"),0 ) );
}

void BaseString::setBit(int i)
{
  if ( ( i >= 0)&&(i < bitLength) ) 
    {
      int byteoffset = i >> 3;        // Divide by 8.
      int bitoffset  = i & 0x07;      // Mod by 8.
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
      int byteoffset = i >> 3;        // Divide by 8.
      int bitoffset  = i & 0x07;      // Mod by 8.
      bits[byteoffset] &= ~( 0x80 >> bitoffset );
    }
  else
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to CLEAR out of range bit");      
    }
}
//
// The externalally accessable methods test/set/clear
// provide an interface to the internal bitstring.
//
int BaseString::test(int baseNo) const
{
   int res      = 0;
   if  ( (baseNo >= 0)&&(baseNo < stringLength) )
     {
       int startBit = startPos[baseNo];
       int endBit   = endPos[baseNo];
       for ( int i = startBit ; i < endBit ; i++ )
	 {
	   res += (this->testBit(i)) << (i - startBit);
	 }
     }
   else
     {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to TEST out of range base");
     }
   return res;
}


void BaseString::set(int baseNo,int value)
{
  if (( baseNo >= 0)&&(baseNo < stringLength))
    {
      int startBit = startPos[baseNo];
      int endBit   = endPos[baseNo];
      for ( int i = startBit ; i < endBit ; i++ )
	{
	  if (value & (0x01 << (i - startBit)))
	    {
	      this->setBit(i);
	    }
	  else
	    {
	      this->clearBit(i);
	    }
	}
    }
  else
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to SET out of range base");
    }
}

void BaseString::clear(int baseNo)
{
  if ( ( baseNo >= 0)&&(baseNo < stringLength) )
    {
      int startBit = startPos[baseNo];
      int endBit   = endPos[baseNo];

      for ( int i = startBit ; i < endBit ; i++ )
	{
	  this->clearBit(i);
	}
    }
  else
    {
      throw GAFatalException(__FILE__,__LINE__,"Attempted to CLEAR out of range base");
    }
}

void BaseString::printBits(std::ostream &ostr) const
{
  std::string printable_bits;
  for ( int i = 0 ; i < bitLength; i++ )
    {
      if (this->testBit(i))
	{
	  printable_bits += "1";
	}
      else
	{
	  printable_bits += "0";
	}
   }
  ostr << printable_bits;
}


void BaseString::print(char **value,std::ostream &ostr) const
{
  std::string printable_bits;
   for ( int i = 0 ; i < stringLength; i++ )
   {
     printable_bits += value[this->test(i)];
   }
   ostr << printable_bits;
}
