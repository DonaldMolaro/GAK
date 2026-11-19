#include <cmath>
#include <iostream>
#include "base.hh"
#include "chromosome.hh"
#include "except.hh"
#include "population.hh"
#include "dome.hh"

double Dome::evaluateFitness(const BaseString& genes)
{
   const long long x1 = genes.decodeBits(0,16);
   const long long y1 = genes.decodeBits(16,32);
   // ((x/100)^2 + (y/100)^2 + 1) * 100 == (x^2 + y^2 + 10000) / 100. The sum
   // is exact in integers and the single division is correctly rounded, so
   // every compiler produces the same value (no fused multiply-add drift).
   // Kept fractional so points near the origin remain distinguishable; the
   // unique optimum is exactly 100 at (0, 0).
   return static_cast<double>(x1 * x1 + y1 * y1 + 10000) / 100.0;
}

void Dome::validatePopulation(const Population& population) const
{
   const Population::Settings& settings = population.settings();
   if (settings.baseStates != 2 || settings.chromosomeLength < 32 ||
       settings.variableLength != Population::VariableLengthMode::Fixed)
   {
      throw GAFatalException(__FILE__,__LINE__,
                             "Dome requires a fixed-length binary chromosome of at least 32 bits");
   }
}

bool Dome::hasReachedSolution(const Population&, const BaseString&, double fitness) const
{
   return fitness <= 100.0;
}


void Dome::printCandidate(const BaseString& genes, std::ostream& out) const
{
   int x1 = genes.decodeBits(0,16);
   int y1 = genes.decodeBits(16,32);

   double x2 = static_cast<double>(x1) / 100.0;
   double y2 = static_cast<double>(y1) / 100.0;
   
   out << "X ( " << x2 << " ) Y ( " << y2 << " ) ";
}
