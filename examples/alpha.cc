#include <cstdio>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <ctime>
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
   //
   // Fitness is the total squared distance that each 
   // element in the BaseString is wrong by.
   //
   // ok for each element in the correct order.
   // 
   int res = 0;
#if 0
   for ( int i = 0 ; i < b->length() ; i++ )
   {
      if (i == b->test(i)) res += i;
      for ( int j = i ; j < b->length() ; j++ )
      {
	 if (b->test(i) < b->test(j)) res++;
      }
   }
#else
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
      for ( int j = i ; j < genes.length() ; j++ )
      {
	 if (genes.test(i) < genes.test(j)) res++;
      }
   }
   for ( int i = genes.length()-1 ; i >= 0; i-- )
   {
      for ( int j = i ; j >= 0 ; j-- )
      {
	 if (genes.test(i) > genes.test(j)) res++;
      }
   }
/*
   for (   i = 0 ; i < b->length()-1 ; i++ )
   {
      if (b->test(i) == b->test(i+1)+1)
      {
	 res++;
      }
   }
   for (   i = b->length()-1 ; i >= 1; i-- )
   {
      if (b->test(i) == b->test(i-1)-1)
      {
	 res++;
      }
   }
*/
#endif
   //
   return res;
};


void Alpha::printCandidate(const BaseString& genes, std::ostream& out)
{
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
//      assert(b->test(i) >= 0);
//      assert(b->test(i) < 26);
      out << static_cast<char>(genes.test(i) + 'a');
   }
   out << " ::";
}

int Alpha::RandomAlgorithm()
{
   std::mt19937 generator(static_cast<unsigned int>(std::time(NULL)));
   std::uniform_int_distribution<int> value_distribution(0, 12);
   std::uniform_int_distribution<int> index_distribution(0, 12);
   std::unique_ptr<BaseString> current = std::make_unique<BaseString>(13,13);
   std::unique_ptr<BaseString> next = std::make_unique<BaseString>(13,13);
   for ( int i = 0 ; i < 13 ; i++ )
   {
      current->set(i,value_distribution(generator));
   }
   int fitness = evaluateFitness(*current);
   int iter = 0;
   while (fitness < 650)
   {
      for ( int i = 0 ; i < 13 ; i++ )
      {
	 next->set(i,current->test(i));
      }
      int index = index_distribution(generator);
      int value = value_distribution(generator);
      next->set(index,value);
      int nextFit = evaluateFitness(*next);
      if (nextFit > fitness)
      {
	 current = std::move(next);
	 next = std::make_unique<BaseString>(13,13);
	 fitness = nextFit;
	 printCandidate(*current, std::cerr);
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
