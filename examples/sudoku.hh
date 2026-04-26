#pragma once


#include <iosfwd>

#include "population.hh"

class Sudoku : public PopulationProblem
{
public:
   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   bool hasReachedSolution(const Population& population,
                           const BaseString& genes,
                           double fitness) const override;

private:
   static const int kBoardSize = 9;
   static const int kCellCount = 81;
   static const int kSubgridSize = 3;
   static const int kPuzzle[kCellCount];

   int uniquenessScoreForRow(const BaseString& genes, int row) const;
   int uniquenessScoreForColumn(const BaseString& genes, int column) const;
   int uniquenessScoreForBox(const BaseString& genes, int boxRow, int boxColumn) const;
   int givenConsistencyScore(const BaseString& genes) const;
};
