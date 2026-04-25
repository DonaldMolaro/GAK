#ifndef __GA_Dome_hh__
#define __GA_Dome_hh__
#include "population.hh"

class Dome : public Population
{
   double sqr ( double x );
public:
   explicit Dome(const Population::Options& options);
   explicit Dome(const Population::Configuration& configuration);
  
   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b, std::ostream& out) override;
};
#endif
