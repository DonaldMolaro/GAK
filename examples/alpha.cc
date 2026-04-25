#include <cstdio>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "alpha.hh"

Alpha::Alpha(const Population::Settings& settings)
   : Population(settings)
{
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

int Alpha::RandomAlgorithm()
{
   std::mt19937 generator(randomSeed());
   std::uniform_int_distribution<int> value_distribution(0, 12);
   std::uniform_int_distribution<int> index_distribution(0, 12);
   BaseString current(13,13);
   BaseString next(13,13);
   for ( int i = 0 ; i < 13 ; i++ )
   {
      current.setValue(i,value_distribution(generator));
   }
   int fitness = evaluateFitness(current);
   int iter = 0;
   while (fitness < 650)
   {
      for ( int i = 0 ; i < 13 ; i++ )
      {
	 next.setValue(i,current.valueAt(i));
      }
      int index = index_distribution(generator);
      int value = value_distribution(generator);
      next.setValue(index,value);
      int nextFit = evaluateFitness(next);
      if (nextFit > fitness)
      {
	 current = next;
	 next = BaseString(13,13);
	 fitness = nextFit;
	 printCandidate(current, std::cerr);
	 std::cerr << "Iteration " << iter << " New Fitness " << fitness << '\n';
      }
      if (( iter % 1000 ) == 0)
      {
	 std::cerr << "Iteration " << iter << '\n';
      }
      iter++;
   }
   return 0;
}
