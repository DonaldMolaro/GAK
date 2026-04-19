#ifndef __GA_Knapsack_hh__
#define __GA_Knapsack_hh__

#include <vector>

#include "population.hh"

class Knapsack : public Population
{
public:
   explicit Knapsack(const Population::Options& options);
   explicit Knapsack(const Population::Configuration& configuration);
   [[deprecated("Use Knapsack(const Population::Options&) or Knapsack(const Population::Configuration&)")]]
   Knapsack(
      Population::OperationTechnique Operation,
      int numberofIndividuals,
      int numberofTrials,
      int GenecticDeversity,
      double BitMutationRate,
      double CrossOverRate,
      Population::ReproductionTechniques PReproductionTechniques,
      Population::ParentSelectionTechnique ParentSelction,
      Population::DeletionTechnique Deletetion,
      Population::FitnessTechnique Fitness,
      Population::VariableLength Variable,
      int baseStates
      );

   double FitnessFunction(BaseString *b) override;
   void FitnessPrint(BaseString *b) override;

private:
   static const int kCapacity = 35;
   static const int kItemCount = 12;
   static const int kWeights[kItemCount];
   static const int kValues[kItemCount];
};

#endif
