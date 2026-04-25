#include <algorithm>
#include <cstdio>
#include <ctime>
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

SudokuConstrained::SudokuConstrained(const Population::Options& options)
   : SudokuConstrained(options.toConfiguration())
{
}

SudokuConstrained::SudokuConstrained(const Population::Configuration& configuration)
   : Population(configuration)
{
   validateConfiguration(configuration);
   randomGenerator_.seed(randomSeed());

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

void SudokuConstrained::validateConfiguration(const Population::Configuration& configuration) const
{
   if (configuration.geneticDiversity != kCellCount)
   {
      throw GAFatalException(__FILE__,__LINE__,"SudokuConstrained expects 81 genes");
   }

   if (configuration.baseStates != kBoardSize)
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
      const int value = b.test((row * kBoardSize) + column);
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
         const int value = b.test((row * kBoardSize) + column);
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

      if ((b.test(cell) + 1) == kPuzzle[cell])
      {
         score += kBoardSize;
      }
   }
   return score;
}

double SudokuConstrained::FitnessFunction(const BaseString& b)
{
   int score = givenConsistencyScore(b);

   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      if (rowIsValidPermutation(&b, row))
      {
         score += kBoardSize;
      }
   }

   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      score += uniquenessScoreForColumn(b, column);
   }

   for ( int boxRow = 0 ; boxRow < kSubgridSize ; boxRow++ )
   {
      for ( int boxColumn = 0 ; boxColumn < kSubgridSize ; boxColumn++ )
      {
         score += uniquenessScoreForBox(b, boxRow, boxColumn);
      }
   }

   return score;
}

void SudokuConstrained::FitnessPrint(const BaseString& b)
{
   fprintf(stderr, "Constraint-aware Sudoku candidate:\n");
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      for ( int column = 0 ; column < kBoardSize ; column++ )
      {
         fprintf(stderr, "%d", b.test((row * kBoardSize) + column) + 1);
         if (column == 2 || column == 5)
         {
            fprintf(stderr, " | ");
         }
         else if (column + 1 < kBoardSize)
         {
            fprintf(stderr, " ");
         }
      }
      fprintf(stderr, "\n");
      if (row == 2 || row == 5)
      {
         fprintf(stderr, "------+-------+------\n");
      }
   }
}

std::unique_ptr<BaseString> SudokuConstrained::cloneBoard(const BaseString *source) const
{
   std::unique_ptr<BaseString> clone = std::make_unique<BaseString>(kCellCount, kBoardSize);
   for ( int cell = 0 ; cell < kCellCount ; cell++ )
   {
      clone->set(cell, source->test(cell));
   }
   return clone;
}

void SudokuConstrained::fillRowFromParent(BaseString *destination, const BaseString *source, int row) const
{
   const int rowStart = row * kBoardSize;
   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      destination->set(rowStart + column, source->test(rowStart + column));
   }
}

void SudokuConstrained::initializeRow(BaseString *board, int row)
{
   const int rowStart = row * kBoardSize;
   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      const int given = kPuzzle[rowStart + column];
      if (given != 0)
      {
         board->set(rowStart + column, given - 1);
      }
   }

   std::vector<int> digits = missingDigitsByRow_[row];
   std::shuffle(digits.begin(), digits.end(), randomGenerator_);
   for ( int i = 0 ; i < static_cast<int>(mutableColumnsByRow_[row].size()) ; i++ )
   {
      board->set(rowStart + mutableColumnsByRow_[row][i], digits[i]);
   }
}

bool SudokuConstrained::rowIsValidPermutation(const BaseString *board, int row) const
{
   std::vector<int> seen(kBoardSize, 0);
   const int rowStart = row * kBoardSize;
   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      const int value = board->test(rowStart + column);
      if (value < 0 || value >= kBoardSize || seen[value])
      {
         return false;
      }
      seen[value] = 1;
   }
   return true;
}

std::unique_ptr<Chromosome> SudokuConstrained::createInitialChromosome()
{
   std::unique_ptr<BaseString> board = std::make_unique<BaseString>(kCellCount, kBoardSize);
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      initializeRow(board.get(), row);
   }
   return std::make_unique<Chromosome>(board.release(),
                                       static_cast<unsigned int>(configuration().variableLength == Population::VariableLengthMode::Variable),
                                       configuration().baseStates);
}

std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
SudokuConstrained::mateChromosomes(Chromosome *mother, Chromosome *father)
{
   std::uniform_real_distribution<double> probability(0.0, 1.0);
   std::uniform_int_distribution<int> parent_choice(0, 1);

   if (probability(randomGenerator_) >= configuration().crossOverRate)
   {
      return std::make_pair(
         std::make_unique<Chromosome>(cloneBoard(&mother->chromosomeString()),
                                      static_cast<unsigned int>(configuration().variableLength == Population::VariableLengthMode::Variable),
                                      configuration().baseStates),
         std::make_unique<Chromosome>(cloneBoard(&father->chromosomeString()),
                                      static_cast<unsigned int>(configuration().variableLength == Population::VariableLengthMode::Variable),
                                      configuration().baseStates));
   }

   std::unique_ptr<BaseString> first = std::make_unique<BaseString>(kCellCount, kBoardSize);
   std::unique_ptr<BaseString> second = std::make_unique<BaseString>(kCellCount, kBoardSize);

   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      const bool useMotherForFirst = parent_choice(randomGenerator_) == 0;
      fillRowFromParent(first.get(), useMotherForFirst ? &mother->chromosomeString() : &father->chromosomeString(), row);
      fillRowFromParent(second.get(), useMotherForFirst ? &father->chromosomeString() : &mother->chromosomeString(), row);
   }

   return std::make_pair(
      std::make_unique<Chromosome>(std::move(first),
                                   static_cast<unsigned int>(configuration().variableLength == Population::VariableLengthMode::Variable),
                                   configuration().baseStates),
      std::make_unique<Chromosome>(std::move(second),
                                   static_cast<unsigned int>(configuration().variableLength == Population::VariableLengthMode::Variable),
                                   configuration().baseStates));
}

void SudokuConstrained::mutateChromosome(Chromosome *chromosome)
{
   const double rowMutationRate = std::min(1.0, configuration().bitMutationRate * kBoardSize);
   std::uniform_real_distribution<double> probability(0.0, 1.0);

   BaseString& board = chromosome->chromosomeString();
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      const RowColumns& mutableColumns = mutableColumnsByRow_[row];
      if (mutableColumns.size() < 2)
      {
         continue;
      }

      if (probability(randomGenerator_) >= rowMutationRate)
      {
         continue;
      }

      std::uniform_int_distribution<int> columnIndex(0, static_cast<int>(mutableColumns.size()) - 1);
      int first = columnIndex(randomGenerator_);
      int second = columnIndex(randomGenerator_);
      while (second == first)
      {
         second = columnIndex(randomGenerator_);
      }

      const int rowStart = row * kBoardSize;
      const int firstCell = rowStart + mutableColumns[first];
      const int secondCell = rowStart + mutableColumns[second];
      const int firstValue = board.test(firstCell);
      const int secondValue = board.test(secondCell);
      board.set(firstCell, secondValue);
      board.set(secondCell, firstValue);
   }
}
