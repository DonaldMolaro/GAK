#ifndef __GA_Population_hh__
#define __GA_Population_hh__
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <random>
#include <utility>
#include <vector>

class BaseString;
class Chromosome;

class Population {
public:
   struct Options;
   enum OperationTechnique        { Minimize,Maximize };
   enum ReproductionTechniques    { DuplicatesAllowed, DuplicatesNotAllowed };
   enum ParrentSelectionTechnique { RouletteWheel,Random };
   enum DeletetionTechnique       { DeleteAll, DeleteAllButBest, DeleteHalf, DeleteQuarter, DeleteLast };
   enum FitnessTechnique          { FitnessIsEvaluation, WindowedFitness,LinearNormalizedFitness };
   enum VariableLength            { VariableLengthNotPermitted = 0,VariableLengthPermitted = 1};
   using ParentSelectionTechnique = ParrentSelectionTechnique;
   using DeletionTechnique = DeletetionTechnique;
   enum class OperationMode { Minimize, Maximize };
   enum class ReproductionMode { AllowDuplicates, DisallowDuplicates };
   enum class ParentSelectionMode { RouletteWheel, Random };
   enum class DeletionMode { DeleteAll, DeleteAllButBest, DeleteHalf, DeleteQuarter, DeleteLast };
   enum class FitnessMode { Evaluation, Windowed, LinearNormalized };
   enum class VariableLengthMode { Fixed, Variable };
   struct Configuration
   {
      OperationTechnique operation = Maximize;
      int numberOfIndividuals = 100;
      int numberOfTrials = 4000;
      int geneticDiversity = 44;
      double bitMutationRate = 0.008;
      double crossOverRate = 0.65;
      ReproductionTechniques reproduction = DuplicatesAllowed;
      ParrentSelectionTechnique parentSelection = RouletteWheel;
      DeletetionTechnique deletion = DeleteAll;
      FitnessTechnique fitness = FitnessIsEvaluation;
      VariableLength variableLength = VariableLengthNotPermitted;
      int baseStates = 2;

      Options toOptions() const;
   };
   struct Options
   {
      OperationMode operation = OperationMode::Maximize;
      int numberOfIndividuals = 100;
      int numberOfTrials = 4000;
      int geneticDiversity = 44;
      double bitMutationRate = 0.008;
      double crossOverRate = 0.65;
      ReproductionMode reproduction = ReproductionMode::AllowDuplicates;
      ParentSelectionMode parentSelection = ParentSelectionMode::RouletteWheel;
      DeletionMode deletion = DeletionMode::DeleteAll;
      FitnessMode fitness = FitnessMode::Evaluation;
      VariableLengthMode variableLength = VariableLengthMode::Fixed;
      int baseStates = 2;

      Configuration toConfiguration() const;
   };
   struct PopulationSummary
   {
      std::vector<double> mostFit;
      std::vector<double> leastFit;
   };
   struct RunResult
   {
      int generationsCompleted = 0;
      int evaluations = 0;
      std::vector<PopulationSummary> generationSummaries;
      PopulationSummary finalSummary;
   };
private:
   bool populationInitialized;
   std::vector<std::unique_ptr<Chromosome> > populationTable;
   std::vector<double> fitnessTable;
   std::vector<double> windowedFitnessTable;
   std::vector<double> linearNormalizedfitnessTable;
   Configuration config_;
   std::mt19937 randomGenerator;
   static const int kSummaryCount = 5;
   //
   // Private methods.
   //
   int initializePopulation();
   int insertNewPopulation(std::vector<std::unique_ptr<Chromosome> > replacementList,int numtoReplace);
   void evaluatePopulation();
   void sortPopulation();
   int replacementCount() const;
   int replacementSlotIndex(int offset) const;
   PopulationSummary buildPopulationSummary() const;
   RunResult executeInternal(bool captureGenerationSummaries, bool logProgress, bool logSummaries);
   void printPopulationSummary(const PopulationSummary& summary);
   void printFinalSummary(const RunResult& result);
   Chromosome *selectRandomParent(int *selected);
   bool containsChromosome(const Chromosome *candidate,
                           const std::vector<std::unique_ptr<Chromosome> >& population,
                           int populationLength) const;
   int randomIndex(int upperBoundExclusive);
   long randomBelow(long upperBoundExclusive);
   bool appendReplacement(std::vector<std::unique_ptr<Chromosome> >& replacementList,
			  Chromosome *candidate,
			  int& numberGenerated,
			  int numberToReplace,
			  bool allowDuplicates);
   double *selectFitnessWeights();
   Chromosome *selectParent(int *selected,double *rouletteTable);
   std::vector<std::unique_ptr<Chromosome> > breedPopulation(int numberToReplace);
   //
   //
   //
protected:
   virtual std::unique_ptr<Chromosome> createInitialChromosome();
   virtual std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
      mateChromosomes(Chromosome *mother, Chromosome *father);
   virtual void mutateChromosome(Chromosome *chromosome);
public:
   explicit Population(const Options& options);
   explicit Population(const Configuration& configuration);
   [[deprecated("Use Population::Options or Population::Configuration")]]
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
   const Configuration& configuration() const noexcept { return config_; }
   int decode(BaseString *b,int start,int end);
   [[nodiscard]] RunResult execute(bool captureGenerationSummaries = false);
   void run();
};
#endif
