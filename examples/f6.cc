#include <cassert>
#include <cmath>
#include <iostream>
#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "f6.hh"

F6::F6(const Population::Settings& settings)
   : Population(settings)
{
}

double F6::sqr(double x)
{
   return x * x;
}

double F6::evaluateFitness(const BaseString& genes)
{
   double x1 = static_cast<double>(decode(genes,0,22));
   double y1 = static_cast<double>(decode(genes,22,44));
   double x2 = x1 / 1000.00;
   double y2 = y1 / 1000.00;
   double x3 = x2 - 100.000;   
   double y3 = y2 - 100.000;
   
   double top = sqr( sin ( sqrt ( sqr ( x3 ) + sqr ( y3 ) ) ) );
   double bottom = 1.0 + 0.001 * sqr ( sqr ( x3 ) + sqr ( y3 ) );
   
   double f6 = 0.5 - ( top / bottom );

   assert ( f6 >= 0 );
   
   return static_cast<int>(f6 * 1000.0) + 1;

}

void F6::printCandidate(const BaseString& genes, std::ostream& out) const
{
   double x1 = static_cast<double>(decode(genes,0,22));
   double y1 = static_cast<double>(decode(genes,22,44));
   double x2 = x1 / 1000.00;
   double y2 = y1 / 1000.00;
   double x3 = x2 - 100.000;   
   double y3 = y2 - 100.000;
   
   out << "X ( " << x3 << " ) Y ( " << y3 << " ) ";
}




