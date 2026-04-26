#include <cmath>
#include <iostream>
#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "dome.hh"

double Dome::sqr(double x)
{
   return x * x;
}

double Dome::evaluateFitness(const BaseString& genes)
{
   int x1 = genes.decodeBits(0,16);
   int y1 = genes.decodeBits(16,32);
   double x2 = static_cast<double>(x1) / 100.0;
   double y2 = static_cast<double>(y1) / 100.0;

   double res = (sqr(x2)+sqr(y2))+1.0;
   return static_cast<int>(res * 100.0);
};


void Dome::printCandidate(const BaseString& genes, std::ostream& out) const
{
   int x1 = genes.decodeBits(0,16);
   int y1 = genes.decodeBits(16,32);

   double x2 = static_cast<double>(x1) / 100.0;
   double y2 = static_cast<double>(y1) / 100.0;
   
   out << "X ( " << x2 << " ) Y ( " << y2 << " ) ";
}
