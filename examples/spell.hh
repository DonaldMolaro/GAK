class Spell : public Population
{
   double sqr ( double x );
public:
   Spell(
      Population::OperationTechnique Operation,
      int numberofIndividuals,
      int numberofTrials,
      int GenecticDeversity,
      double BitMutationRate,
      double CrossOverRate,
      Population::ReproductionTechniques PReproductionTechniques,
      Population::ParrentSelectionTechnique ParentSelction,
      Population::DeletetionTechnique Deletetion,
      Population::FitnessTechnique Fitness,
      Population::VariableLength Variable,
      int baseStates
      );
  
   virtual double FitnessFunction(BaseString *b);
   virtual void FitnessPrint(BaseString *b);
};
