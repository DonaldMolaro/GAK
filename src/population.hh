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
   struct Settings;
   struct GenerationReport;
   enum class OperationMode { Minimize, Maximize };
   enum class ReproductionMode { AllowDuplicates, DisallowDuplicates };
   enum class ParentSelectionMode { RouletteWheel, Random };
   enum class DeletionMode { DeleteAll, DeleteAllButBest, DeleteHalf, DeleteQuarter, DeleteLast };
   enum class FitnessMode { Evaluation, Windowed, LinearNormalized };
   enum class VariableLengthMode { Fixed, Variable };
   struct Settings
   {
      // Supported configuration surface for populations and examples.
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
   Settings settings_;
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
   void printSettingsSummary(std::ostream& out) const;
   void printGenerationProgress(std::ostream& out, const GenerationReport& report, bool printSummary);
   void reportRun(std::ostream& out, const RunResult& result, bool printGenerationSummaries);
   void printPopulationSummary(std::ostream& out, const PopulationSummary& summary);
   void printFinalSummary(std::ostream& out, const RunResult& result);
   int selectRandomParent();
   bool containsChromosome(const Chromosome& candidate,
                           const std::vector<std::unique_ptr<Chromosome> >& population,
                           int populationLength) const;
   int randomIndex(int upperBoundExclusive);
   long randomBelow(long upperBoundExclusive);
   bool appendReplacement(std::vector<std::unique_ptr<Chromosome> >& replacementList,
			  std::unique_ptr<Chromosome> candidate,
			  int numberToReplace,
			  bool allowDuplicates);
   const std::vector<double>& selectFitnessWeights();
   int selectParent(const std::vector<double>& rouletteTable);
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
      mateChromosomes(Chromosome& mother, Chromosome& father);
   // Apply a mutation step to a chromosome. Override this when mutations need
   // to preserve invariants such as fixed givens or permutation validity.
   virtual void mutateChromosome(Chromosome& chromosome);
public:
   explicit Population(const Settings& settings);
   virtual ~Population();
   virtual double evaluateFitness(const BaseString& genes)=0;
   virtual void printCandidate(const BaseString& genes, std::ostream& out)=0;
   const Settings& settings() const noexcept { return settings_; }
   unsigned int randomSeed() const noexcept { return activeRandomSeed_; }
   void setRandomSeed(unsigned int seed);
   int decode(const BaseString& genes,int start,int end) const;
   // Run the GA silently and return a structured summary.
   [[nodiscard]] RunResult execute(bool captureGenerationSummaries = false);
   // Compatibility wrapper around `execute()` that prints progress/summaries.
   void run();
};
#endif
