#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "dome.hh"

Dome::Dome(const Population::Options& options)
   : Dome(options.toConfiguration())
{
}

Dome::Dome(const Population::Configuration& configuration)
   : Population(configuration)
{
}

Dome::Dome(
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
   : Dome(Population::Configuration{Operation,
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


double Dome::sqr(double x)
{
   return x * x;
}

double Dome::FitnessFunction(BaseString *b)
{
   int x1 = decode(b, 0,16);
   int y1 = decode(b,16,32);
   double x2 = ((double)x1) / 100.0;
   double y2 = ((double)y1) / 100.0;

   double res = (sqr(x2)+sqr(y2))+1.0;
   return (int)(res * 100.0);
};


void Dome::FitnessPrint(BaseString *b)
{
   int x1 = decode(b,0,16);
   int y1 = decode(b,16,32);

   double x2 = ((double)x1) / 100.0;
   double y2 = ((double)y1) / 100.0;
   
   fprintf(stderr,"X ( %6.2f ) Y ( %6.2f ) ",x2,y2);
}
