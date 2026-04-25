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

double NQueens::FitnessFunction(const BaseString& b)
{
   const int boardSize = b.length();
   int nonAttackingPairs = 0;

   for ( int column = 0 ; column < boardSize ; column++ )
   {
      for ( int otherColumn = column + 1 ; otherColumn < boardSize ; otherColumn++ )
      {
         const int row = b.test(column);
         const int otherRow = b.test(otherColumn);
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

void NQueens::FitnessPrint(const BaseString& b)
{
   fprintf(stderr, "Queens:");
   for ( int column = 0 ; column < b.length() ; column++ )
   {
      fprintf(stderr, " %d", b.test(column));
   }
   fprintf(stderr, "\n");

   for ( int row = 0 ; row < b.length() ; row++ )
   {
      for ( int column = 0 ; column < b.length() ; column++ )
      {
         fprintf(stderr, "%c", b.test(column) == row ? 'Q' : '.');
      }
      fprintf(stderr, "\n");
   }
}
