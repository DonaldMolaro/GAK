#pragma once

#include "population.hh"

class Alpha : public Population
{
   double sqr ( double x );
public:
   explicit Alpha(const Population::Settings& settings);

   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   int RandomAlgorithm();
};
