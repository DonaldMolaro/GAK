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

Alpha::Alpha(const Population::Options& options)
   : Alpha(options.toConfiguration())
{
}

Alpha::Alpha(const Population::Configuration& configuration)
   : Population(configuration)
{
}

Alpha::Alpha(
   Population::OperationTechnique Operation,
   int numberofIndividuals,
   int numberofTrials,
   int GenecticDeversity,
   double BitMutationRate,
   double CrossOverRate,
   Population::ReproductionTechniques ReproductionTechniques,
   Population::ParentSelectionTechnique ParentSelction,
   Population::DeletionTechnique Deletetion,
   Population::FitnessTechnique Fitness,
   Population::VariableLength Variable,
   int baseStates
   )
   : Alpha(Population::Configuration{Operation,
                                     numberofIndividuals,
                                     numberofTrials,
                                     GenecticDeversity,
                                     BitMutationRate,
                                     CrossOverRate,
                                     ReproductionTechniques,
                                     ParentSelction,
                                     Deletetion,
                                     Fitness,
                                     Variable,
                                     baseStates})
{
}

double Alpha::FitnessFunction(BaseString *b)
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
   for ( int i = 0 ; i < b->length() ; i++ )
   {
      for ( int j = i ; j < b->length() ; j++ )
      {
	 if (b->test(i) < b->test(j)) res++;
      }
   }
   for ( int i = b->length()-1 ; i >= 0; i-- )
   {
      for ( int j = i ; j >= 0 ; j-- )
      {
	 if (b->test(i) > b->test(j)) res++;
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


void Alpha::FitnessPrint(BaseString *b)
{
   for ( int i = 0 ; i < b->length() ; i++ )
   {
//      assert(b->test(i) >= 0);
//      assert(b->test(i) < 26);
      fprintf(stderr,"%c",b->test(i) + 'a');
   }
   fprintf(stderr," ::");
}

int Alpha::RandomAlgorithim()
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
   int fitness = FitnessFunction(current.get());
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
      int nextFit = FitnessFunction(next.get());
      if (nextFit > fitness)
      {
	 current = std::move(next);
	 next = std::make_unique<BaseString>(13,13);
	 fitness = nextFit;
	 FitnessPrint(current.get());
	 fprintf(stderr,"Iteration %d New Fitness %d\n",iter,fitness);
      }
      if (( iter % 1000 ) == 0)
      {
	 fprintf(stderr,"Iteration %d\n",iter);
      }
      iter++;
   }
   return 0;
}
