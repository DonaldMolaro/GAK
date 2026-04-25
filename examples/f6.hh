#ifndef __GA_F6_hh__
#define __GA_F6_hh__
#include "population.hh"

class F6 : public Population
{
   double sqr ( double x );
public:
   explicit F6(const Population::Settings& settings);

   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
};
#endif
