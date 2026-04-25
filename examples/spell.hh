#pragma once

#include "population.hh"

class Spell : public Population
{
   double sqr ( double x );
public:
   explicit Spell(const Population::Settings& settings);

   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
};
