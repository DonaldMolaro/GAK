#pragma once

#include "population.hh"

class GraphColoring : public PopulationProblem
{
public:
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   void writeVisualizationJson(const BaseString& genes, std::ostream& out) const override;
   bool hasReachedSolution(const Population& population,
                           const BaseString& genes,
                           double fitness) const override;
   void validatePopulation(const Population& population) const override;

   static const int kNodeCount = 16;
   static const int kColorCount = 3;
   static const int kEdgeCount = 29;

private:
   struct Edge
   {
      int left;
      int right;
   };

   static const Edge kEdges[kEdgeCount];
   static const char* kNodeNames[kNodeCount];
   static const char* kColorNames[kColorCount];
};
