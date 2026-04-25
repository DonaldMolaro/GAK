#include <cstdio>
#include <ostream>

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

void Knapsack::FitnessPrint(const BaseString& b, std::ostream& out)
{
   int weight = 0;
   int value = 0;

   out << "Items:";
   for ( int item = 0 ; item < b.length() ; item++ )
   {
      if (b.test(item))
      {
         out << ' ' << item;
         weight += kWeights[item];
         value += kValues[item];
      }
   }
   out << " | weight=" << weight << '/' << kCapacity << " value=" << value;
}
