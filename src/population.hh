#ifndef __GA_Population_hh__
#define __GA_Population_hh__
#include <cstddef>
#include <memory>
#include <random>
#include <vector>

class Population {
public:
   enum OperationTechnique        { Minimize,Maximize };
   enum ReproductionTechniques    { DuplicatesAllowed, DuplicatesNotAllowed };
   enum ParrentSelectionTechnique { RouletteWheel,Random };
   enum DeletetionTechnique       { DeleteAll, DeleteAllButBest, DeleteHalf, DeleteQuarter, DeleteLast };
   enum FitnessTechnique          { FitnessIsEvaluation, WindowedFitness,LinearNormalizedFitness };
   enum VariableLength            { VariableLengthNotPermitted = 0,VariableLengthPermitted = 1};
private:
   bool populationInitialized;
   std::vector<std::unique_ptr<Chromosome> > populationTable;
   std::vector<double> fitnessTable;
   std::vector<double> windowedFitnessTable;
   std::vector<double> linearNormalizedfitnessTable;

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
   std::mt19937 randomGenerator;
   static const int kSummaryCount = 5;
   //
   // Private methods.
   //
   int initializePopulation();
   int insertNewPopulation(std::vector<std::unique_ptr<Chromosome> > replacementList,int numtoReplace);
   void evaluatePopulation();
   void sortPopulation();
   int numToReplaceForDeletion() const;
   int replacementIndex(int offset) const;
   void printPopulationSummary();
   Chromosome *selectRandomParent(int *selected);
   bool findMatch(const Chromosome *candidate,
		  const std::vector<std::unique_ptr<Chromosome> >& population,
		  int tableLength) const;
   int randomIndex(int upperBoundExclusive);
   long randomBelow(long upperBoundExclusive);
   bool appendReplacement(std::vector<std::unique_ptr<Chromosome> >& replacementList,
			  Chromosome *candidate,
			  int& numberGenerated,
			  int numberToReplace,
			  bool allowDuplicates);
   double     *selectFitnessTable();
   Chromosome *selectParrent(int *selected,double *rouletteTable);
   std::vector<std::unique_ptr<Chromosome> > breedPopulation(int numberToReplace);
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
   virtual ~Population() = default;
   virtual double FitnessFunction(BaseString *b)=0;
   virtual void FitnessPrint(BaseString *b)=0;
   int decode(BaseString *b,int start,int end);
   void run();
};
#endif
