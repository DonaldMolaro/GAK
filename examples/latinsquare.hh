#ifndef __GA_LatinSquare_hh__
#define __GA_LatinSquare_hh__

#include "population.hh"

class LatinSquare : public Population
{
public:
   explicit LatinSquare(const Population::Settings& settings);

   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;

private:
   int squareSize(const BaseString& genes) const;
};

#endif
