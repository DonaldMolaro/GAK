//
// Genetic Algorithm Implementation.
//
// Author:
//        Donald Molaro
//
// Genesis:        Sept 1994
//
#ifndef __GA_Chromosome_hh__
#define __GA_Chromosome_hh__
#include "base.hh"

#include <iosfwd>
#include <memory>
#include <ostream>
#include <random>

class Chromosome
{
public:
  enum class CrossoverType { SinglePoint, TwoPoint, Uniform };
private:
  using BaseStringValue = BaseString;
  using ChromosomePtr = std::unique_ptr<Chromosome>;
  static int randomBit(std::mt19937& randomGenerator);
  static int randomIndex(std::mt19937& randomGenerator, int upperBoundExclusive);
  int chromosomeLength;
  bool variableLength;
  int baseStates;
  BaseString chromosomeString_;
  bool shouldCrossover(std::mt19937& randomGenerator, double crossoverRate);
  static BaseStringValue cloneBaseString(const BaseString& source, int baseStates);
  std::pair<BaseStringValue, BaseStringValue> singlePointCrossOver(const BaseString& mother,
                                                                   const BaseString& father,
                                                                   std::mt19937& randomGenerator);
  std::pair<BaseStringValue, BaseStringValue> twoPointCrossOver(const BaseString& mother,
                                                                const BaseString& father,
                                                                std::mt19937& randomGenerator);
  std::pair<BaseStringValue, BaseStringValue> uniformCrossOver(const BaseString& mother,
                                                               const BaseString& father,
                                                               std::mt19937& randomGenerator);
public:
  // Default is a 32-gene binary chromosome with fixed length.
  Chromosome(unsigned int chromosomeLength = 32,
	     bool variableLength = false,
	     unsigned int numStates = 2,
	     std::mt19937* randomGenerator = nullptr); 
   
  explicit Chromosome(BaseString b,
	     bool variableLength = false,
	     unsigned int numStates = 2);

  ~Chromosome() = default;
  int length() const                       { return chromosomeLength; }
  bool isVariableLength() const            { return variableLength; }
  BaseString& genes()                      { return chromosomeString_; }
  const BaseString& genes() const          { return chromosomeString_; }
  // Apply the built-in point mutation operator across the chromosome.
  void mutate(double probability = 0.008, std::mt19937* randomGenerator = nullptr);
  // Produce two children using one of the built-in crossover strategies.
  std::pair<ChromosomePtr, ChromosomePtr> mate(Chromosome& father,
                                               double crossoverRate = 0.65,
                                               CrossoverType crossoverType = CrossoverType::SinglePoint,
                                               std::mt19937* randomGenerator = nullptr);
  bool equals(const Chromosome& candidate) const;
  void print(std::ostream &ostr) const     { genes().printBits(ostr); ostr << '\n'; }
};
#endif
