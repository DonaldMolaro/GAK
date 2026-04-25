#include <cmath>
#include <cstdio>
#include <ostream>

#include "base.hh"
#include "population.hh"
#include "nqueens.hh"

NQueens::NQueens(const Population::Settings& settings)
   : Population(settings)
{
}

double NQueens::evaluateFitness(const BaseString& genes)
{
   const int boardSize = genes.length();
   int nonAttackingPairs = 0;

   for ( int column = 0 ; column < boardSize ; column++ )
   {
      for ( int otherColumn = column + 1 ; otherColumn < boardSize ; otherColumn++ )
      {
         const int row = genes.valueAt(column);
         const int otherRow = genes.valueAt(otherColumn);
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

void NQueens::printCandidate(const BaseString& genes, std::ostream& out)
{
   out << "Queens:";
   for ( int column = 0 ; column < genes.length() ; column++ )
   {
      out << ' ' << genes.valueAt(column);
   }
   out << '\n';

   for ( int row = 0 ; row < genes.length() ; row++ )
   {
      for ( int column = 0 ; column < genes.length() ; column++ )
      {
         out << (genes.valueAt(column) == row ? 'Q' : '.');
      }
      out << '\n';
   }
}
