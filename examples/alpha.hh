#ifndef __GA_Alpha_hh__
#define __GA_Alpha_hh__
#include "population.hh"

class Alpha : public Population
{
   double sqr ( double x );
public:
   explicit Alpha(const Population::Settings& settings);

   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) override;
   int RandomAlgorithm();
};
#endif
