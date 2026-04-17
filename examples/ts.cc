/*
 * Traveling salesman problem solved with a high order GA.
 * Donald Molaro - Copyright 1997.
 *
 * file            ts.cc
 *
 * Implements the virtual functions required for a complete population object.
 * required functions are the fitness function, which tells how well a particular
 * chromosome string will work & the fitness print function which can print out 
 * a chromosome string.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <genetic.hh>
#include "ts.hh"

#define sqr(f)  ((f)*(f))


TravelingSalesman::TravelingSalesman(
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
   int baseStates,
   int gridS
   ):Population(
      Operation,numberofIndividuals,numberofTrials,
      GenecticDeversity,BitMutationRate,CrossOverRate,
      ReproductionTechniques,ParentSelction,
      Deletetion,Fitness,
      Variable,baseStates
      )
{
   fprintf(stderr,"In TravelingSalesman Construtor\n");

   NumCities = GenecticDeversity;
   
   if (gridS == 0)
   {
      GridSize = NumCities;
   }
   else
   {
      GridSize = gridS;
   }

   XCoord = new int[GridSize];
   YCoord = new int[GridSize];
   
   int allocated = 0;
   while ( allocated < NumCities )
   {
      int x = random() % GridSize;
      int y = random() % GridSize;
      int found = 0;
      for ( int i = 0 ; i < allocated ; i++ )
      {
	 if ( (XCoord[i] == x) && (YCoord[i] == y) )
	 {
	    found = 1;
	 }
      }

      if (found == 0)
      {
	 XCoord[allocated] = x;
	 YCoord[allocated] = y;
	 allocated++;
      }
   }
   fprintf(stdout,"City List is:\n");
   for ( int i = 0 ; i < NumCities ; i++ )
   {
      fprintf(stdout,"%c:(%d,%d)\n",(i + 'a'),XCoord[i],YCoord[i]);
   }
   return;
}

double TravelingSalesman::FitnessFunction(BaseString *b)
{
   const double PENALTYLENGTH = 250.0;
   
   int visited[NumCities];

   for ( int i = 0 ; i < NumCities ; i++ )
   {
      visited[i] = 0;
   }

   double length = 0.0;

   for ( int i = 0 ; i < b->length()-1 ; i++ )
   {
      visited[b->test(i)] = 1;
      visited[b->test(i+1)] = 1;

      double clength = 
	 sqrt(
	   sqr( ((double)XCoord[b->test(i)])-((double)XCoord[b->test(i+1)]) )
	    + sqr( ((double)YCoord[b->test(i)])-((double)YCoord[b->test(i+1)]) )
	    );
      
      if (clength == 0) clength = PENALTYLENGTH;


      length += clength;
   }

   for ( int i = 0 ; i < NumCities ; i++ )
   {
      if (!visited[i]) length += PENALTYLENGTH;
   }
   return (int)(length+0.5);
}


void TravelingSalesman::FitnessPrint(BaseString *b)
{
   for ( int i = 0 ; i < b->length() ; i++ )
   {
      fprintf(stderr,"%c",b->test(i) + 'a');
   }
   fprintf(stderr," ::");
}


