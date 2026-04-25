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

TravelingSalesman::TravelingSalesman(const Population::Settings& settings,
                                     int requestedGridSize)
   : Population(settings),
     numCities(settings.geneticDiversity),
     gridSize(requestedGridSize == 0 ? settings.geneticDiversity : requestedGridSize)
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

double TravelingSalesman::evaluateFitness(const BaseString& genes)
{
   const double PENALTYLENGTH = 250.0;
   std::vector<int> visited(numCities, 0);

   double length = 0.0;

   for ( int i = 0 ; i < genes.length()-1 ; i++ )
   {
      visited[genes.valueAt(i)] = 1;
      visited[genes.valueAt(i+1)] = 1;

      double clength = 
	 std::sqrt(
	   Square(static_cast<double>(xCoordinates[genes.valueAt(i)]) - static_cast<double>(xCoordinates[genes.valueAt(i+1)]))
	    + Square(static_cast<double>(yCoordinates[genes.valueAt(i)]) - static_cast<double>(yCoordinates[genes.valueAt(i+1)]))
	    );
      
      if (clength == 0) clength = PENALTYLENGTH;


      length += clength;
   }

   for ( int i = 0 ; i < numCities ; i++ )
   {
      if (!visited[i]) length += PENALTYLENGTH;
   }
   return static_cast<int>(length + 0.5);
}


void TravelingSalesman::printCandidate(const BaseString& genes, std::ostream& out) const
{
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
      out << static_cast<char>(genes.valueAt(i) + 'a');
   }
   out << " ::";
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
