#pragma once

#include "population.hh"

class Timetable : public PopulationProblem
{
public:
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   void writeVisualizationJson(const BaseString& genes, std::ostream& out) const override;
   bool hasReachedSolution(const Population& population,
                           const BaseString& genes,
                           double fitness) const override;
   void validatePopulation(const Population& population) const override;

   static const int kCourseCount = 8;
   static const int kSlotCount = 4;
   static const int kConflictCount = 10;
   static const int kConflictReward = 10;

private:
   struct Conflict
   {
      int left;
      int right;
   };

   static const Conflict kConflicts[kConflictCount];
   static const int kPreferredSlots[kCourseCount];
   static const char* kCourseNames[kCourseCount];
};
