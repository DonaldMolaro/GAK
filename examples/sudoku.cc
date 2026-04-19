#include <cstdio>
#include <vector>

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

Sudoku::Sudoku(const Population::Options& options)
   : Sudoku(options.toConfiguration())
{
}

Sudoku::Sudoku(const Population::Configuration& configuration)
   : Population(configuration)
{
   validateConfiguration(configuration);
}

Sudoku::Sudoku(
   Population::OperationTechnique Operation,
   int numberofIndividuals,
   int numberofTrials,
   int GenecticDeversity,
   double BitMutationRate,
   double CrossOverRate,
   Population::ReproductionTechniques ReproductionTechniques,
   Population::ParentSelectionTechnique ParentSelction,
   Population::DeletionTechnique Deletetion,
   Population::FitnessTechnique Fitness,
   Population::VariableLength Variable,
   int baseStates
   )
   : Sudoku(Population::Configuration{Operation,
                                      numberofIndividuals,
                                      numberofTrials,
                                      GenecticDeversity,
                                      BitMutationRate,
                                      CrossOverRate,
                                      ReproductionTechniques,
                                      ParentSelction,
                                      Deletetion,
                                      Fitness,
                                      Variable,
                                      baseStates})
{
}

void Sudoku::validateConfiguration(const Population::Configuration& configuration) const
{
   if (configuration.geneticDiversity != kCellCount)
   {
      throw GAFatalException(__FILE__,__LINE__,"Sudoku expects 81 genes");
   }

   if (configuration.baseStates != kBoardSize)
   {
      throw GAFatalException(__FILE__,__LINE__,"Sudoku expects 9 symbol states");
   }
}

int Sudoku::uniquenessScoreForRow(BaseString *b, int row) const
{
   std::vector<int> seen(kBoardSize, 0);
   int score = 0;
   for ( int column = 0 ; column < kBoardSize ; column++ )
   {
      const int value = b->test((row * kBoardSize) + column);
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

int Sudoku::uniquenessScoreForColumn(BaseString *b, int column) const
{
   std::vector<int> seen(kBoardSize, 0);
   int score = 0;
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      const int value = b->test((row * kBoardSize) + column);
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

int Sudoku::uniquenessScoreForBox(BaseString *b, int boxRow, int boxColumn) const
{
   std::vector<int> seen(kBoardSize, 0);
   int score = 0;
   for ( int rowOffset = 0 ; rowOffset < kSubgridSize ; rowOffset++ )
   {
      for ( int columnOffset = 0 ; columnOffset < kSubgridSize ; columnOffset++ )
      {
         const int row = (boxRow * kSubgridSize) + rowOffset;
         const int column = (boxColumn * kSubgridSize) + columnOffset;
         const int value = b->test((row * kBoardSize) + column);
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

int Sudoku::givenConsistencyScore(BaseString *b) const
{
   int score = 0;
   for ( int cell = 0 ; cell < kCellCount ; cell++ )
   {
      if (kPuzzle[cell] == 0)
      {
         continue;
      }

      if ((b->test(cell) + 1) == kPuzzle[cell])
      {
         score += kBoardSize;
      }
   }
   return score;
}

double Sudoku::FitnessFunction(BaseString *b)
{
   int score = givenConsistencyScore(b);

   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      score += uniquenessScoreForRow(b, row);
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

void Sudoku::FitnessPrint(BaseString *b)
{
   fprintf(stderr, "Sudoku candidate:\n");
   for ( int row = 0 ; row < kBoardSize ; row++ )
   {
      for ( int column = 0 ; column < kBoardSize ; column++ )
      {
         fprintf(stderr, "%d", b->test((row * kBoardSize) + column) + 1);
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
