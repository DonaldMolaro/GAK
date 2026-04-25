/*
 * Traveling salesman problem solved with a high order GA.
 * Donald Molaro - Copyright 1997.
 * Image Integration Inc.
 *
 * file            ts.hh
 *
 * Implements the traveling salesman class, with an inheritance to
 * the population object.
 */

#ifndef __GA_TS_hh__
#define __GA_TS_hh__
#include "population.hh"
#include <random>
#include <utility>
#include <vector>

class TravelingSalesman : public Population
{
public:
   TravelingSalesman(const Population::Options& options,
                     int gridS = 0);
   TravelingSalesman(const Population::Configuration& configuration,
                     int gridS = 0);
  
   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b, std::ostream& out) override;
private:
   using Coordinate = std::pair<int, int>;

   bool hasCityCoordinate(int allocated, const Coordinate& coordinate) const;
   Coordinate randomCoordinate();
   void initializeCityCoordinates();

   int numCities;
   int gridSize;
   std::vector<int> xCoordinates;
   std::vector<int> yCoordinates;
   std::mt19937 randomGenerator;
};
#endif
