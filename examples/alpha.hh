#ifndef __GA_Alpha_hh__
#define __GA_Alpha_hh__
#include "population.hh"

class Alpha : public Population
{
   double sqr ( double x );
public:
   explicit Alpha(const Population::Options& options);
   explicit Alpha(const Population::Configuration& configuration);
   [[deprecated("Use Alpha(const Population::Options&) or Alpha(const Population::Configuration&)")]]
   Alpha(
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
   int RandomAlgorithim();
};
#endif
