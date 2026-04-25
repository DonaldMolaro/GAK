#ifndef __GA_Alpha_hh__
#define __GA_Alpha_hh__
#include "population.hh"

class Alpha : public Population
{
   double sqr ( double x );
public:
   explicit Alpha(const Population::Options& options);
   explicit Alpha(const Population::Configuration& configuration);

   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b, std::ostream& out) override;
   int RandomAlgorithm();
};
#endif
