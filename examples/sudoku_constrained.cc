#include <algorithm>
#include <cstdio>
#include <ctime>
#include <ostream>
#include <vector>

#include "base.hh"
#include "chromosome.hh"
#include "except.hh"
#include "population.hh"
#include "sudoku_constrained.hh"

const int SudokuConstrained::kPuzzle[SudokuConstrained::kCellCount] = {
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

SudokuConstrained::SudokuConstrained(const Population::Settings& settings)
   : Population(settings)
{
   validateSettings(settings);

   mutableColumnsByRow_.resize(kBoardSize);
   missingDigitsByRow_.resize(kBoardSize);

   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      std::vector<int> seen(kBoardSize, 0);
      for ( int column = 0 ; column < kBoardSize ; column++ )
      {
         const int given = kPuzzle[(row * kBoardSize) + column];
         if (given == 0)
         {
            mutableColumnsByRow_[row].push_back(column);
         }
         else
         {
            seen[given - 1] = 1;
         }
      }

      for ( int digit = 0 ; digit < kBoardSize ; digit++ )
      {
         if (!seen[digit])
         {
            missingDigitsByRow_[row].push_back(digit);
         }
      }
   }
}

void SudokuConstrained::validateSettings(const Population::Settings& settings) const
{
   if (settings.geneticDiversity != kCellCount)
   {
      throw GAFatalException(__FILE__,__LINE__,"SudokuConstrained expects 81 genes");
   }

   if (settings.baseStates != kBoardSize)
   {
      throw GAFatalException(__FILE__,__LINE__,"SudokuConstrained expects 9 symbol states");
   }
}

int SudokuConstrained::uniquenessScoreForColumn(const BaseString& b, int column) const
{
   std::vector<int> seen(kBoardSize, 0);
   int score = 0;
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      const int value = b.valueAt((row * kBoardSize) + column);
      if (value < 0 || value >= kBoardSize)
      {
         throw GAFatalException(__FILE__,__LINE__,"SudokuConstrained encountered an out-of-range symbol");
      }
      if (!seen[value])
      {
         seen[value] = 1;
         score++;
      }
   }
   return score;
}

