#include <cstdio>
#include <vector>

#include <ostream>

#include "base.hh"
#include "except.hh"
#include "population.hh"
#include "sudoku.hh"

const int Sudoku::kPuzzle[Sudoku::kCellCount] = {
   5, 3, 0, 0, 7, 0, 0, 0, 0,
   6, 0, 0, 1, 9, 5, 0, 0, 0,
   0, 9, 8, 0, 0, 0, 0, 6, 0,
   8, 0, 0, 0, 6, 0, 0, 0, 3,
   4, 0, 0, 8, 0, 3, 0, 0, 1,
   7, 0, 0, 0, 2, 0, 0, 0, 6,
   0, 6, 0, 0, 0, 0, 2, 8, 0,
   0, 0, 0, 4, 1, 9, 0, 0, 5,
   0, 0, 0, 0, 8, 0, 0, 7, 9
};

void Sudoku::validatePopulation(const Population& population) const
{
   const Population::Settings& settings = population.settings();
   if (settings.geneticDiversity != kCellCount)
   {
      throw GAFatalException(__FILE__,__LINE__,"Sudoku expects 81 genes");
   }

   if (settings.baseStates != kBoardSize)
   {
      throw GAFatalException(__FILE__,__LINE__,"Sudoku expects 9 symbol states");
   }
}

int Sudoku::uniquenessScoreForRow(const BaseString& genes, int row) const
{
   std::vector<int> seen(kBoardSize, 0);
   int score = 0;
   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      const int value = genes.valueAt((row * kBoardSize) + column);
      if (value < 0 || value >= kBoardSize)
      {
         throw GAFatalException(__FILE__,__LINE__,"Sudoku encountered an out-of-range symbol");
      }
      if (!seen[value])
      {
         seen[value] = 1;
         score++;
      }
   }
   return score;
}

int Sudoku::uniquenessScoreForColumn(const BaseString& genes, int column) const
{
   std::vector<int> seen(kBoardSize, 0);
   int score = 0;
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      const int value = genes.valueAt((row * kBoardSize) + column);
      if (value < 0 || value >= kBoardSize)
      {
         throw GAFatalException(__FILE__,__LINE__,"Sudoku encountered an out-of-range symbol");
      }
      if (!seen[value])
      {
         seen[value] = 1;
         score++;
      }
   }
   return score;
}

int Sudoku::uniquenessScoreForBox(const BaseString& genes, int boxRow, int boxColumn) const
{
   std::vector<int> seen(kBoardSize, 0);
   int score = 0;
   for ( int rowOffset = 0 ; rowOffset < kSubgridSize ; rowOffset++ )
   {
      for ( int columnOffset = 0 ; columnOffset < kSubgridSize ; columnOffset++ )
      {
         const int row = (boxRow * kSubgridSize) + rowOffset;
         const int column = (boxColumn * kSubgridSize) + columnOffset;
         const int value = genes.valueAt((row * kBoardSize) + column);
         if (value < 0 || value >= kBoardSize)
         {
            throw GAFatalException(__FILE__,__LINE__,"Sudoku encountered an out-of-range symbol");
         }
         if (!seen[value])
         {
            seen[value] = 1;
            score++;
         }
      }
   }
   return score;
}

int Sudoku::givenConsistencyScore(const BaseString& genes) const
{
   int score = 0;
   for ( int cell = 0 ; cell < kCellCount ; cell++ )
   {
      if (kPuzzle[cell] == 0)
      {
         continue;
      }

      if ((genes.valueAt(cell) + 1) == kPuzzle[cell])
      {
         score += kBoardSize;
      }
   }
   return score;
}

double Sudoku::evaluateFitness(const BaseString& genes)
{
   int score = givenConsistencyScore(genes);

   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      score += uniquenessScoreForRow(genes, row);
   }

   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      score += uniquenessScoreForColumn(genes, column);
   }

   for ( int boxRow = 0 ; boxRow < kSubgridSize ; boxRow++ )
   {
      for ( int boxColumn = 0 ; boxColumn < kSubgridSize ; boxColumn++ )
      {
         score += uniquenessScoreForBox(genes, boxRow, boxColumn);
      }
   }

   return score;
}

void Sudoku::printCandidate(const BaseString& genes, std::ostream& out) const
{
   out << "Sudoku candidate:\n";
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      for ( int column = 0 ; column < kBoardSize ; column++ )
      {
         out << (genes.valueAt((row * kBoardSize) + column) + 1);
         if (column == 2 || column == 5)
         {
            out << " | ";
         }
         else if (column + 1 < kBoardSize)
         {
            out << ' ';
         }
      }
      out << '\n';
      if (row == 2 || row == 5)
      {
         out << "------+-------+------\n";
      }
   }
}
