#ifndef __GA_Knapsack_hh__
#define __GA_Knapsack_hh__

#include <vector>

#include "population.hh"

class Knapsack : public Population
{
public:
   explicit Knapsack(const Population::Options& options);
   explicit Knapsack(const Population::Configuration& configuration);

   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b) override;

private:
   static const int kCapacity = 35;
   static const int kItemCount = 12;
   static const int kWeights[kItemCount];
   static const int kValues[kItemCount];
};

#endif
