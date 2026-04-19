#ifndef __GA_F6_hh__
#define __GA_F6_hh__
#include "population.hh"

class F6 : public Population
{
   double sqr ( double x );
public:
  explicit F6(const Population::Options& options);
  explicit F6(const Population::Configuration& configuration);
  [[deprecated("Use F6(const Population::Options&) or F6(const Population::Configuration&)")]]
  F6(
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
  
   double FitnessFunction(BaseString *b) override;
   void FitnessPrint(BaseString *b) override;
};
#endif
