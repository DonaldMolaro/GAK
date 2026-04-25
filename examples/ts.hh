#pragma once

// Traveling salesman example built on the GA population engine.

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
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
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
