#include <cmath>
#include <iostream>
#include "base.hh"
#include "chromosome.hh"
#include "except.hh"
#include "population.hh"
#include "f6.hh"

namespace
{
constexpr int kBitsPerAxis = 22;
constexpr double kAxisMaximumCode = static_cast<double>((1 << kBitsPerAxis) - 1);
constexpr double kDomainLow = -100.0;
constexpr double kDomainHigh = 100.0;
}

double F6::sqr(double x)
{
   return x * x;
}

// Each axis is a 22-bit code mapped linearly onto the standard Schaffer F6
// domain [-100, 100].
double F6::decodeAxis(const BaseString& genes, int axis)
{
   const int start = axis * kBitsPerAxis;
   const double code = static_cast<double>(genes.decodeBits(start, start + kBitsPerAxis));
   return kDomainLow + (kDomainHigh - kDomainLow) * (code / kAxisMaximumCode);
}

void F6::validatePopulation(const Population& population) const
{
   const Population::Settings& settings = population.settings();
   if (settings.baseStates != 2 || settings.chromosomeLength < 2 * kBitsPerAxis ||
       settings.variableLength != Population::VariableLengthMode::Fixed)
   {
      throw GAFatalException(__FILE__,__LINE__,
                             "F6 requires a fixed-length binary chromosome of at least 44 bits");
   }
}

double F6::evaluateFitness(const BaseString& genes)
{
   const double x = decodeAxis(genes, 0);
   const double y = decodeAxis(genes, 1);

   // Schaffer F6: 0.5 + (sin^2(r) - 0.5) / (1 + 0.001 r^2)^2, which lies in
   // [0, 1) with its global minimum of 0 at the origin.
   const double radiusSquared = sqr ( x ) + sqr ( y );
   const double top = sqr( std::sin ( std::sqrt ( radiusSquared ) ) ) - 0.5;
   const double bottom = sqr ( 1.0 + 0.001 * radiusSquared );
   const double f6 = 0.5 + ( top / bottom );

   // Scaled to [1, 1001) for readability; kept fractional so nearby points
   // are still distinguishable near the optimum.
   return f6 * 1000.0 + 1.0;
}

void F6::printCandidate(const BaseString& genes, std::ostream& out) const
{
   out << "X ( " << decodeAxis(genes, 0) << " ) Y ( " << decodeAxis(genes, 1) << " ) ";
}
