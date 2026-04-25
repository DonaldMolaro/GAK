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
   TravelingSalesman(const Population::Settings& settings,
                     int gridSize = 0);
  
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) override;
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
