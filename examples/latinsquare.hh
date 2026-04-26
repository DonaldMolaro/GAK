#pragma once


#include "population.hh"

class LatinSquare : public PopulationProblem
{
public:
   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;

private:
   int squareSize(const BaseString& genes) const;
};