int SudokuConstrained::uniquenessScoreForBox(const BaseString& b, int boxRow, int boxColumn) const
{
   std::vector<int> seen(kBoardSize, 0);
   int score = 0;
   for ( int rowOffset = 0 ; rowOffset < kSubgridSize ; rowOffset++ )
   {
      for ( int columnOffset = 0 ; columnOffset < kSubgridSize ; columnOffset++ )
      {
         const int row = (boxRow * kSubgridSize) + rowOffset;
         const int column = (boxColumn * kSubgridSize) + columnOffset;
         const int value = b.valueAt((row * kBoardSize) + column);
         if (value < 0 || value >= kBoardSize)
         {
            throw GAFatalException(__FILE__,__LINE__,"SudokuConstrained encountered an out-of-range symbol");
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

int SudokuConstrained::givenConsistencyScore(const BaseString& b) const
{
   int score = 0;
   for ( int cell = 0 ; cell < kCellCount ; cell++ )
   {
      if (kPuzzle[cell] == 0)
      {
         continue;
      }

      if ((b.valueAt(cell) + 1) == kPuzzle[cell])
      {
         score += kBoardSize;
      }
   }
   return score;
}

double SudokuConstrained::evaluateFitness(const BaseString& genes)
{
   int score = givenConsistencyScore(genes);

   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      if (rowIsValidPermutation(genes, row))
      {
         score += kBoardSize;
      }
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

void SudokuConstrained::printCandidate(const BaseString& genes, std::ostream& out) const
{
   out << "Constraint-aware Sudoku candidate:\n";
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

BaseString SudokuConstrained::cloneBoard(const BaseString& source) const
{
   BaseString clone(kCellCount, kBoardSize);
   for ( int cell = 0 ; cell < kCellCount ; cell++ )
   {
      clone.setValue(cell, source.valueAt(cell));
   }
   return clone;
}

void SudokuConstrained::fillRowFromParent(BaseString& destination, const BaseString& source, int row) const
{
   const int rowStart = row * kBoardSize;
   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      destination.setValue(rowStart + column, source.valueAt(rowStart + column));
   }
}

void SudokuConstrained::initializeRow(BaseString& board, int row)
{
   const int rowStart = row * kBoardSize;
   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      const int given = kPuzzle[rowStart + column];
      if (given != 0)
      {
         board.setValue(rowStart + column, given - 1);
      }
   }

   std::vector<int> digits = missingDigitsByRow_[row];
   std::shuffle(digits.begin(), digits.end(), randomEngine());
   for ( int i = 0 ; i < static_cast<int>(mutableColumnsByRow_[row].size()) ; i++ )
   {
      board.setValue(rowStart + mutableColumnsByRow_[row][i], digits[i]);
   }
}

bool SudokuConstrained::rowIsValidPermutation(const BaseString& board, int row) const
{
   std::vector<int> seen(kBoardSize, 0);
   const int rowStart = row * kBoardSize;
   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      const int value = board.valueAt(rowStart + column);
      if (value < 0 || value >= kBoardSize || seen[value])
      {
         return false;
      }
      seen[value] = 1;
   }
   return true;
}

std::unique_ptr<Chromosome> SudokuConstrained::initializeCandidate(Population&)
{
   BaseString board(kCellCount, kBoardSize);
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      initializeRow(board, row);
   }
   return std::make_unique<Chromosome>(std::move(board),
                                       settings().variableLength == Population::VariableLengthMode::Variable,
                                       settings().baseStates);
}

std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
SudokuConstrained::mateCandidates(Population&, Chromosome& mother, Chromosome& father)
{
   std::uniform_real_distribution<double> probability(0.0, 1.0);
   std::uniform_int_distribution<int> parent_choice(0, 1);

   if (probability(randomEngine()) >= settings().crossOverRate)
   {
      return std::make_pair(
         std::make_unique<Chromosome>(cloneBoard(mother.genes()),
                                      settings().variableLength == Population::VariableLengthMode::Variable,
                                      settings().baseStates),
         std::make_unique<Chromosome>(cloneBoard(father.genes()),
                                      settings().variableLength == Population::VariableLengthMode::Variable,
                                      settings().baseStates));
   }

   BaseString first(kCellCount, kBoardSize);
   BaseString second(kCellCount, kBoardSize);

   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      const bool useMotherForFirst = parent_choice(randomEngine()) == 0;
      fillRowFromParent(first, useMotherForFirst ? mother.genes() : father.genes(), row);
      fillRowFromParent(second, useMotherForFirst ? father.genes() : mother.genes(), row);
   }

   return std::make_pair(
      std::make_unique<Chromosome>(std::move(first),
                                   settings().variableLength == Population::VariableLengthMode::Variable,
                                   settings().baseStates),
      std::make_unique<Chromosome>(std::move(second),
                                   settings().variableLength == Population::VariableLengthMode::Variable,
                                   settings().baseStates));
}

void SudokuConstrained::mutateCandidate(Population&, Chromosome& chromosome)
{
   const double rowMutationRate = std::min(1.0, settings().bitMutationRate * kBoardSize);
   std::uniform_real_distribution<double> probability(0.0, 1.0);

   BaseString& board = chromosome.genes();
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      const RowColumns& mutableColumns = mutableColumnsByRow_[row];
      if (mutableColumns.size() < 2)
      {
         continue;
      }

      if (probability(randomEngine()) >= rowMutationRate)
      {
         continue;
      }

      std::uniform_int_distribution<int> columnIndex(0, static_cast<int>(mutableColumns.size()) - 1);
      int first = columnIndex(randomEngine());
      int second = columnIndex(randomEngine());
      while (second == first)
      {
         second = columnIndex(randomEngine());
      }

      const int rowStart = row * kBoardSize;
      const int firstCell = rowStart + mutableColumns[first];
      const int secondCell = rowStart + mutableColumns[second];
      board.swapValues(firstCell, secondCell);
   }
}
