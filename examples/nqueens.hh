#ifndef __GA_NQueens_hh__
#define __GA_NQueens_hh__

#include "population.hh"

class NQueens : public Population
{
public:
   explicit NQueens(const Population::Options& options);
   explicit NQueens(const Population::Configuration& configuration);
   [[deprecated("Use NQueens(const Population::Options&) or NQueens(const Population::Configuration&)")]]
   NQueens(
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
