//
// Genectic Algoritim Implementation.
//
// Author:
//        Donald Molaro
//
// Genesis:        Sept 1994
//
// Added in other CrossOvers    Oct 2-3 1994
//
// Started implementation of variable length
// chromosome strings.
// Added in VERBOSE compilation flag.
//                               Dec 23 1994
// Completed implementation of variable length
// chromosome strings...
//                               Dec 24 1994
//
// Code cleanup. Added in exceptions.
//                               January 2001
//
//
#include <cstdlib>
#include <iostream>
#include <random>

#include "except.hh"
#include "base.hh"
#include "chromosome.hh"

namespace
{
std::mt19937& ChromosomeRandomGenerator()
{
  static std::mt19937 generator(0);
  return generator;
}
}
std::mt19937& Chromosome::randomGenerator()
{
  return ChromosomeRandomGenerator();
}

int Chromosome::randomBit()
{
  std::uniform_int_distribution<int> distribution(0, 1);
  return distribution(randomGenerator());
}

int Chromosome::randomIndex(int upperBoundExclusive)
{
  std::uniform_int_distribution<int> distribution(0, upperBoundExclusive - 1);
  return distribution(randomGenerator());
}

void Chromosome::seedRandom(unsigned int seed)
{
  randomGenerator().seed(seed);
}

//
// Chromosome implementation. 
//
//
Chromosome::Chromosome(unsigned int CLength,unsigned int vlength,unsigned int PbaseStates)
{
  //
  // Randomly initialze the chromosome
  // at the start of of popultaion run.
  //
  /*
   * There is no real practical limit on the length of a chromosome string.
   * but a string that gets really too long is probably an error. Feel free
   * to increase this limit.
   */
  if (CLength > 2048) 
    throw GAFatalException(__FILE__,__LINE__,"Requested Chromosome Length over 2048. Proabably a mistake.");
  //
  //
  ChromosomeLength = CLength;
  variableLength = vlength;
  baseStates     = PbaseStates;
  ChromosomeString.reset(new BaseString(ChromosomeLength,baseStates));

  if (baseStates == 2)
    {
      for ( int i = 0 ; i < ChromosomeLength ; i++ )
	{
	  if (randomBit()) ChromosomeString->set(i);
	  else ChromosomeString->clear(i);
	}
    }
  else
    {
      for ( int i = 0 ; i < ChromosomeLength ; i++ )
	{
	  ChromosomeString->set(i,randomIndex(baseStates));
	}
    }
}

Chromosome::Chromosome(BaseString *b,unsigned int vlength,unsigned int PbaseStates)
{
  //
  ChromosomeLength = b->length();
  ChromosomeString.reset(b);
  variableLength = vlength;
  baseStates     = PbaseStates;
}

