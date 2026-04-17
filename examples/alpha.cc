#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "alpha.hh"

Alpha::Alpha(
   Population::OperationTechnique Operation,
   int numberofIndividuals,
   int numberofTrials,
   int GenecticDeversity,
   double BitMutationRate,
   double CrossOverRate,
   Population::ReproductionTechniques ReproductionTechniques,
   Population::ParrentSelectionTechnique ParentSelction,
   Population::DeletetionTechnique Deletetion,
   Population::FitnessTechnique Fitness,
   Population::VariableLength Variable,
   int baseStates
   ):Population(
      Operation,numberofIndividuals,numberofTrials,
      GenecticDeversity,BitMutationRate,CrossOverRate,
      ReproductionTechniques,ParentSelction,
      Deletetion,Fitness,
      Variable,baseStates
      )
{
   fprintf(stderr,"In Alpha Construtor\n");
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
   BaseString *current = new BaseString(13,13);
   BaseString *next    = new BaseString(13,13);
   for ( int i = 0 ; i < 13 ; i++ )
   {
      current->set(i,random() % 13);
   }
   int fitness = 0;
   int iter = 0;
   while (fitness < 650)
   {
      for ( int i = 0 ; i < 13 ; i++ )
      {
	 next->set(i,current->test(i));
      }
      int index = random() % 13;
      int value = random() % 13;
      next->set(index,value);
      int nextFit = FitnessFunction(next);
      if (nextFit > fitness)
      {
	 delete current;
	 current = next;
	 next = new BaseString(13,13);
	 fitness = nextFit;
	 FitnessPrint(current);
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



