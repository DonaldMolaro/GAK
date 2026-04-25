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
   struct GenerationReport;
   enum class OperationMode { Minimize, Maximize };
   enum class ReproductionMode { AllowDuplicates, DisallowDuplicates };
   enum class ParentSelectionMode { RouletteWheel, Random };
   enum class DeletionMode { DeleteAll, DeleteAllButBest, DeleteHalf, DeleteQuarter, DeleteLast };
   enum class FitnessMode { Evaluation, Windowed, LinearNormalized };
   enum class VariableLengthMode { Fixed, Variable };
   struct Configuration
   {
      // Legacy-compatible configuration surface. New code should prefer
      // `Options`, but this remains useful for bridging older callers.
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
      bool useFixedRandomSeed = false;
      unsigned int randomSeed = 0;

      Options toOptions() const;
   };
   struct Options
   {
      // Preferred modern configuration surface for new code and examples.
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
      bool useFixedRandomSeed = false;
      unsigned int randomSeed = 0;

      Configuration toConfiguration() const;
   };
   struct PopulationSummary
   {
      // Snapshot of the strongest and weakest fitness values observed in a
      // generation or at the end of a run.
      std::vector<double> mostFit;
      std::vector<double> leastFit;
   };
   struct RunResult
   {
      // Structured execution result returned by `execute()`. This allows
      // callers to run the GA without coupling to the legacy stderr output in
      // `run()`.
      unsigned int randomSeed = 0;
      bool usedConfiguredSeed = false;
      int generationsCompleted = 0;
      int evaluations = 0;
      std::vector<GenerationReport> generationReports;
      std::vector<PopulationSummary> generationSummaries;
      PopulationSummary finalSummary;
   };
   struct GenerationReport
   {
      int generation = 0;
      int evaluations = 0;
      PopulationSummary summary;
   };
private:
   bool populationInitialized;
   std::vector<std::unique_ptr<Chromosome> > populationTable;
   std::vector<double> fitnessTable;
   std::vector<double> windowedFitnessTable;
   std::vector<double> linearNormalizedfitnessTable;
   Configuration config_;
   std::mt19937 randomGenerator;
   unsigned int activeRandomSeed_;
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
   RunResult executeInternal(bool captureGenerationSummaries);
   void printConfigurationSummary() const;
   void printGenerationProgress(const GenerationReport& report, bool printSummary);
   void reportRun(const RunResult& result, bool printGenerationSummaries);
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
   // Create an initial chromosome for the starting population. Subclasses can
   // override this to generate candidates that already satisfy key constraints.
   virtual std::unique_ptr<Chromosome> createInitialChromosome();
   // Produce two children from a pair of parents. Override this when the
   // default generic crossover should be replaced with problem-aware mating.
   virtual std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
      mateChromosomes(Chromosome *mother, Chromosome *father);
   // Apply a mutation step to a chromosome. Override this when mutations need
   // to preserve invariants such as fixed givens or permutation validity.
   virtual void mutateChromosome(Chromosome *chromosome);
public:
   // Construct a population from the preferred modern option set.
   explicit Population(const Options& options);
   // Construct a population from the legacy-compatible configuration object.
   explicit Population(const Configuration& configuration);
   virtual ~Population();
   // Subclasses provide the problem-specific score for a chromosome.
   virtual double FitnessFunction(const BaseString& b)=0;
   // Subclasses provide a human-readable rendering of a chromosome.
   virtual void FitnessPrint(const BaseString& b)=0;
   const Configuration& configuration() const noexcept { return config_; }
   unsigned int randomSeed() const noexcept { return activeRandomSeed_; }
   void setRandomSeed(unsigned int seed);
   // Decode a contiguous range of genes as a base-2 integer.
   int decode(const BaseString& b,int start,int end) const;
   // Run the GA silently and return a structured summary.
   [[nodiscard]] RunResult execute(bool captureGenerationSummaries = false);
   // Compatibility wrapper around `execute()` that prints progress/summaries.
   void run();
};
#endif
