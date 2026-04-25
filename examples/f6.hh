#ifndef __GA_F6_hh__
#define __GA_F6_hh__
#include "population.hh"

class F6 : public Population
{
   double sqr ( double x );
public:
   explicit F6(const Population::Options& options);
   explicit F6(const Population::Configuration& configuration);

   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b, std::ostream& out) override;
};
#endif
