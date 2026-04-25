#ifndef __GA_Spell_hh__
#define __GA_Spell_hh__
#include "population.hh"

class Spell : public Population
{
   double sqr ( double x );
public:
   explicit Spell(const Population::Settings& settings);

   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) override;
};
#endif
