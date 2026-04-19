#include <cmath>
#include <cstdio>

#include "base.hh"
#include "population.hh"
#include "nqueens.hh"

NQueens::NQueens(const Population::Options& options)
   : NQueens(options.toConfiguration())
{
}

NQueens::NQueens(const Population::Configuration& configuration)
   : Population(configuration)
{
}

NQueens::NQueens(
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
   : NQueens(Population::Configuration{Operation,
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

double NQueens::FitnessFunction(BaseString *b)
{
   const int boardSize = b->length();
   int nonAttackingPairs = 0;

   for ( int column = 0 ; column < boardSize ; column++ )
   {
      for ( int otherColumn = column + 1 ; otherColumn < boardSize ; otherColumn++ )
      {
         const int row = b->test(column);
         const int otherRow = b->test(otherColumn);
         const bool sameRow = row == otherRow;
         const bool sameDiagonal = std::abs(row - otherRow) == (otherColumn - column);
         if (!sameRow && !sameDiagonal)
         {
            nonAttackingPairs++;
         }
      }
   }

   return nonAttackingPairs;
}

void NQueens::FitnessPrint(BaseString *b)
{
   fprintf(stderr, "Queens:");
   for ( int column = 0 ; column < b->length() ; column++ )
   {
      fprintf(stderr, " %d", b->test(column));
   }
   fprintf(stderr, "\n");

   for ( int row = 0 ; row < b->length() ; row++ )
   {
      for ( int column = 0 ; column < b->length() ; column++ )
      {
         fprintf(stderr, "%c", b->test(column) == row ? 'Q' : '.');
      }
      fprintf(stderr, "\n");
   }
}