//
// based upon probablity, randomly set or clear a bit
// in the string.
//
// Changes required for variable length chromosome support
//            - none.
//
void Chromosome::SingleBitMutate(double probability)
{
  if ((probability >= 0.0)&&(probability <= 1.0))
    {
      int probabilityMask = (int) ( probability * 65535.0 );
      std::uniform_int_distribution<int> distribution(0, 0xFFFF);
   
      for ( int i = 0 ; i < ChromosomeLength ; i++ )
	{
	  if (distribution(randomGenerator()) < probabilityMask)
	    {
	      if (baseStates == 2)
		{
		  if (randomBit()) ChromosomeString->set(i);
		  else ChromosomeString->clear(i);
		}
	      else
		{
		  ChromosomeString->set(i,randomIndex(baseStates));
		}
	      
	    }
	}
    }
  else
    {
      throw GANonFatalException(__FILE__,__LINE__,"SingleBitMutate called with an impossible probablity. Ignored.");
    }
}
//
// Private Internal function,
//    Randomly deside if a chromosome pair will cross over.
//
//
int Chromosome::testCrossOverRate(double crossOverRate)
{
  int probabilityMask = (int) ( crossOverRate * 65535.0 );
  std::uniform_int_distribution<int> distribution(0, 0xFFFF);
  if (distribution(randomGenerator()) < probabilityMask) return 1;
  else return 0;
}
//
//
// Required Changes for variable length support.
//
//         - Select two crossover points; one spliting 
//           the father chromosomem one spliting the
//           mother chromosome.
//         - using these two cross over points cut
//           the father into primary and secondary bitstrings
//           cut mother into primary and secondary bitstrings.
//
//           01000101010001010101010101010101001010101     -- Father
//           <<----FPrimary---->><<---FSecondary---->>
//           1000101000100101010101010011010100            -- Mother
//           <<--MPrimary-->><<--MSecondary-->>
//
//         - Generate two children son and daughter from the cut
//           material.
//
//           01000101010001010101010101010011010100        -- Son
//           <<----FPrimary---->><<--MSecondary-->>
//           1000101000100101010101010101001010101         -- Daughter
//           <<--MPrimary-->><<----FSecondary--->>
//           
//
//         - return the two children.
//
//
void Chromosome::singlePointCrossOver(BaseString *mother,BaseString *father,
				      BaseString **son,BaseString **daughter)
{
  int FatherCrossoverPoint;
  int MotherCrossoverPoint;
  if (this->variableLength) 
    {
      FatherCrossoverPoint = randomIndex(father->length());
      MotherCrossoverPoint = randomIndex(mother->length());
    }
  else
    { 
      FatherCrossoverPoint = randomIndex(father->length());
      MotherCrossoverPoint = FatherCrossoverPoint;
    }
  int FatherPrimaryLength = FatherCrossoverPoint;
  int FatherSecondaryLength = father->length() - FatherCrossoverPoint;
  int MotherPrimaryLength = MotherCrossoverPoint;
  int MotherSecondaryLength = mother->length() - MotherCrossoverPoint;
  //
  // The son is made up of the Primary section of the father chromosome and the
  //     secondary section of the mother chromosome.
  // The daughter is made up of the Primary section of the mother chromosome and the
  //     secondary section of the father chromosome.
  //
  BaseString *boy   = new BaseString(FatherPrimaryLength + MotherSecondaryLength,baseStates);
  BaseString *girl  = new BaseString(MotherPrimaryLength + FatherSecondaryLength,baseStates);
  //
  // Copy primary sections of chromosomes.
  //
  for ( int i = 0 ; i < boy->length() ; i ++ )
    {
      if ( i < FatherPrimaryLength )
	{
	  boy->assign(i,father->test(i));
	}
      else
	{
	  boy->assign(i,mother->test(i + (MotherPrimaryLength - FatherPrimaryLength)));
	}
    }
  //
  for ( int i = 0 ; i < girl->length() ; i ++ )
    {
      if ( i < MotherPrimaryLength)
	{
	  girl->assign(i,mother->test(i));
	}
      else
	{
	  girl->assign(i,father->test(i + (FatherPrimaryLength - MotherPrimaryLength)));
	}
      
    }
  *son = boy;
  *daughter = girl;
}
//
//
void Chromosome::twoPointCrossOver(BaseString *mother,BaseString *father,
				   BaseString **son,BaseString **daughter)
{
  //
  // A two point cross over is just two single points and some extra memory.
  //
  BaseString *tmpFather = 0;
  BaseString *tmpMother = 0;
  singlePointCrossOver(mother,father,&tmpFather,&tmpMother);
  singlePointCrossOver(tmpMother,tmpFather,son,daughter);
  delete tmpFather;
  delete tmpMother;
}
//
//
void Chromosome::uniformCrossOver(BaseString *mother,BaseString *father,
				  BaseString **son,BaseString **daughter)
{
  BaseString *boy;
  BaseString *girl;
  if (this->variableLength)
    {
      //
      // A variable length uniform crossover randomly
      // selects bits from the two parrents until the end of 
      // one of the parrents strings, it then copies the remaining
      // bits into the appropriate sexed children.
      //
      boy  = new BaseString(father->length(),baseStates);
      girl = new BaseString(mother->length(),baseStates);
      int copyLength = std::min(father->length(),mother->length());
      // New Chromosomes are made up of random bits of the two parrents
      for ( int i = 0 ; i < copyLength ; i++ )
	{
	  if (randomBit() == 0)
	    {
	      boy->assign(i,father->test(i));
	      girl->assign(i,mother->test(i));
	    }
	  else
	    {
	      boy->assign(i,mother->test(i));
	      girl->assign(i,father->test(i));
	    }
	}
      for ( int i = copyLength ; i < father->length() ; i++ )
	{
	  boy->assign(i,father->test(i));
	}
      for ( int i = copyLength ; i < mother->length() ; i++ )
	{
	  girl->assign(i,mother->test(i));
	}
    }
  else
    {
      boy  = new BaseString(father->length(),baseStates);
      girl = new BaseString(mother->length(),baseStates); 
      // New Chromosomes are made up of random bits of the two parrents
      for ( int i = 0 ; i < father->length() ; i++ )
	{
	  if (randomBit() == 0)
	    {
	      boy->assign(i,father->test(i));
	      girl->assign(i,mother->test(i));
	    }
	  else
	    {
	      boy->assign(i,mother->test(i));
	      girl->assign(i,father->test(i));
	    }
	}
    }
  *son = boy;
  *daughter = girl;
}
//
// Probabilsitcially mate two Chromosomes together, resultsing in two
// children. 
//
void Chromosome::Mate(Chromosome *father,Chromosome **son,Chromosome **daughter,
		       double crossOverRate,CrossOverType crossType)
{
  Chromosome *mother = this;         // Just for notation purposes.
  //
  //
  if (!(this->variableLength) && (mother->ChromosomeLen() != father->ChromosomeLen()))
    {
      throw GAFatalException(__FILE__,__LINE__,"Missmatch in Chromosome strings");
    }
  //
  BaseString *sonString;
  BaseString *daughterString; 
  //
  // Only cross the chromosomes if we pass the
  // crossOverRate test.
  //
  if (testCrossOverRate(crossOverRate))
    {
      switch (crossType)
	{
	case SinglePoint:
	  singlePointCrossOver(mother->ChromosomeStr(),father->ChromosomeStr(),
			       &sonString,&daughterString);
	  break;
	case TwoPoint:
	  twoPointCrossOver(mother->ChromosomeStr(),father->ChromosomeStr(),
			    &sonString,&daughterString);
	 
	  break;
	case Uniform:
	  uniformCrossOver(mother->ChromosomeStr(),father->ChromosomeStr(),
			   &sonString,&daughterString);
	  break;
	default:
	  throw GANonFatalException(__FILE__,__LINE__,"Unimplemented crossover type");
	}
    }
  else 
    {
      //
      // Failed the Cross over test. just copy the bits from
      // the Father into the Son and the bits from the mother
      // into the daughter.
      //
      sonString      = new BaseString(father->ChromosomeLen(),baseStates);
      daughterString = new BaseString(mother->ChromosomeLen(),baseStates);
      for ( int i = 0 ; i < father->ChromosomeLen() ; i++ )
	{
	  sonString->assign(i,father->ChromosomeStr()->test(i));
	}
      for ( int i = 0 ; i < mother->ChromosomeLen() ; i++ )
	{
	  daughterString->assign(i,mother->ChromosomeStr()->test(i));
	}
    }
  *son      = new Chromosome(sonString,father->variableLength,baseStates);
  *daughter = new Chromosome(daughterString,mother->variableLength,baseStates);
  return;
}

bool Chromosome::compare(const Chromosome *candidate) const
{
  if (candidate->ChromosomeLen() == this->ChromosomeLen())
    {
      for ( int i = 0 ; i < this->ChromosomeLen() ; i++ )
	{
	  if (this->ChromosomeStr()->test(i) != candidate->ChromosomeStr()->test(i)) return false;
	}
      return true;
    }
  return false;
}

