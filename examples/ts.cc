// Traveling salesman example built on the GA population engine.
#include <cmath>
#include <iostream>
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
   : settings_(settings),
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

   randomGenerator.seed(settings_.useFixedRandomSeed ? settings_.randomSeed
                                                     : std::random_device{}());
   initializeCityCoordinates();
}

void TravelingSalesman::validatePopulation(const Population& population) const
{
   if (population.settings().geneticDiversity != settings_.geneticDiversity ||
       population.settings().baseStates != settings_.baseStates ||
       population.settings().variableLength != settings_.variableLength)
   {
      throw GAFatalException(__FILE__,__LINE__,"TravelingSalesman problem settings do not match population settings");
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
	   Square(static_cast<double>(cityCoordinates_[genes.valueAt(i)].first) - static_cast<double>(cityCoordinates_[genes.valueAt(i+1)].first))
	    + Square(static_cast<double>(cityCoordinates_[genes.valueAt(i)].second) - static_cast<double>(cityCoordinates_[genes.valueAt(i+1)].second))
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
      if (cityCoordinates_[i] == coordinate)
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
   cityCoordinates_.assign(numCities, Coordinate(0, 0));

   int allocated = 0;
   while (allocated < numCities)
   {
      const Coordinate candidate = randomCoordinate();
      if (hasCityCoordinate(allocated, candidate))
      {
         continue;
      }

      cityCoordinates_[allocated] = candidate;
      allocated++;
   }
}

void TravelingSalesman::writeCityList(std::ostream& out) const
{
   out << "Problem input:\n";
   out << "City list:\n";
   for ( int i = 0 ; i < numCities ; i++ )
   {
      out << static_cast<char>(i + 'a') << ":("
          << cityCoordinates_[i].first << ','
          << cityCoordinates_[i].second << ")\n";
   }
}
