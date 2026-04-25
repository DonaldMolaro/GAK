#ifndef __GA_Dome_hh__
#define __GA_Dome_hh__
#include "population.hh"

class Dome : public Population
{
   double sqr ( double x );
public:
   explicit Dome(const Population::Options& options);
   explicit Dome(const Population::Configuration& configuration);
   [[deprecated("Use Dome(const Population::Options&) or Dome(const Population::Configuration&)")]]
   Dome(
      Population::OperationTechnique Operation,
      int numberofIndividuals,
      int numberofTrials,
      int GenecticDeversity,
      double BitMutationRate,
      double CrossOverRate,
      Population::ReproductionTechniques PReproductionTechniques,
      Population::ParentSelectionTechnique ParentSelction,
      Population::DeletionTechnique Deletetion,
      Population::FitnessTechnique Fitness,
      Population::VariableLength Variable,
      int baseStates
      );
  
   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b) override;
};
#endif
