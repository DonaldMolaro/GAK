#pragma once

// Traveling salesman example built on the GA population engine.

#include "population.hh"
#include <random>
#include <utility>
#include <vector>

class TravelingSalesman : public PopulationProblem
{
public:
   using Coordinate = std::pair<int, int>;

   TravelingSalesman(const Population::Settings& settings,
                     int gridSize = 0);
  
   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   int cityCount() const noexcept { return numCities; }
   int cityGridSize() const noexcept { return gridSize; }
   const std::vector<Coordinate>& cityCoordinates() const noexcept { return cityCoordinates_; }
   void writeCityList(std::ostream& out) const;
private:
   bool hasCityCoordinate(int allocated, const Coordinate& coordinate) const;
   Coordinate randomCoordinate();
   void initializeCityCoordinates();

   Population::Settings settings_;
   int numCities;
   int gridSize;
   std::vector<Coordinate> cityCoordinates_;
   std::mt19937 randomGenerator;
};
