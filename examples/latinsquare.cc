#include <cmath>
#include <cstdio>
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

LatinSquare::LatinSquare(
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
   : LatinSquare(Population::Configuration{Operation,
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

int LatinSquare::squareSize(const BaseString *b) const
{
   const int size = static_cast<int>(std::sqrt(static_cast<double>(b->length())));
   if ((size * size) != b->length())
   {
      throw GAFatalException(__FILE__,__LINE__,"LatinSquare expects square chromosome instances");
   }
   return size;
}

double LatinSquare::FitnessFunction(BaseString *b)
{
   const int size = squareSize(b);
   int score = 0;

   for ( int row = 0 ; row < size ; row++ )
   {
      std::vector<int> seen(size, 0);
      for ( int column = 0 ; column < size ; column++ )
      {
        const int value = b->test((row * size) + column);
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
        const int value = b->test((row * size) + column);
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

void LatinSquare::FitnessPrint(BaseString *b)
{
   const int size = squareSize(b);
   fprintf(stderr, "Latin square (%dx%d):\n", size, size);
   for ( int row = 0 ; row < size ; row++ )
   {
      for ( int column = 0 ; column < size ; column++ )
      {
         fprintf(stderr, "%d", b->test((row * size) + column));
         if (column + 1 < size)
         {
            fprintf(stderr, " ");
         }
      }
      fprintf(stderr, "\n");
   }
}
