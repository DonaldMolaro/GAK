#ifndef __GA_Dome_hh__
#define __GA_Dome_hh__
#include "population.hh"

class Dome : public Population
{
   double sqr ( double x );
public:
   explicit Dome(const Population::Settings& settings);
  
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) override;
};
#endif
