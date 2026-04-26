#include <cmath>
#include <cstdio>
#include <ostream>
#include <vector>

#include "base.hh"
#include "except.hh"
#include "population.hh"
#include "latinsquare.hh"

void LatinSquare::validatePopulation(const Population& population) const
{
   const Population::Settings& settings = population.settings();
   if (settings.chromosomeLength <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"LatinSquare requires a positive chromosome length");
   }

   const int size = static_cast<int>(std::sqrt(static_cast<double>(settings.chromosomeLength)));
   if ((size * size) != settings.chromosomeLength)
   {
      throw GAFatalException(__FILE__,__LINE__,"LatinSquare expects a square chromosome length");
   }

   if (settings.baseStates != size)
   {
      throw GAFatalException(__FILE__,__LINE__,"LatinSquare expects baseStates to equal the square width");
   }
}

int LatinSquare::squareSize(const BaseString& genes) const
{
   const int size = static_cast<int>(std::sqrt(static_cast<double>(genes.length())));
   if ((size * size) != genes.length())
   {
      throw GAFatalException(__FILE__,__LINE__,"LatinSquare expects square chromosome instances");
   }
   return size;
}

double LatinSquare::evaluateFitness(const BaseString& genes)
{
   const int size = squareSize(genes);
   int score = 0;

   for ( int row = 0 ; row < size ; row++ )
   {
      std::vector<int> seen(size, 0);
      for ( int column = 0 ; column < size ; column++ )
      {
        const int value = genes.valueAt((row * size) + column);
         if (value < 0 || value >= size)
         {
            throw GAFatalException(__FILE__,__LINE__,"LatinSquare encountered an out-of-range symbol");
         }
         if (!seen[value])
         {
            seen[value] = 1;
            score++;
         }
      }
   }

   for ( int column = 0 ; column < size ; column++ )
   {
      std::vector<int> seen(size, 0);
      for ( int row = 0 ; row < size ; row++ )
      {
        const int value = genes.valueAt((row * size) + column);
         if (value < 0 || value >= size)
         {
            throw GAFatalException(__FILE__,__LINE__,"LatinSquare encountered an out-of-range symbol");
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

void LatinSquare::printCandidate(const BaseString& genes, std::ostream& out) const
{
   const int size = squareSize(genes);
   out << "Latin square (" << size << "x" << size << "):\n";
   for ( int row = 0 ; row < size ; row++ )
   {
      for ( int column = 0 ; column < size ; column++ )
      {
         out << genes.valueAt((row * size) + column);
         if (column + 1 < size)
         {
            out << ' ';
         }
      }
      out << '\n';
   }
}
