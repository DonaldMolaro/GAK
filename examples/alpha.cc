#include <iostream>

#include "base.hh"
#include "except.hh"
#include "population.hh"
#include "alpha.hh"

Alpha::Alpha(const Population::Settings& settings)
   : settings_(settings)
{
}

void Alpha::validatePopulation(const Population& population) const
{
   if (population.settings().geneticDiversity != settings_.geneticDiversity ||
       population.settings().baseStates != settings_.baseStates ||
       population.settings().variableLength != settings_.variableLength)
   {
      throw GAFatalException(__FILE__,__LINE__,"Alpha problem settings do not match population settings");
   }
}

double Alpha::evaluateFitness(const BaseString& genes)
{
   int res = 0;
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
      for ( int j = i ; j < genes.length() ; j++ )
      {
	 if (genes.valueAt(i) < genes.valueAt(j)) res++;
      }
   }
   for ( int i = genes.length()-1 ; i >= 0; i-- )
   {
      for ( int j = i ; j >= 0 ; j-- )
      {
	 if (genes.valueAt(i) > genes.valueAt(j)) res++;
      }
   }
   return res;
}

void Alpha::printCandidate(const BaseString& genes, std::ostream& out) const
{
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
      out << static_cast<char>(genes.valueAt(i) + 'a');
   }
   out << " ::";
}

bool Alpha::hasReachedSolution(const Population&, const BaseString& genes, double fitness) const
{
   const int length = genes.length();
   const double perfectFitness = static_cast<double>(length * (length - 1));
   return fitness >= perfectFitness;
}
