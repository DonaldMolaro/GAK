#ifndef __GA_LatinSquare_hh__
#define __GA_LatinSquare_hh__

#include "population.hh"

class LatinSquare : public Population
{
public:
   explicit LatinSquare(const Population::Options& options);
   explicit LatinSquare(const Population::Configuration& configuration);

   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b, std::ostream& out) override;

private:
   int squareSize(const BaseString& b) const;
};

#endif
