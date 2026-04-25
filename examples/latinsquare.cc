#include <cmath>
#include <cstdio>
#include <ostream>
#include <vector>

#include "base.hh"
#include "except.hh"
#include "population.hh"
#include "latinsquare.hh"

LatinSquare::LatinSquare(const Population::Options& options)
   : LatinSquare(options.toConfiguration())
{
}

LatinSquare::LatinSquare(const Population::Configuration& configuration)
   : Population(configuration)
{
   if (configuration.geneticDiversity <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"LatinSquare requires a positive genetic diversity");
   }

   const int size = static_cast<int>(std::sqrt(static_cast<double>(configuration.geneticDiversity)));
   if ((size * size) != configuration.geneticDiversity)
   {
      throw GAFatalException(__FILE__,__LINE__,"LatinSquare expects a square chromosome length");
   }

   if (configuration.baseStates != size)
   {
      throw GAFatalException(__FILE__,__LINE__,"LatinSquare expects baseStates to equal the square width");
   }
}

int LatinSquare::squareSize(const BaseString& b) const
{
   const int size = static_cast<int>(std::sqrt(static_cast<double>(b.length())));
   if ((size * size) != b.length())
   {
      throw GAFatalException(__FILE__,__LINE__,"LatinSquare expects square chromosome instances");
   }
   return size;
}

double LatinSquare::FitnessFunction(const BaseString& b)
{
   const int size = squareSize(b);
   int score = 0;

   for ( int row = 0 ; row < size ; row++ )
   {
      std::vector<int> seen(size, 0);
      for ( int column = 0 ; column < size ; column++ )
      {
        const int value = b.test((row * size) + column);
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
        const int value = b.test((row * size) + column);
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

void LatinSquare::FitnessPrint(const BaseString& b, std::ostream& out)
{
   const int size = squareSize(b);
   out << "Latin square (" << size << "x" << size << "):\n";
   for ( int row = 0 ; row < size ; row++ )
   {
      for ( int column = 0 ; column < size ; column++ )
      {
         out << b.test((row * size) + column);
         if (column + 1 < size)
         {
            out << ' ';
         }
      }
      out << '\n';
   }
}
