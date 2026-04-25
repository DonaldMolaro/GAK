#include <cstdio>

#include "base.hh"
#include "population.hh"
#include "knapsack.hh"

const int Knapsack::kWeights[Knapsack::kItemCount] = {2, 3, 5, 7, 1, 4, 1, 6, 9, 8, 3, 5};
const int Knapsack::kValues[Knapsack::kItemCount] = {6, 5, 8, 9, 6, 7, 3, 7, 12, 10, 4, 8};

Knapsack::Knapsack(const Population::Options& options)
   : Knapsack(options.toConfiguration())
{
}

Knapsack::Knapsack(const Population::Configuration& configuration)
   : Population(configuration)
{
}

Knapsack::Knapsack(
   Population::OperationTechnique Operation,
   int numberofIndividuals,
   int numberofTrials,
   int GenecticDeversity,
   double BitMutationRate,
   double CrossOverRate,
   Population::ReproductionTechniques ReproductionTechniques,
   Population::ParentSelectionTechnique ParentSelction,
   Population::DeletionTechnique Deletetion,
   Population::FitnessTechnique Fitness,
   Population::VariableLength Variable,
   int baseStates
   )
   : Knapsack(Population::Configuration{Operation,
                                        numberofIndividuals,
                                        numberofTrials,
                                        GenecticDeversity,
                                        BitMutationRate,
                                        CrossOverRate,
                                        ReproductionTechniques,
                                        ParentSelction,
                                        Deletetion,
                                        Fitness,
                                        Variable,
                                        baseStates})
{
}

double Knapsack::FitnessFunction(const BaseString& b)
{
   int weight = 0;
   int value = 0;

   for ( int item = 0 ; item < b.length() ; item++ )
   {
      if (b.test(item))
      {
         weight += kWeights[item];
         value += kValues[item];
      }
   }

   if (weight <= kCapacity)
   {
      return value;
   }

   const int excessWeight = weight - kCapacity;
   const int penalty = excessWeight * excessWeight * 4;
   return value > penalty ? value - penalty : 0;
}

void Knapsack::FitnessPrint(const BaseString& b)
{
   int weight = 0;
   int value = 0;

   fprintf(stderr, "Items:");
   for ( int item = 0 ; item < b.length() ; item++ )
   {
      if (b.test(item))
      {
         fprintf(stderr, " %d", item);
         weight += kWeights[item];
         value += kValues[item];
      }
   }
   fprintf(stderr, " | weight=%d/%d value=%d", weight, kCapacity, value);
}
