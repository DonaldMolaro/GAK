#include <cstdio>
#include <ostream>

#include "base.hh"
#include "population.hh"
#include "knapsack.hh"

const int Knapsack::kWeights[Knapsack::kItemCount] = {2, 3, 5, 7, 1, 4, 1, 6, 9, 8, 3, 5};
const int Knapsack::kValues[Knapsack::kItemCount] = {6, 5, 8, 9, 6, 7, 3, 7, 12, 10, 4, 8};

double Knapsack::evaluateFitness(const BaseString& genes)
{
   int weight = 0;
   int value = 0;

   for ( int item = 0 ; item < genes.length() ; item++ )
   {
      if (genes.valueAt(item))
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

void Knapsack::printCandidate(const BaseString& genes, std::ostream& out) const
{
   int weight = 0;
   int value = 0;

   out << "Items:";
   for ( int item = 0 ; item < genes.length() ; item++ )
   {
      if (genes.valueAt(item))
      {
         out << ' ' << item;
         weight += kWeights[item];
         value += kValues[item];
      }
   }
   out << " | weight=" << weight << '/' << kCapacity << " value=" << value;
}

void Knapsack::writeVisualizationJson(const BaseString& genes, std::ostream& out) const
{
   int weight = 0;
   int value = 0;
   out << "{ \"type\": \"knapsack\", \"capacity\": " << kCapacity << ", \"items\": [";
   for (int item = 0 ; item < kItemCount ; item++)
   {
      if (item > 0)
      {
         out << ", ";
      }
      const bool selected = genes.valueAt(item) != 0;
      if (selected)
      {
         weight += kWeights[item];
         value += kValues[item];
      }
      out << "{ \"index\": " << item
          << ", \"weight\": " << kWeights[item]
          << ", \"value\": " << kValues[item]
          << ", \"selected\": " << (selected ? "true" : "false") << " }";
   }
   out << "], \"total_weight\": " << weight
       << ", \"total_value\": " << value
       << ", \"feasible\": " << (weight <= kCapacity ? "true" : "false") << " }";
}
