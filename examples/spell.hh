#ifndef __GA_Spell_hh__
#define __GA_Spell_hh__
#include "population.hh"

class Spell : public Population
{
   double sqr ( double x );
public:
   explicit Spell(const Population::Options& options);
   explicit Spell(const Population::Configuration& configuration);

   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b) override;
};
#endif
