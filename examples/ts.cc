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
#include <cstdio>
#include <cmath>
#include <iostream>
#include <ctime>
#include <genetic.hh>
#include "except.hh"
#include "ts.hh"

namespace
{
double Square(double value)
{
   return value * value;
}
}

TravelingSalesman::TravelingSalesman(const Population::Options& options,
                                     int gridS)
   : TravelingSalesman(options.toConfiguration(), gridS)
{
}

TravelingSalesman::TravelingSalesman(const Population::Configuration& configuration,
                                     int gridS)
   : Population(configuration),
     numCities(configuration.geneticDiversity),
     gridSize(gridS == 0 ? configuration.geneticDiversity : gridS)
{
   if (gridSize <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"TravelingSalesman requires a positive grid size");
   }

   if (numCities <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"TravelingSalesman requires at least one city");
   }

   if (numCities > (gridSize * gridSize))
   {
      throw GAFatalException(__FILE__,__LINE__,"TravelingSalesman grid is too small for unique city coordinates");
   }

   randomGenerator.seed(randomSeed());

   initializeCityCoordinates();

   fprintf(stdout,"City List is:\n");
   for ( int i = 0 ; i < numCities ; i++ )
   {
      fprintf(stdout,"%c:(%d,%d)\n",(i + 'a'),xCoordinates[i],yCoordinates[i]);
   }
}

TravelingSalesman::TravelingSalesman(
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
   int baseStates,
   int gridS
   )
   : TravelingSalesman(Population::Configuration{Operation,
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
                                                 baseStates},
                       gridS)
{
}

double TravelingSalesman::FitnessFunction(const BaseString& b)
{
   const double PENALTYLENGTH = 250.0;
   std::vector<int> visited(numCities, 0);

   double length = 0.0;

   for ( int i = 0 ; i < b.length()-1 ; i++ )
   {
      visited[b.test(i)] = 1;
      visited[b.test(i+1)] = 1;

      double clength = 
	 std::sqrt(
	   Square(static_cast<double>(xCoordinates[b.test(i)]) - static_cast<double>(xCoordinates[b.test(i+1)]))
	    + Square(static_cast<double>(yCoordinates[b.test(i)]) - static_cast<double>(yCoordinates[b.test(i+1)]))
	    );
      
      if (clength == 0) clength = PENALTYLENGTH;


      length += clength;
   }

   for ( int i = 0 ; i < numCities ; i++ )
   {
      if (!visited[i]) length += PENALTYLENGTH;
   }
   return (int)(length+0.5);
}


void TravelingSalesman::FitnessPrint(const BaseString& b)
{
   for ( int i = 0 ; i < b.length() ; i++ )
   {
      fprintf(stderr,"%c",b.test(i) + 'a');
   }
   fprintf(stderr," ::");
}

bool TravelingSalesman::hasCityCoordinate(int allocated, const Coordinate& coordinate) const
{
   for ( int i = 0 ; i < allocated ; i++ )
   {
      if (xCoordinates[i] == coordinate.first && yCoordinates[i] == coordinate.second)
      {
         return true;
      }
   }

   return false;
}

TravelingSalesman::Coordinate TravelingSalesman::randomCoordinate()
{
   std::uniform_int_distribution<int> distribution(0, gridSize - 1);
   return Coordinate(distribution(randomGenerator), distribution(randomGenerator));
}

void TravelingSalesman::initializeCityCoordinates()
{
   xCoordinates.assign(numCities, 0);
   yCoordinates.assign(numCities, 0);

   int allocated = 0;
   while (allocated < numCities)
   {
      const Coordinate candidate = randomCoordinate();
      if (hasCityCoordinate(allocated, candidate))
      {
         continue;
      }

      xCoordinates[allocated] = candidate.first;
      yCoordinates[allocated] = candidate.second;
      allocated++;
   }
}
