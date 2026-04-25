#ifndef __GA_Spell_hh__
#define __GA_Spell_hh__
#include "population.hh"

class Spell : public Population
{
   double sqr ( double x );
public:
   explicit Spell(const Population::Options& options);
   explicit Spell(const Population::Configuration& configuration);
   [[deprecated("Use Spell(const Population::Options&) or Spell(const Population::Configuration&)")]]
   Spell(
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
