#ifndef __GA_Population_hh__
#define __GA_Population_hh__
class Population {
public:
   enum OperationTechnique        { Minimize,Maximize };
   enum ReproductionTechniques    { DuplicatesAllowed, DuplicatesNotAllowed };
   enum ParrentSelectionTechnique { RouletteWheel,Random };
   enum DeletetionTechnique       { DeleteAll, DeleteAllButBest, DeleteHalf, DeleteQuarter, DeleteLast };
   enum FitnessTechnique          { FitnessIsEvaluation, WindowedFitness,LinearNormalizedFitness };
   enum VariableLength            { VariableLengthNotPermitted = 0,VariableLengthPermitted = 1};
private:
   int populationInitialized;
   Chromosome **populationTable;
   double      *fitnessTable;
   double      *windowedFitnessTable;
   double      *linearNormalizedfitnessTable;

   int numberofIndividuals;
   int numberofTrials;
   int GenecticDeversity;
   double BitMutationRate;
   double CrossOverRate;
   OperationTechnique Operation;
   ReproductionTechniques Reproduction;
   ParrentSelectionTechnique ParentSelection;
   DeletetionTechnique Deletion;
   FitnessTechnique Fitness;
   VariableLength  Variable;
   int baseStates;
   //
   // Private methods.
   //
   int initializePopulation();
   int insertNewPopulation(Chromosome **replacementLList,int numtoReplace);
   void evaluatePopulation();
   void sortPopulation();
   bool findMatch(Chromosome *candidate,Chromosome **population, int tableLength);
   double     *selectFitnessTable();
   Chromosome *selectParrent(int *selected,double *rouletteTable);
   Chromosome **breedPopulation(int numberToReplace);
   //
   //
   //
public:
   Population(
      OperationTechnique POperation = Maximize,
      int PnumberofIndividuals = 100,
      int PnumberofTrials      = 4000,
      int PGenecticDeversity   = 44,
      double PBitMutationRate     = 0.008,
      double PCrossOverRate       = 0.65,
      ReproductionTechniques PReproductionTechniques = DuplicatesAllowed,
      ParrentSelectionTechnique ParentSelction = RouletteWheel,
      DeletetionTechnique Deletetion           = DeleteAll,
      FitnessTechnique Fitness                 = FitnessIsEvaluation,
      VariableLength PVariable                 = VariableLengthNotPermitted,
      int PbaseStates                          = 2);
   virtual ~Population();
   virtual double FitnessFunction(BaseString *b)=0;
   virtual void FitnessPrint(BaseString *b)=0;
   int decode(BaseString *b,int start,int end);
   void run();
};
#endif

