// Traveling salesman example built on the GA population engine.
#include <cmath>
#include <iostream>
#include <genetic.hh>
#include "except.hh"
#include "random.hh"
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
     numCities(settings.chromosomeLength),
     gridSize(requestedGridSize == 0 ? settings.chromosomeLength : requestedGridSize)
{
   if (gridSize <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"TravelingSalesman requires a positive grid size");
   }

   if (numCities <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"TravelingSalesman requires at least one city");
   }

   if (settings_.baseStates != numCities)
   {
      // Gene values index the city table, so the alphabet must be exactly
      // one symbol per city.
      throw GAFatalException(__FILE__,__LINE__,"TravelingSalesman requires baseStates == chromosomeLength (one state per city)");
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
   if (population.settings().chromosomeLength != settings_.chromosomeLength ||
       population.settings().baseStates != settings_.baseStates ||
       population.settings().variableLength != settings_.variableLength)
   {
      throw GAFatalException(__FILE__,__LINE__,"TravelingSalesman problem settings do not match population settings");
   }
}

double TravelingSalesman::evaluateFitness(const BaseString& genes)
{
   // Longer than any possible edge, so replacing a repeated city with a
   // missing one always shortens the tour: invalid tours can never win.
   const double PENALTYLENGTH = 2.0 * std::sqrt(2.0) * gridSize;
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

std::unique_ptr<Chromosome> TravelingSalesman::initializeCandidate(Population& population)
{
   std::vector<int> tour(numCities);
   for (int i = 0 ; i < numCities ; i++)
   {
      tour[i] = i;
   }
   gak::shuffle(tour.begin(), tour.end(), population.randomEngine());
   BaseString genes(numCities, numCities);
   for (int i = 0 ; i < numCities ; i++)
   {
      genes.setValue(i, tour[i]);
   }
   return std::make_unique<Chromosome>(std::move(genes),
                                       population.settings().variableLength == Population::VariableLengthMode::Variable,
                                       population.settings().baseStates);
}

// Order crossover (OX1): copy first[start, end) into the child, then fill the
// remaining positions, starting after `end`, with the cities missing from
// that slice in the order they appear in `second` (also starting after `end`).
BaseString TravelingSalesman::orderCrossover(const BaseString& first, const BaseString& second,
                                             int start, int end) const
{
   BaseString child(numCities, numCities);
   std::vector<int> used(numCities, 0);
   for (int i = start ; i < end ; i++)
   {
      child.setValue(i, first.valueAt(i));
      used[first.valueAt(i)] = 1;
   }
   int write = end % numCities;
   for (int offset = 0 ; offset < numCities ; offset++)
   {
      const int city = second.valueAt((end + offset) % numCities);
      if (used[city])
      {
         continue;
      }
      child.setValue(write, city);
      used[city] = 1;
      write = (write + 1) % numCities;
   }
   return child;
}

std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
TravelingSalesman::mateCandidates(Population& population, Chromosome& mother, Chromosome& father)
{
   const bool variable = population.settings().variableLength == Population::VariableLengthMode::Variable;
   std::mt19937& generator = population.randomEngine();
   if (!gak::chance(generator, population.settings().crossOverRate))
   {
      return mother.mate(father, 0.0, Chromosome::CrossoverType::SinglePoint, &generator);
   }
   int start = gak::uniformIndex(generator, numCities);
   int end = gak::uniformIndex(generator, numCities);
   if (start > end) std::swap(start, end);
   ++end;
   return std::make_pair(
      std::make_unique<Chromosome>(orderCrossover(mother.genes(), father.genes(), start, end),
                                   variable, population.settings().baseStates),
      std::make_unique<Chromosome>(orderCrossover(father.genes(), mother.genes(), start, end),
                                   variable, population.settings().baseStates));
}

void TravelingSalesman::mutateCandidate(Population& population, Chromosome& chromosome)
{
   if (numCities < 2)
   {
      return;
   }
   std::mt19937& generator = population.randomEngine();
   BaseString& genes = chromosome.genes();
   for (int i = 0 ; i < numCities ; i++)
   {
      if (gak::chance(generator, population.settings().bitMutationRate))
      {
         int other = gak::uniformIndex(generator, numCities - 1);
         if (other >= i) ++other;
         genes.swapValues(i, other);
      }
   }
}

void TravelingSalesman::printCandidate(const BaseString& genes, std::ostream& out) const
{
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
      out << static_cast<char>(genes.valueAt(i) + 'a');
   }
   out << " ::";
}

void TravelingSalesman::writeVisualizationJson(const BaseString& genes, std::ostream& out) const
{
   out << "{ \"type\": \"traveling_salesman\", \"cities\": [";
   for (int i = 0 ; i < numCities ; i++)
   {
      if (i > 0)
      {
         out << ", ";
      }
      out << "{ \"name\": \"" << static_cast<char>('a' + i)
          << "\", \"x\": " << cityCoordinates_[i].first
          << ", \"y\": " << cityCoordinates_[i].second << " }";
   }
   out << "], \"route\": [";
   for (int i = 0 ; i < genes.length() ; i++)
   {
      if (i > 0)
      {
         out << ", ";
      }
      out << genes.valueAt(i);
   }
   out << "] }";
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
   const int x = gak::uniformIndex(randomGenerator, gridSize);
   const int y = gak::uniformIndex(randomGenerator, gridSize);
   return Coordinate(x, y);
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
