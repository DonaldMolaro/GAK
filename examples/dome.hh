#pragma once

#include "population.hh"

class Dome : public PopulationProblem
{
   double sqr ( double x );
public:
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
};
