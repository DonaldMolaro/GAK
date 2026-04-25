#ifndef __GA_NQueens_hh__
#define __GA_NQueens_hh__

#include "population.hh"

class NQueens : public Population
{
public:
   explicit NQueens(const Population::Settings& settings);

   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
};

#endif
