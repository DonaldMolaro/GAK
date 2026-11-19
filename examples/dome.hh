#pragma once

#include "population.hh"

class Dome : public PopulationProblem
{
public:
   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   bool hasReachedSolution(const Population& population,
                           const BaseString& genes,
                           double fitness) const override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
};
