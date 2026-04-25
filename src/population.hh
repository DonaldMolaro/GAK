#pragma once

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
   class InitializationStrategy
   {
   public:
      virtual ~InitializationStrategy() = default;
      virtual std::unique_ptr<Chromosome> create(Population& population) = 0;
   };

   class MatingStrategy
   {
   public:
      virtual ~MatingStrategy() = default;
      virtual std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
         mate(Population& population, Chromosome& mother, Chromosome& father) = 0;
   };

   class MutationStrategy
   {
   public:
      virtual ~MutationStrategy() = default;
      virtual void mutate(Population& population, Chromosome& chromosome) = 0;
   };

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

   struct RunReportOptions
   {
      RunReportOptions(bool includeSettingsValue = false,
                       bool includeGenerationSummariesValue = false)
         : includeSettings(includeSettingsValue),
           includeGenerationSummaries(includeGenerationSummariesValue)
      {
      }

      bool includeSettings;
      bool includeGenerationSummaries;
   };

   class RunReporter
   {
   public:
      static void write(std::ostream& out,
                        const Population& population,
                        const RunResult& result,
                        const RunReportOptions& options = RunReportOptions());
   };
private:
   friend class RunReporter;
   bool populationInitialized;
   std::vector<std::unique_ptr<Chromosome> > populationTable;
   std::vector<double> fitnessTable;
   std::vector<double> windowedFitnessTable;
   std::vector<double> linearNormalizedfitnessTable;
   Settings settings_;
   std::unique_ptr<InitializationStrategy> initializationStrategy_;
   std::unique_ptr<MatingStrategy> matingStrategy_;
   std::unique_ptr<MutationStrategy> mutationStrategy_;
   std::mt19937 randomGenerator;
   unsigned int activeRandomSeed_;
   static const int kSummaryCount = 5;
protected:
   int initializePopulation();
   int insertNewPopulation(std::vector<std::unique_ptr<Chromosome> > replacementList,int numtoReplace);
   void evaluatePopulation();
   int randomIndex(int upperBoundExclusive);
   long randomBelow(long upperBoundExclusive);
   const std::vector<double>& selectFitnessWeights();
   int selectParent(const std::vector<double>& rouletteTable);
   std::vector<std::unique_ptr<Chromosome> > breedPopulation(int numberToReplace);
   bool containsChromosome(const Chromosome& candidate,
                           const std::vector<std::unique_ptr<Chromosome> >& population,
                           int populationLength) const;
   bool appendReplacement(std::vector<std::unique_ptr<Chromosome> >& replacementList,
			  std::unique_ptr<Chromosome> candidate,
			  int numberToReplace,
			  bool allowDuplicates);
   void setInitializationStrategy(std::unique_ptr<InitializationStrategy> strategy);
   void setMatingStrategy(std::unique_ptr<MatingStrategy> strategy);
   void setMutationStrategy(std::unique_ptr<MutationStrategy> strategy);
   std::unique_ptr<Chromosome> createInitialChromosome();
   std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
      mateChromosomes(Chromosome& mother, Chromosome& father);
   void mutateChromosome(Chromosome& chromosome);
   std::unique_ptr<Chromosome> createDefaultChromosome();
   std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
      mateDefaultChromosomes(Chromosome& mother, Chromosome& father);
   void mutateDefaultChromosome(Chromosome& chromosome);
   std::vector<std::unique_ptr<Chromosome> >& chromosomes() noexcept { return populationTable; }
   const std::vector<std::unique_ptr<Chromosome> >& chromosomes() const noexcept { return populationTable; }
   std::vector<double>& fitnessValues() noexcept { return fitnessTable; }
   const std::vector<double>& fitnessValues() const noexcept { return fitnessTable; }
   std::vector<double>& windowedFitnessValues() noexcept { return windowedFitnessTable; }
   const std::vector<double>& windowedFitnessValues() const noexcept { return windowedFitnessTable; }
   std::vector<double>& normalizedFitnessValues() noexcept { return linearNormalizedfitnessTable; }
   const std::vector<double>& normalizedFitnessValues() const noexcept { return linearNormalizedfitnessTable; }
   bool isPopulationInitialized() const noexcept { return populationInitialized; }
   void setPopulationInitialized(bool initialized) noexcept { populationInitialized = initialized; }
public:
   explicit Population(const Settings& settings);
   virtual ~Population();
   virtual double evaluateFitness(const BaseString& genes)=0;
   virtual void printCandidate(const BaseString& genes, std::ostream& out) const=0;
   const Settings& settings() const noexcept { return settings_; }
   unsigned int randomSeed() const noexcept { return activeRandomSeed_; }
   std::mt19937& randomEngine() noexcept { return randomGenerator; }
   void setRandomSeed(unsigned int seed);
   void setOperationMode(OperationMode mode) noexcept { settings_.operation = mode; }
   void setParentSelectionMode(ParentSelectionMode mode) noexcept { settings_.parentSelection = mode; }
   void setDeletionMode(DeletionMode mode) noexcept { settings_.deletion = mode; }
   void setFitnessMode(FitnessMode mode) noexcept { settings_.fitness = mode; }
   // Run the GA silently and return a structured summary.
   [[nodiscard]] RunResult execute(bool captureGenerationSummaries = false);
   void run(std::ostream& out, const RunReportOptions& options);
   // Compatibility wrapper around `execute()` that prints progress/summaries.
   void run();
private:
   void sortPopulation();
   int replacementCount() const;
   int replacementSlotIndex(int offset) const;
   PopulationSummary buildPopulationSummary() const;
   RunResult executeInternal(bool captureGenerationSummaries);
   int selectRandomParent();
};
