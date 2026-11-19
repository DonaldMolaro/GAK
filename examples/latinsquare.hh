#pragma once


#include "population.hh"

class LatinSquare : public PopulationProblem
{
public:
   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   // A perfect square scores 2 * size^2 (every row and column fully distinct).
   bool hasReachedSolution(const Population& population,
                           const BaseString& genes,
                           double fitness) const override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   void writeVisualizationJson(const BaseString& genes, std::ostream& out) const override;

private:
   int squareSize(const BaseString& genes) const;
};
