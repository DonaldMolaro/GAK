#pragma once

#include "population.hh"

class F6 : public PopulationProblem
{
   static double sqr ( double x );
   static double decodeAxis(const BaseString& genes, int axis);
public:
   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
};
