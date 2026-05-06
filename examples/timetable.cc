#include <ostream>

#include "base.hh"
#include "except.hh"
#include "population.hh"
#include "timetable.hh"

const Timetable::Conflict Timetable::kConflicts[Timetable::kConflictCount] = {
   {0, 1}, {0, 3}, {1, 2}, {1, 7}, {2, 4},
   {2, 6}, {3, 5}, {3, 7}, {4, 6}, {5, 7}
};

const int Timetable::kPreferredSlots[Timetable::kCourseCount] = {
   0, 1, 2, 3, 0, 1, 3, 2
};

const char* Timetable::kCourseNames[Timetable::kCourseCount] = {
   "Calculus", "Physics", "Chemistry", "Biology",
   "History", "Literature", "Economics", "Programming"
};

double Timetable::evaluateFitness(const BaseString& genes)
{
   int score = 0;

   for (int conflict = 0 ; conflict < kConflictCount ; conflict++)
   {
      const int leftSlot = genes.valueAt(kConflicts[conflict].left);
      const int rightSlot = genes.valueAt(kConflicts[conflict].right);
      if (leftSlot != rightSlot)
      {
         score += kConflictReward;
      }
   }

   for (int course = 0 ; course < kCourseCount ; course++)
   {
      if (genes.valueAt(course) == kPreferredSlots[course])
      {
         score += 1;
      }
   }

   return score;
}

void Timetable::printCandidate(const BaseString& genes, std::ostream& out) const
{
   out << "Timetable:";
   for (int course = 0 ; course < kCourseCount ; course++)
   {
      out << ' ' << kCourseNames[course] << "=slot-" << genes.valueAt(course);
   }

   int conflicts = 0;
   int preferencesMet = 0;
   for (int conflict = 0 ; conflict < kConflictCount ; conflict++)
   {
      if (genes.valueAt(kConflicts[conflict].left) == genes.valueAt(kConflicts[conflict].right))
      {
         conflicts++;
      }
   }
   for (int course = 0 ; course < kCourseCount ; course++)
   {
      if (genes.valueAt(course) == kPreferredSlots[course])
      {
         preferencesMet++;
      }
   }
   out << " | conflicts=" << conflicts
       << " preferred=" << preferencesMet << '/' << kCourseCount
       << " ::";
}

void Timetable::writeVisualizationJson(const BaseString& genes, std::ostream& out) const
{
   out << "{ \"type\": \"timetable\", \"slot_count\": " << kSlotCount << ", \"courses\": [";
   for (int course = 0 ; course < kCourseCount ; course++)
   {
      if (course > 0)
      {
         out << ", ";
      }
      out << "{ \"name\": \"" << kCourseNames[course]
          << "\", \"slot\": " << genes.valueAt(course)
          << ", \"preferred_slot\": " << kPreferredSlots[course]
          << ", \"preferred\": " << (genes.valueAt(course) == kPreferredSlots[course] ? "true" : "false")
          << " }";
   }
   out << "], \"conflicts\": [";
   for (int conflict = 0 ; conflict < kConflictCount ; conflict++)
   {
      if (conflict > 0)
      {
         out << ", ";
      }
      const bool activeConflict = genes.valueAt(kConflicts[conflict].left) == genes.valueAt(kConflicts[conflict].right);
      out << "{ \"left\": " << kConflicts[conflict].left
          << ", \"right\": " << kConflicts[conflict].right
          << ", \"active\": " << (activeConflict ? "true" : "false") << " }";
   }
   out << "] }";
}

bool Timetable::hasReachedSolution(const Population&, const BaseString&, double fitness) const
{
   return fitness >= (kConflictCount * kConflictReward) + kCourseCount;
}

void Timetable::validatePopulation(const Population& population) const
{
   if (population.settings().chromosomeLength != kCourseCount)
   {
      throw GAFatalException(__FILE__, __LINE__,
                             "Timetable requires one gene per course.");
   }
   if (population.settings().baseStates != kSlotCount)
   {
      throw GAFatalException(__FILE__, __LINE__,
                             "Timetable requires exactly four timetable slots.");
   }
}
