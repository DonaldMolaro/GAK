#ifndef __GA_NQueens_hh__
#define __GA_NQueens_hh__

#include "population.hh"

class NQueens : public Population
{
public:
   explicit NQueens(const Population::Options& options);
   explicit NQueens(const Population::Configuration& configuration);

   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b) override;
};

#endif
