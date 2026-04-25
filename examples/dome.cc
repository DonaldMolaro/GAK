#include <cmath>
#include <iostream>
#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "dome.hh"

Dome::Dome(const Population::Options& options)
   : Dome(options.toConfiguration())
{
}

Dome::Dome(const Population::Configuration& configuration)
   : Population(configuration)
{
}

double Dome::sqr(double x)
{
   return x * x;
}

double Dome::FitnessFunction(const BaseString& b)
{
   int x1 = decode(b, 0,16);
   int y1 = decode(b,16,32);
   double x2 = static_cast<double>(x1) / 100.0;
   double y2 = static_cast<double>(y1) / 100.0;

   double res = (sqr(x2)+sqr(y2))+1.0;
   return static_cast<int>(res * 100.0);
};


void Dome::FitnessPrint(const BaseString& b, std::ostream& out)
{
   int x1 = decode(b,0,16);
   int y1 = decode(b,16,32);

   double x2 = static_cast<double>(x1) / 100.0;
   double y2 = static_cast<double>(y1) / 100.0;
   
   out << "X ( " << x2 << " ) Y ( " << y2 << " ) ";
}
