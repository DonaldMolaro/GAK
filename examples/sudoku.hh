#ifndef __GA_Sudoku_hh__
#define __GA_Sudoku_hh__

#include <iosfwd>

#include "population.hh"

class Sudoku : public Population
{
public:
   explicit Sudoku(const Population::Settings& settings);

   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;

private:
   static const int kBoardSize = 9;
   static const int kCellCount = 81;
   static const int kSubgridSize = 3;
   static const int kPuzzle[kCellCount];

   void validateSettings(const Population::Settings& settings) const;
   int uniquenessScoreForRow(const BaseString& genes, int row) const;
   int uniquenessScoreForColumn(const BaseString& genes, int column) const;
   int uniquenessScoreForBox(const BaseString& genes, int boxRow, int boxColumn) const;
   int givenConsistencyScore(const BaseString& genes) const;
};

#endif
