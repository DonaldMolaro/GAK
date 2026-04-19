#ifndef __GA_LatinSquare_hh__
#define __GA_LatinSquare_hh__

#include "population.hh"

class LatinSquare : public Population
{
public:
   explicit LatinSquare(const Population::Options& options);
   explicit LatinSquare(const Population::Configuration& configuration);
   [[deprecated("Use LatinSquare(const Population::Options&) or LatinSquare(const Population::Configuration&)")]]
   LatinSquare(
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

private:
   int squareSize(const BaseString *b) const;
};

#endif
