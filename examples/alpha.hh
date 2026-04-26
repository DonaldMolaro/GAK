#pragma once

#include "population.hh"

class Alpha : public PopulationProblem
{
public:
   explicit Alpha(const Population::Settings& settings);

   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   int runRandomAlgorithm(unsigned int seed, std::ostream& out);

private:
   Population::Settings settings_;
};
