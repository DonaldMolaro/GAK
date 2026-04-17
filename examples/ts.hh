/*
 * Traveling salesman problem solved with a high order GA.
 * Donald Molaro - Copyright 1997.
 * Image Integration Inc.
 *
 * file            ts.hh
 *
 * Implements the traveling salesman class, with an inheritance to
 * the population object.
 */

class TravelingSalesman : public Population
{
   double sqr ( double x );
public:
   TravelingSalesman(
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
      int baseStates,
      int gridS = 0
      );
  
   virtual double FitnessFunction(BaseString *b);
   virtual void FitnessPrint(BaseString *b);
private:
   int NumCities;
   int GridSize;
   int *XCoord;
   int *YCoord;
};
