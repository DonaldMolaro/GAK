//
// Genectic Algoritim Implementation.
//
// Author:
//        Donald Molaro
//
// Genesis:        Sept 1994
//
#ifndef __GA_Chromosome_hh__
#define __GA_Chromosome_hh__
#include <iosfwd>
#include <memory>
#include <random>

class Chromosome
{
public:
  enum CrossOverType { SinglePoint,TwoPoint,Uniform };
private:
  static std::mt19937& randomGenerator();
  static int randomBit();
  static int randomIndex(int upperBoundExclusive);
  int  ChromosomeLength;
  int  variableLength;
  int  baseStates;
  std::unique_ptr<BaseString> ChromosomeString;
  int testCrossOverRate(double crossOverRate);
  void singlePointCrossOver(BaseString *mother,BaseString *father,
			    BaseString **son,BaseString **daughter);
  void twoPointCrossOver(BaseString *mother,BaseString *father,
			 BaseString **son,BaseString **daughter);
  void uniformCrossOver(BaseString *mother,BaseString *father,
			BaseString **son,BaseString **daughter);
public:
  // Default is a 32 Bit Chromosome
  // variable length not permitted.
  // default is a binary base string.
  Chromosome(unsigned int ChromosomeLength = 32,
	     unsigned int vlength = 0,
	     unsigned int numStates = 2); 
   
  // Create an entity with a defined
  // genectic makeup. 
  Chromosome(BaseString *b,
	     unsigned int vlength = 0,
	     unsigned int numStates = 2); 

  ~Chromosome() = default;
  static void seedRandom(unsigned int seed);
  int ChromosomeLen() const                { return ChromosomeLength; }
  BaseString *ChromosomeStr() const        { return ChromosomeString.get(); }
  void SingleBitMutate(double probability = 0.008);
  void Mate(Chromosome *father,Chromosome **son,Chromosome **daughter,
	    double crossOverRate = 0.65,CrossOverType crossType = SinglePoint);
  bool compare(const Chromosome *candidate) const;
  void print(std::ostream &ostr) const     { ChromosomeStr()->printBits(ostr); ostr << std::endl; }
};
#endif
