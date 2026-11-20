#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "base.hh"
#include "chromosome.hh"
#include "except.hh"
#include "random.hh"

#include "population.hh"
#include "population_report.hh"

class PopulationTestRig
{
public:
  static int initializePopulation(Population& population)
  {
    return population.initializePopulation();
  }

  static int insertNewPopulation(Population& population,
                                 std::vector<std::unique_ptr<Chromosome> > replacements,
                                 int number_to_replace)
  {
    return population.insertNewPopulation(std::move(replacements), number_to_replace);
  }

  static void evaluatePopulation(Population& population)
  {
    population.evaluatePopulation();
  }

  static Population::PopulationSummary buildPopulationSummary(const Population& population)
  {
    return population.buildPopulationSummary();
  }

  static int randomIndex(Population& population, int upper_bound_exclusive)
  {
    return population.randomIndex(upper_bound_exclusive);
  }

  static long randomBelow(Population& population, long upper_bound_exclusive)
  {
    return population.randomBelow(upper_bound_exclusive);
  }

  static const std::vector<double>& selectFitnessWeights(Population& population)
  {
    return population.selectFitnessWeights();
  }

  static int selectTournamentParent(Population& population)
  {
    return population.selectTournamentParent();
  }

  static int selectParent(Population& population, const std::vector<double>& roulette_table)
  {
    return population.selectParent(roulette_table);
  }

  static std::vector<std::unique_ptr<Chromosome> > breedPopulation(Population& population, int number_to_replace)
  {
    return population.breedPopulation(number_to_replace);
  }

  static bool containsChromosome(const Population& population,
                                 const Chromosome& candidate,
                                 const std::vector<std::unique_ptr<Chromosome> >& population_list,
                                 int population_length)
  {
    return population.containsChromosome(candidate, population_list, population_length);
  }

  static bool appendReplacement(Population& population,
                                std::vector<std::unique_ptr<Chromosome> >& replacement_list,
                                std::unique_ptr<Chromosome> candidate,
                                int number_to_replace,
                                bool allow_duplicates)
  {
    return population.appendReplacement(replacement_list, std::move(candidate), number_to_replace, allow_duplicates);
  }

  static std::unique_ptr<Chromosome> createInitialChromosome(Population& population)
  {
    return population.createInitialChromosome();
  }

  static std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
  mateChromosomes(Population& population, Chromosome& mother, Chromosome& father)
  {
    return population.mateChromosomes(mother, father);
  }

  static void mutateChromosome(Population& population, Chromosome& chromosome)
  {
    population.mutateChromosome(chromosome);
  }

  static std::vector<std::unique_ptr<Chromosome> >& chromosomes(Population& population)
  {
    return population.populationTable;
  }

  static std::vector<double>& fitnessValues(Population& population)
  {
    return population.fitnessTable;
  }

  static std::vector<bool>& evaluatedFitness(Population& population)
  {
    return population.fitnessEvaluated;
  }

  static const std::vector<bool>& evaluatedFitness(const Population& population)
  {
    return population.fitnessEvaluated;
  }

  static const std::vector<double>& fitnessValues(const Population& population)
  {
    return population.fitnessTable;
  }

  static std::vector<double>& windowedFitnessValues(Population& population)
  {
    return population.windowedFitnessTable;
  }

  static const std::vector<double>& windowedFitnessValues(const Population& population)
  {
    return population.windowedFitnessTable;
  }

  static std::vector<double>& normalizedFitnessValues(Population& population)
  {
    return population.linearNormalizedfitnessTable;
  }

  static const std::vector<double>& normalizedFitnessValues(const Population& population)
  {
    return population.linearNormalizedfitnessTable;
  }

  static bool isPopulationInitialized(const Population& population)
  {
    return population.populationInitialized;
  }

  static void setPopulationInitialized(Population& population, bool initialized)
  {
    population.populationInitialized = initialized;
  }
};

namespace {

int g_failures = 0;

void expect_true(bool condition, const std::string& message)
{
  if (!condition)
    {
      std::cerr << "FAIL: " << message << std::endl;
      ++g_failures;
    }
}

template <class ExceptionType, class Fn>
void expect_throws(Fn fn, const std::string& message)
{
  try
    {
      fn();
      expect_true(false, message);
    }
  catch (const ExceptionType&)
    {
      expect_true(true, message);
    }
  catch (...)
    {
      expect_true(false, message + " (threw wrong exception type)");
    }
}

BaseString makeBinaryString(const std::string& bits)
{
  BaseString b(bits.length(), 2);
  for (int i = 0 ; i < (int)bits.length() ; i++)
    {
      if (bits[i] == '1') b.setValue(i);
      else b.clearValue(i);
    }
  return b;
}

Population::Settings make_population_options(Population::OperationMode operation,
					    int individuals,
					    int trials,
					    int chromosome_length,
					    double mutation,
					    double crossover,
					    Population::ReproductionMode reproduction,
					    Population::ParentSelectionMode selection,
					    Population::DeletionMode deletion,
					    Population::FitnessMode fitness,
					    Population::VariableLengthMode variable_length,
					    int states);

class InspectablePopulation : public PopulationProblem
{
public:
  explicit InspectablePopulation(const Population::Settings& settings)
    : population_(settings, *this)
  {
  }

  double evaluateFitness(const BaseString& b) override
  {
    return b.valueAt(0);
  }

  void printCandidate(const BaseString&, std::ostream&) const override
  {
  }

  operator Population&() { return population_; }
  operator const Population&() const { return population_; }
  const Population::Settings& settings() const { return population_.settings(); }
  Population::RunResult execute(bool captureGenerationSummaries = false) { return population_.execute(captureGenerationSummaries); }
  void run(std::ostream& out, const PopulationRunReportOptions& options) { population_.run(out, options); }
  void run() { population_.run(); }
  void setDeletionMode(Population::DeletionMode mode) { population_.setDeletionMode(mode); }
  void setFitnessMode(Population::FitnessMode mode) { population_.setFitnessMode(mode); }
  void setParentSelectionMode(Population::ParentSelectionMode mode) { population_.setParentSelectionMode(mode); }
  void setOperationMode(Population::OperationMode mode) { population_.setOperationMode(mode); }
  void setRandomSeed(unsigned int seed) { population_.setRandomSeed(seed); }

private:
  Population population_;
};

class DefaultHookPopulation : public PopulationProblem
{
public:
  explicit DefaultHookPopulation(const Population::Settings& settings)
    : population_(settings, *this)
  {
  }

  double evaluateFitness(const BaseString& b) override
  {
    return b.valueAt(0);
  }

  void printCandidate(const BaseString& b, std::ostream& out) const override
  {
    out << "Chromosome:" << b.valueAt(0) << '\n';
  }

  operator Population&() { return population_; }
  operator const Population&() const { return population_; }
  const Population::Settings& settings() const { return population_.settings(); }
  Population::RunResult execute(bool captureGenerationSummaries = false) { return population_.execute(captureGenerationSummaries); }
  void run(std::ostream& out, const PopulationRunReportOptions& options) { population_.run(out, options); }
  void run() { population_.run(); }

private:
  Population population_;
};

class StrategyHookPopulation : public PopulationProblem
{
public:
  explicit StrategyHookPopulation(const Population::Settings& settings)
    : population_(settings, *this)
  {
  }

  double evaluateFitness(const BaseString& b) override
  {
    return b.valueAt(0);
  }

  void printCandidate(const BaseString&, std::ostream&) const override
  {
  }

  std::unique_ptr<Chromosome> initializeCandidate(Population& population) override
  {
    BaseString genes(population.settings().chromosomeLength, population.settings().baseStates);
    for (int i = 0 ; i < genes.length() ; i++)
      {
        genes.setValue(i, 1);
      }
    return std::make_unique<Chromosome>(std::move(genes),
                                        population.settings().variableLength == Population::VariableLengthMode::Variable,
                                        population.settings().baseStates);
  }

  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
  mateCandidates(Population&, Chromosome& mother, Chromosome& father) override
  {
    BaseString first = father.genes();
    BaseString second = mother.genes();
    return std::make_pair(std::make_unique<Chromosome>(std::move(first)),
                          std::make_unique<Chromosome>(std::move(second)));
  }

  void mutateCandidate(Population&, Chromosome& chromosome) override
  {
    for (int i = 0 ; i < chromosome.length() ; i++)
      {
        chromosome.genes().clearValue(i);
      }
  }

  operator Population&() { return population_; }
  operator const Population&() const { return population_; }
  const Population::Settings& settings() const { return population_.settings(); }
  Population::RunResult execute(bool captureGenerationSummaries = false) { return population_.execute(captureGenerationSummaries); }
  void run(std::ostream& out, const PopulationRunReportOptions& options) { population_.run(out, options); }
  void run() { population_.run(); }

private:
  Population population_;
};

class ConstantProblem : public PopulationProblem
{
public:
  double evaluateFitness(const BaseString& genes) override
  {
    return genes.valueAt(0);
  }

  void printCandidate(const BaseString& genes, std::ostream& out) const override
  {
    out << "External:" << genes.valueAt(0) << '\n';
  }
};

class EarlyStopProblem : public PopulationProblem
{
public:
  double evaluateFitness(const BaseString& genes) override
  {
    return genes.valueAt(0);
  }

  void printCandidate(const BaseString&, std::ostream&) const override
  {
  }

  bool hasReachedSolution(const Population&, const BaseString&, double fitness) const override
  {
    return fitness >= 1.0;
  }

  std::unique_ptr<Chromosome> initializeCandidate(Population& population) override
  {
    BaseString genes(population.settings().chromosomeLength, population.settings().baseStates);
    genes.setValue(0, 1);
    return std::make_unique<Chromosome>(std::move(genes),
                                        population.settings().variableLength == Population::VariableLengthMode::Variable,
                                        population.settings().baseStates);
  }
};

Population::Settings make_population_options(Population::OperationMode operation,
					    int individuals,
					    int trials,
					    int chromosome_length,
					    double mutation,
					    double crossover,
					    Population::ReproductionMode reproduction,
					    Population::ParentSelectionMode selection,
					    Population::DeletionMode deletion,
					    Population::FitnessMode fitness,
					    Population::VariableLengthMode variable_length,
					    int states)
{
  Population::Settings options;
  options.operation = operation;
  options.numberOfIndividuals = individuals;
  options.numberOfTrials = trials;
  options.chromosomeLength = chromosome_length;
  options.bitMutationRate = mutation;
  options.crossOverRate = crossover;
  options.reproduction = reproduction;
  options.parentSelection = selection;
  options.deletion = deletion;
  options.fitness = fitness;
  options.variableLength = variable_length;
  options.baseStates = states;
  return options;
}

void test_base_string()
{
  BaseString binary(4, 2);
  binary.clearValue(0);
  binary.setValue(1);
  binary.clearValue(2);
  binary.setValue(3);

  expect_true(binary.length() == 4, "BaseString length should match constructor");
  expect_true(binary.states() == 2, "BaseString should report its state count");
  expect_true(binary.bitsPerValue() == 1, "Binary BaseString should use one bit per value");
  expect_true(binary.bitCount() == 4, "Binary BaseString should report its packed bit length");
  expect_true(binary.valueAt(0) == 0, "Binary base 0 should be cleared");
  expect_true(binary.valueAt(1) == 1, "Binary base 1 should be set");
  expect_true(binary.valueAt(2) == 0, "Binary base 2 should be cleared");
  expect_true(binary.valueAt(3) == 1, "Binary base 3 should be set");

  BaseString symbolic(3, 5);
  symbolic.setValue(0, 4);
  symbolic.setValue(1, 2);
  symbolic.clearValue(2);

  BaseString byte_aligned(4, 4);
  byte_aligned.setValue(0, 3);
  byte_aligned.setValue(1, 2);
  byte_aligned.setValue(2, 1);
  byte_aligned.clearValue(3);

  expect_true(symbolic.valueAt(0) == 4, "Symbolic base should preserve assigned values");
  expect_true(symbolic.valueAt(1) == 2, "Symbolic base should preserve non-binary states");
  expect_true(symbolic.valueAt(2) == 0, "Clear should reset symbolic base to zero");
  expect_true(byte_aligned.valueAt(0) == 3, "Byte-aligned symbolic bases should store values correctly");
  expect_true(byte_aligned.valueAt(3) == 0, "Byte-aligned clear should preserve zero values");

  symbolic.swapValues(0, 1);
  expect_true(symbolic.valueAt(0) == 2 && symbolic.valueAt(1) == 4,
              "swapValues should exchange symbolic gene values");
}

void test_population_options_round_trip()
{
  Population::Settings settings;
  settings.operation = Population::OperationMode::Minimize;
  settings.numberOfIndividuals = 17;
  settings.numberOfTrials = 99;
  settings.chromosomeLength = 23;
  settings.bitMutationRate = 0.125;
  settings.crossOverRate = 0.75;
  settings.reproduction = Population::ReproductionMode::DisallowDuplicates;
  settings.parentSelection = Population::ParentSelectionMode::Random;
  settings.deletion = Population::DeletionMode::DeleteQuarter;
  settings.fitness = Population::FitnessMode::LinearNormalized;
  settings.variableLength = Population::VariableLengthMode::Variable;
  settings.baseStates = 7;
  settings.useFixedRandomSeed = true;
  settings.randomSeed = 4242U;

  expect_true(settings.operation == Population::OperationMode::Minimize,
              "Settings should preserve operation");
  expect_true(settings.reproduction == Population::ReproductionMode::DisallowDuplicates,
              "Settings should preserve reproduction");
  expect_true(settings.parentSelection == Population::ParentSelectionMode::Random,
              "Settings should preserve parent selection");
  expect_true(settings.deletion == Population::DeletionMode::DeleteQuarter,
              "Settings should preserve deletion mode");
  expect_true(settings.fitness == Population::FitnessMode::LinearNormalized,
              "Settings should preserve fitness mode");
  expect_true(settings.variableLength == Population::VariableLengthMode::Variable,
              "Settings should preserve variable-length mode");
  expect_true(settings.numberOfIndividuals == 17,
              "Settings should preserve individual count");
  expect_true(settings.numberOfTrials == 99,
              "Settings should preserve trial count");
  expect_true(settings.chromosomeLength == 23,
              "Settings should preserve chromosome length");
  expect_true(settings.baseStates == 7,
              "Settings should preserve base state count");
  expect_true(settings.useFixedRandomSeed,
              "Settings should preserve fixed-seed mode");
  expect_true(settings.randomSeed == 4242U,
              "Settings should preserve the configured random seed");
}

void test_population_can_use_external_problem_object()
{
  ConstantProblem problem;
  Population pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
                                         Population::ReproductionMode::AllowDuplicates,
                                         Population::ParentSelectionMode::RouletteWheel,
                                         Population::DeletionMode::DeleteAll,
                                         Population::FitnessMode::Evaluation,
                                         Population::VariableLengthMode::Fixed, 2),
                 problem);

  Population::RunResult result = pop.execute(true);
  expect_true(result.evaluations >= pop.settings().numberOfIndividuals,
              "Population should be runnable with an external problem object");

  std::ostringstream out;
  pop.run(out, PopulationRunReportOptions{true, true});
  expect_true(out.str().find("External:") != std::string::npos,
              "External problem objects should drive candidate formatting");
}

void test_base_string_error_paths()
{
  expect_throws<GAFatalException>(
    []() {
      BaseString invalid(0, 2);
    },
    "Invalid BaseString constructor arguments should throw");
  expect_throws<GAFatalException>(
    []() {
      BaseString invalid_states(1, 1);
    },
    "Invalid BaseString state count should throw");

  BaseString binary(2, 2);
  expect_throws<GAFatalException>(
    [&binary]() { binary.valueAt(3); },
    "Out-of-range test should throw");
  expect_throws<GAFatalException>(
    [&binary]() { binary.setValue(3); },
    "Out-of-range set should throw");
  expect_throws<GAFatalException>(
    [&binary]() { binary.clearValue(3); },
    "Out-of-range clear should throw");

  expect_throws<GAFatalException>(
    [&binary]() { binary.testBitForTesting(-1); },
    "Out-of-range internal testBit should throw");
  expect_throws<GAFatalException>(
    [&binary]() { binary.setBitForTesting(-1); },
    "Out-of-range internal setBit should throw");
  expect_throws<GAFatalException>(
    [&binary]() { binary.clearBitForTesting(-1); },
    "Out-of-range internal clearBit should throw");
}

void test_base_string_print_helpers()
{
  BaseString binary(4, 2);
  binary.setValue(0);
  binary.clearValue(1);
  binary.setValue(2);
  binary.clearValue(3);

  std::ostringstream bits_out;
  binary.printBits(bits_out);
  expect_true(bits_out.str() == "1010", "printBits should render the packed bit string");
  expect_true(binary.bitString() == "1010", "bitString should return the packed bit representation");

  BaseString symbolic(3, 3);
  symbolic.setValue(0, 0);
  symbolic.setValue(1, 1);
  symbolic.setValue(2, 2);
  std::vector<std::string> values;
  values.push_back("a");
  values.push_back("b");
  values.push_back("c");
  std::ostringstream text_out;
  symbolic.printSymbols(values, text_out);
  expect_true(text_out.str() == "abc", "printSymbols should render symbolic values");

  values.pop_back();
  expect_throws<GAFatalException>(
    [&symbolic, &values, &text_out]() { symbolic.printSymbols(values, text_out); },
    "printSymbols should reject incomplete symbol tables");
}

void test_exception_helpers()
{
  GAFatalException fatal(__FILE__, __LINE__, "fatal-reason");
  std::ostringstream out;

  out << fatal.what();
  fatal << out;

  expect_true(std::string(fatal.what()) == "fatal-reason", "Fatal exception should preserve its reason");
  expect_true(out.str().find("fatal-reason") != std::string::npos, "Exception stream operator should print the reason");
}

void test_mutation_rate_zero_preserves_chromosome()
{
  std::mt19937 random_generator(1);
  Chromosome chromosome(makeBinaryString("10101100"));
  Chromosome original(makeBinaryString("10101100"));

  chromosome.mutate(0.0, &random_generator);

  expect_true(chromosome.equals(original), "Mutation rate 0 should leave chromosome unchanged");
}

void test_chromosome_constructor_and_compare_paths()
{
  std::mt19937 random_generator(2);

  expect_throws<GAFatalException>(
    []() {
      Chromosome too_long(2049, 0, 2);
    },
    "Overlong chromosome constructor should throw");

  Chromosome left(makeBinaryString("1010"));
  Chromosome right(makeBinaryString("0101"));
  Chromosome symbolic(5, 0, 4, &random_generator);
  expect_true(!left.equals(right), "equals should return false for different chromosomes");
  expect_true(!left.equals(symbolic), "equals should return false for mismatched lengths");
  for (int i = 0 ; i < symbolic.length() ; i++)
    {
      int value = symbolic.genes().valueAt(i);
      expect_true(value >= 0 && value < 4, "Non-binary constructor should initialize values in range");
    }

  std::ostringstream out;
  left.print(out);
  expect_true(!out.str().empty(), "print should write chromosome contents");
}

void test_invalid_mutation_probability_throws()
{
  std::mt19937 random_generator(3);
  Chromosome chromosome(makeBinaryString("10101100"));

  expect_throws<GAFatalException>(
    [&chromosome, &random_generator]() { chromosome.mutate(2.0, &random_generator); },
    "Impossible mutation probability should throw a fatal exception");
}

void test_non_binary_mutation_with_probability_one_stays_in_range()
{
  std::mt19937 random_generator(4);
  Chromosome chromosome(BaseString(5, 4), 0, 4);
  chromosome.mutate(1.0, &random_generator);

  for (int i = 0 ; i < chromosome.length() ; i++)
    {
      int value = chromosome.genes().valueAt(i);
      expect_true(value >= 0 && value < 4, "Non-binary mutation should keep values within range");
    }
}

void test_binary_mutation_with_probability_one_changes_only_bits()
{
  std::mt19937 random_generator(5);
  Chromosome chromosome(makeBinaryString("000000"));
  chromosome.mutate(1.0, &random_generator);

  for (int i = 0 ; i < chromosome.length() ; i++)
    {
      int value = chromosome.genes().valueAt(i);
      expect_true(value == 0 || value == 1, "Binary mutation should keep values binary");
    }
}

void test_mate_without_crossover_clones_parents()
{
  std::mt19937 random_generator(6);
  Chromosome mother(makeBinaryString("11110000"));
  Chromosome father(makeBinaryString("00001111"));
  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
    mother.mate(father, 0.0, Chromosome::CrossoverType::SinglePoint, &random_generator);

  expect_true(children.first.get() != 0, "Mate should create a son");
  expect_true(children.second.get() != 0, "Mate should create a daughter");
  expect_true(children.first->equals(father), "Son should clone father when crossover rate is zero");
  expect_true(children.second->equals(mother), "Daughter should clone mother when crossover rate is zero");
}

void test_uniform_crossover_is_reachable_and_safe()
{
  std::mt19937 random_generator(7);
  Chromosome mother(makeBinaryString("11110000"));
  Chromosome father(makeBinaryString("00001111"));

  for (int i = 0 ; i < 20 ; i++)
    {
      std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
        mother.mate(father, 1.0, Chromosome::CrossoverType::Uniform, &random_generator);

      expect_true(children.first.get() != 0, "Uniform crossover should produce a son");
      expect_true(children.second.get() != 0, "Uniform crossover should produce a daughter");
      expect_true(children.first->length() == father.length(),
		  "Uniform crossover should preserve fixed-length son length");
      expect_true(children.second->length() == mother.length(),
		  "Uniform crossover should preserve fixed-length daughter length");
    }
}

void test_two_point_crossover_is_reachable_and_safe()
{
  std::mt19937 random_generator(8);
  Chromosome mother(makeBinaryString("11110000"));
  Chromosome father(makeBinaryString("00001111"));

  for (int i = 0 ; i < 20 ; i++)
    {
      std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
        mother.mate(father, 1.0, Chromosome::CrossoverType::TwoPoint, &random_generator);

      expect_true(children.first.get() != 0, "Two-point crossover should produce a son");
      expect_true(children.second.get() != 0, "Two-point crossover should produce a daughter");
      expect_true(children.first->length() == father.length(),
		  "Fixed-length two-point crossover should preserve son length");
      expect_true(children.second->length() == mother.length(),
		  "Fixed-length two-point crossover should preserve daughter length");
    }
}

void test_mate_rejects_mismatched_fixed_lengths()
{
  std::mt19937 random_generator(9);
  Chromosome mother(makeBinaryString("1111"));
  Chromosome father(makeBinaryString("00001111"));

  expect_throws<GAFatalException>(
    [&mother, &father, &random_generator]() {
      mother.mate(father, 1.0, Chromosome::CrossoverType::SinglePoint, &random_generator);
    },
    "Fixed-length mating should reject mismatched parent lengths");
}

void test_variable_length_mating_supports_different_lengths()
{
  std::mt19937 random_generator(10);
  Chromosome mother(makeBinaryString("1111"), 1, 2);
  Chromosome father(makeBinaryString("000011"), 1, 2);

  for (int i = 0 ; i < 20 ; i++)
    {
      std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
        mother.mate(father, 1.0, Chromosome::CrossoverType::SinglePoint, &random_generator);

      expect_true(children.first.get() != 0, "Variable-length mating should produce a son");
      expect_true(children.second.get() != 0, "Variable-length mating should produce a daughter");
      expect_true(children.first->length() >= 1, "Variable-length son should have a valid length");
      expect_true(children.second->length() >= 1, "Variable-length daughter should have a valid length");
    }
}

void test_variable_length_uniform_crossover()
{
  std::mt19937 random_generator(11);
  Chromosome mother(makeBinaryString("1111"), 1, 2);
  Chromosome father(makeBinaryString("000011"), 1, 2);

  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
    mother.mate(father, 1.0, Chromosome::CrossoverType::Uniform, &random_generator);

  expect_true(children.first.get() != 0 && children.second.get() != 0, "Variable-length uniform crossover should produce children");
  expect_true(children.first->length() == father.length(),
	      "Variable-length uniform crossover should preserve son length");
  expect_true(children.second->length() == mother.length(),
	      "Variable-length uniform crossover should preserve daughter length");
}

void test_variable_length_uniform_crossover_with_longer_mother()
{
  std::mt19937 random_generator(13);
  Chromosome mother(makeBinaryString("111100"), 1, 2);
  Chromosome father(makeBinaryString("0000"), 1, 2);

  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
    mother.mate(father, 1.0, Chromosome::CrossoverType::Uniform, &random_generator);

  expect_true(children.first.get() != 0 && children.second.get() != 0, "Variable-length uniform crossover should handle longer mothers");
  expect_true(children.first->length() == father.length(),
	      "Longer-mother uniform crossover should preserve son length");
  expect_true(children.second->length() == mother.length(),
	      "Longer-mother uniform crossover should preserve daughter length");
}

void test_invalid_crossover_type_throws()
{
  std::mt19937 random_generator(12);
  Chromosome mother(makeBinaryString("1111"));
  Chromosome father(makeBinaryString("0000"));

  expect_throws<GAFatalException>(
    [&mother, &father, &random_generator]() {
      mother.mate(father, 1.0, static_cast<Chromosome::CrossoverType>(99), &random_generator);
    },
    "Unsupported crossover type should throw");
}

void test_base_string_decode_bits()
{
  BaseString bits(4, 2);
  bits.setValue(0);
  bits.clearValue(1);
  bits.setValue(2);
  bits.setValue(3);

  expect_true(bits.decodeBits(0, 4) == 11, "decodeBits should interpret bits in big-endian order");
  expect_throws<GAFatalException>(
    [&bits]() { bits.decodeBits(-1, 1); },
    "decodeBits should reject a negative start");
  expect_throws<GAFatalException>(
    [&bits]() { bits.decodeBits(0, 5); },
    "decodeBits should reject a range that exceeds the packed bit count");
}

void test_population_random_helpers()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 4, 4, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));

  for (int i = 0 ; i < 20 ; i++)
    {
      int index = PopulationTestRig::randomIndex(pop, 3);
      expect_true(index >= 0 && index < 3, "randomIndex should stay within bounds");
    }

  for (int i = 0 ; i < 20 ; i++)
    {
      long value = PopulationTestRig::randomBelow(pop, 7);
      expect_true(value >= 0 && value < 7, "randomBelow should stay within bounds");
    }

  expect_throws<GAFatalException>(
    [&pop]() { PopulationTestRig::randomIndex(pop, 0); },
    "randomIndex should reject non-positive bounds");
  expect_throws<GAFatalException>(
    [&pop]() { PopulationTestRig::randomBelow(pop, 0); },
    "randomBelow should reject non-positive bounds");
}

void test_population_zero_total_selection_falls_back_to_uniform()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  PopulationTestRig::initializePopulation(pop);
  std::vector<double> roulette(2, 0.0);
  const int selected = PopulationTestRig::selectParent(pop, roulette);

  expect_true(selected >= 0 && selected < pop.settings().numberOfIndividuals,
	      "Uniform fallback selection should still return an in-range index");
}

void test_windowed_fitness_is_positive_for_maximize()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 3, 3, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Windowed,
						    Population::VariableLengthMode::Fixed, 2));

  PopulationTestRig::initializePopulation(pop);
  for (int i = 0 ; i < 3 ; i++)
    {
      PopulationTestRig::chromosomes(pop)[i] = std::make_unique<Chromosome>(makeBinaryString(i == 0 ? "0" : "1"));
      PopulationTestRig::fitnessValues(pop)[i] = 0.0;
      PopulationTestRig::evaluatedFitness(pop)[i] = false;
    }
  PopulationTestRig::setPopulationInitialized(pop, true);
  PopulationTestRig::evaluatePopulation(pop);

  expect_true(PopulationTestRig::windowedFitnessValues(pop)[0] > 0.0, "Windowed fitness should stay positive");
  expect_true(PopulationTestRig::windowedFitnessValues(pop)[1] > 0.0, "Windowed fitness should stay positive for middle members");
  expect_true(PopulationTestRig::windowedFitnessValues(pop)[2] > PopulationTestRig::windowedFitnessValues(pop)[0],
	      "Best maximizing member should receive more windowed fitness than the worst");
}

void test_windowed_fitness_is_positive_for_minimize()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Minimize, 3, 3, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Windowed,
						    Population::VariableLengthMode::Fixed, 2));

  PopulationTestRig::initializePopulation(pop);
  for (int i = 0 ; i < 3 ; i++)
    {
      PopulationTestRig::chromosomes(pop)[i] = std::make_unique<Chromosome>(makeBinaryString(i == 0 ? "0" : "1"));
      PopulationTestRig::fitnessValues(pop)[i] = 0.0;
      PopulationTestRig::evaluatedFitness(pop)[i] = false;
    }
  PopulationTestRig::setPopulationInitialized(pop, true);
  PopulationTestRig::evaluatePopulation(pop);

  expect_true(PopulationTestRig::windowedFitnessValues(pop)[0] > 0.0, "Minimizing windowed fitness should stay positive");
  expect_true(PopulationTestRig::windowedFitnessValues(pop)[0] > PopulationTestRig::windowedFitnessValues(pop)[2],
	      "Best minimizing member should receive more windowed fitness than the worst");
}

void test_fitness_table_selection()
{
  InspectablePopulation evaluation(make_population_options(Population::OperationMode::Maximize, 3, 3, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  InspectablePopulation windowed(make_population_options(Population::OperationMode::Maximize, 3, 3, 1, 0.0, 0.0,
							 Population::ReproductionMode::AllowDuplicates,
							 Population::ParentSelectionMode::RouletteWheel,
							 Population::DeletionMode::DeleteAll,
							 Population::FitnessMode::Windowed,
							 Population::VariableLengthMode::Fixed, 2));
  InspectablePopulation normalized(make_population_options(Population::OperationMode::Maximize, 3, 3, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::LinearNormalized,
							   Population::VariableLengthMode::Fixed, 2));

  expect_true(&PopulationTestRig::selectFitnessWeights(evaluation) == &PopulationTestRig::fitnessValues(evaluation),
	      "FitnessIsEvaluation should select the raw fitness table");
  expect_true(&PopulationTestRig::selectFitnessWeights(windowed) == &PopulationTestRig::windowedFitnessValues(windowed),
	      "WindowedFitness should select the windowed fitness table");
  expect_true(&PopulationTestRig::selectFitnessWeights(normalized) == &PopulationTestRig::normalizedFitnessValues(normalized),
	      "LinearNormalizedFitness should select the normalized fitness table");
}

void prepare_population(InspectablePopulation& pop, const std::string& first_bits,
			const std::string& second_bits)
{
  PopulationTestRig::initializePopulation(pop);
  PopulationTestRig::chromosomes(pop)[0] = std::make_unique<Chromosome>(makeBinaryString(first_bits));
  PopulationTestRig::chromosomes(pop)[1] = std::make_unique<Chromosome>(makeBinaryString(second_bits));
  for (int i = 0 ; i < pop.settings().numberOfIndividuals ; i++)
    {
      PopulationTestRig::fitnessValues(pop)[i] = 0.0;
      PopulationTestRig::evaluatedFitness(pop)[i] = false;
    }
  PopulationTestRig::setPopulationInitialized(pop, true);
  PopulationTestRig::evaluatePopulation(pop);
}

class NegativeFitnessProblem : public PopulationProblem
{
public:
  double evaluateFitness(const BaseString& genes) override
  {
    return genes.valueAt(0) == 0 ? -1.0 : -2.0;
  }

  void printCandidate(const BaseString&, std::ostream&) const override
  {
  }
};

void test_negative_fitness_values_are_not_treated_as_unevaluated()
{
  NegativeFitnessProblem problem;
  Population pop(make_population_options(Population::OperationMode::Minimize, 2, 4, 1, 0.0, 0.0,
                                         Population::ReproductionMode::AllowDuplicates,
                                         Population::ParentSelectionMode::RouletteWheel,
                                         Population::DeletionMode::DeleteAll,
                                         Population::FitnessMode::Evaluation,
                                         Population::VariableLengthMode::Fixed, 2),
                 problem);

  PopulationTestRig::initializePopulation(pop);
  PopulationTestRig::chromosomes(pop)[0] = std::make_unique<Chromosome>(makeBinaryString("0"));
  PopulationTestRig::chromosomes(pop)[1] = std::make_unique<Chromosome>(makeBinaryString("1"));
  PopulationTestRig::evaluatePopulation(pop);

  expect_true(PopulationTestRig::evaluatedFitness(pop)[0], "Negative fitness entries should be marked evaluated");
  expect_true(PopulationTestRig::evaluatedFitness(pop)[1], "All fitness entries should be marked evaluated");

  const std::vector<double>& fitness = PopulationTestRig::fitnessValues(pop);
  expect_true(fitness[0] == -2.0, "Minimize populations should preserve legitimate negative fitness values");
  expect_true(fitness[1] == -1.0, "A fitness of -1.0 should remain a valid evaluated result");
}

void test_population_selection_and_replacement_branches()
{
  InspectablePopulation min_pop(make_population_options(Population::OperationMode::Minimize, 2, 2, 4, 0.0, 0.0,
							Population::ReproductionMode::AllowDuplicates,
							Population::ParentSelectionMode::RouletteWheel,
							Population::DeletionMode::DeleteAll,
							Population::FitnessMode::Evaluation,
							Population::VariableLengthMode::Fixed, 2));
  prepare_population(min_pop, "1111", "0000");
  int selected = PopulationTestRig::selectParent(min_pop, PopulationTestRig::selectFitnessWeights(min_pop));
  Chromosome *chosen = PopulationTestRig::chromosomes(min_pop)[selected].get();
  expect_true(chosen != 0, "Minimize roulette selection should return a chromosome");
  expect_true(selected >= 0 && selected < min_pop.settings().numberOfIndividuals,
	      "Minimize roulette selection should produce an in-range index");

  InspectablePopulation dup_not_allowed(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
								Population::ReproductionMode::DisallowDuplicates,
								Population::ParentSelectionMode::RouletteWheel,
								Population::DeletionMode::DeleteAll,
								Population::FitnessMode::Evaluation,
								Population::VariableLengthMode::Fixed, 2));
  prepare_population(dup_not_allowed, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > bred = PopulationTestRig::breedPopulation(dup_not_allowed, 1);
  expect_true(bred.size() == 1, "DuplicatesNotAllowed breeding should still generate requested children");
  expect_true(PopulationTestRig::containsChromosome(dup_not_allowed, *bred[0], bred, 1),
	      "containsChromosome should report a present chromosome");

  std::vector<std::unique_ptr<Chromosome> > replacements_max;
  replacements_max.push_back(std::make_unique<Chromosome>(makeBinaryString("1111")));
  replacements_max.push_back(std::make_unique<Chromosome>(makeBinaryString("0000")));
  int replaced_max = PopulationTestRig::insertNewPopulation(dup_not_allowed, std::move(replacements_max), 2);
  expect_true(replaced_max >= 0, "insertNewPopulation should handle duplicates-not-allowed maximize path");

  InspectablePopulation dup_not_allowed_min(make_population_options(Population::OperationMode::Minimize, 2, 2, 4, 0.0, 0.0,
								    Population::ReproductionMode::DisallowDuplicates,
								    Population::ParentSelectionMode::RouletteWheel,
								    Population::DeletionMode::DeleteAll,
								    Population::FitnessMode::Evaluation,
								    Population::VariableLengthMode::Fixed, 2));
  prepare_population(dup_not_allowed_min, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > replacements_min;
  replacements_min.push_back(std::make_unique<Chromosome>(makeBinaryString("1111")));
  replacements_min.push_back(std::make_unique<Chromosome>(makeBinaryString("0000")));
  int replaced_min = PopulationTestRig::insertNewPopulation(dup_not_allowed_min, std::move(replacements_min), 2);
  expect_true(replaced_min >= 0, "insertNewPopulation should handle duplicates-not-allowed minimize path");

  InspectablePopulation dup_not_allowed_new(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
								    Population::ReproductionMode::DisallowDuplicates,
								    Population::ParentSelectionMode::RouletteWheel,
								    Population::DeletionMode::DeleteAll,
								    Population::FitnessMode::Evaluation,
								    Population::VariableLengthMode::Fixed, 2));
  prepare_population(dup_not_allowed_new, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > replacements_new;
  replacements_new.push_back(std::make_unique<Chromosome>(makeBinaryString("0011")));
  replacements_new.push_back(std::make_unique<Chromosome>(makeBinaryString("1100")));
  int replaced_new = PopulationTestRig::insertNewPopulation(dup_not_allowed_new, std::move(replacements_new), 2);
  expect_true(replaced_new > 0, "Duplicates-not-allowed maximize path should insert new chromosomes");

  InspectablePopulation dup_not_allowed_new_min(make_population_options(Population::OperationMode::Minimize, 2, 2, 4, 0.0, 0.0,
										Population::ReproductionMode::DisallowDuplicates,
										Population::ParentSelectionMode::RouletteWheel,
										Population::DeletionMode::DeleteAll,
										Population::FitnessMode::Evaluation,
										Population::VariableLengthMode::Fixed, 2));
  prepare_population(dup_not_allowed_new_min, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > replacements_new_min;
  replacements_new_min.push_back(std::make_unique<Chromosome>(makeBinaryString("0011")));
  replacements_new_min.push_back(std::make_unique<Chromosome>(makeBinaryString("1100")));
  int replaced_new_min = PopulationTestRig::insertNewPopulation(dup_not_allowed_new_min, std::move(replacements_new_min), 2);
  expect_true(replaced_new_min > 0, "Duplicates-not-allowed minimize path should insert new chromosomes");

}

void test_population_append_replacement_helper()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
						    Population::ReproductionMode::DisallowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));

  std::vector<std::unique_ptr<Chromosome> > replacement_list;
  expect_true(PopulationTestRig::appendReplacement(pop, replacement_list,
				    std::make_unique<Chromosome>(makeBinaryString("1111")),
				    2, false),
	      "appendReplacement should keep the first unique child");
  expect_true(!PopulationTestRig::appendReplacement(pop, replacement_list,
				     std::make_unique<Chromosome>(makeBinaryString("1111")),
				     2, false),
	      "appendReplacement should reject duplicate unique-only children");
  expect_true(PopulationTestRig::appendReplacement(pop, replacement_list,
				    std::make_unique<Chromosome>(makeBinaryString("0000")),
				    2, false),
	      "appendReplacement should keep a second unique child");
  expect_true(!PopulationTestRig::appendReplacement(pop, replacement_list,
				     std::make_unique<Chromosome>(makeBinaryString("0011")),
				     2, true),
	      "appendReplacement should reject overflow children when the replacement list is full");
}

void test_population_insert_new_population_rejects_overflow()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  prepare_population(pop, "1111", "0000");

  std::vector<std::unique_ptr<Chromosome> > replacements;
  replacements.push_back(std::make_unique<Chromosome>(makeBinaryString("0011")));
  replacements.push_back(std::make_unique<Chromosome>(makeBinaryString("1100")));
  replacements.push_back(std::make_unique<Chromosome>(makeBinaryString("0101")));

  expect_throws<GAFatalException>(
    [&pop, &replacements]() mutable {
      PopulationTestRig::insertNewPopulation(pop, std::move(replacements), 3);
    },
    "insertNewPopulation should reject replacement counts larger than the population");
}

void test_population_selection_guard_and_random_fitness_modes()
{
  InspectablePopulation fractional(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  PopulationTestRig::initializePopulation(fractional);
  std::vector<double> roulette(2, 0.4);
  int fractional_selected = PopulationTestRig::selectParent(fractional, roulette);
  expect_true(fractional_selected >= 0 && fractional_selected < fractional.settings().numberOfIndividuals,
              "Roulette selection should handle fractional weights without producing an invalid index");

  std::vector<double> tiny_roulette(2, 1e-16);
  int tiny_selected = PopulationTestRig::selectParent(fractional, tiny_roulette);
  expect_true(tiny_selected >= 0 && tiny_selected < fractional.settings().numberOfIndividuals,
              "Roulette selection should fall back safely for near-zero weights");

  InspectablePopulation windowed(make_population_options(Population::OperationMode::Maximize, 4, 4, 1, 0.0, 0.0,
							 Population::ReproductionMode::AllowDuplicates,
							 Population::ParentSelectionMode::RouletteWheel,
							 Population::DeletionMode::DeleteAll,
							 Population::FitnessMode::Windowed,
							 Population::VariableLengthMode::Fixed, 2));
  prepare_population(windowed, "1", "0");
  int selected = PopulationTestRig::selectParent(windowed, PopulationTestRig::selectFitnessWeights(windowed));
  expect_true(selected >= 0 && selected < windowed.settings().numberOfIndividuals,
	      "Windowed fitness selection should return an in-range parent index");

  InspectablePopulation normalized(make_population_options(Population::OperationMode::Maximize, 4, 4, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::LinearNormalized,
							   Population::VariableLengthMode::Fixed, 2));
  prepare_population(normalized, "1", "0");
  selected = PopulationTestRig::selectParent(normalized, PopulationTestRig::selectFitnessWeights(normalized));
  expect_true(selected >= 0 && selected < normalized.settings().numberOfIndividuals,
	      "Linear normalized fitness selection should return an in-range parent index");

  InspectablePopulation random_selection(make_population_options(Population::OperationMode::Maximize, 2, 4, 4, 0.0, 0.0,
								 Population::ReproductionMode::AllowDuplicates,
								 Population::ParentSelectionMode::Random,
								 Population::DeletionMode::DeleteAll,
								 Population::FitnessMode::Evaluation,
								 Population::VariableLengthMode::Fixed, 2));
  prepare_population(random_selection, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > random_bred = PopulationTestRig::breedPopulation(random_selection, 2);
  expect_true(random_bred.size() == 2,
	      "Random parent selection should generate the requested number of children");
}

void test_population_invalid_enum_paths()
{
  InspectablePopulation bad_delete(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  bad_delete.setDeletionMode(static_cast<Population::DeletionMode>(99));
  expect_throws<GAFatalException>(
    [&bad_delete]() { bad_delete.run(); },
    "Unsupported deletion technique should throw");

  InspectablePopulation bad_fitness(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
							    Population::ReproductionMode::AllowDuplicates,
							    Population::ParentSelectionMode::RouletteWheel,
							    Population::DeletionMode::DeleteAll,
							    Population::FitnessMode::Evaluation,
							    Population::VariableLengthMode::Fixed, 2));
  bad_fitness.setFitnessMode(static_cast<Population::FitnessMode>(99));
  expect_throws<GAFatalException>(
    [&bad_fitness]() { PopulationTestRig::selectFitnessWeights(bad_fitness); },
    "Unsupported fitness technique should throw");

  InspectablePopulation bad_parent(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  bad_parent.setParentSelectionMode(static_cast<Population::ParentSelectionMode>(99));
  prepare_population(bad_parent, "1111", "0000");
  expect_throws<GAFatalException>(
    [&bad_parent]() { PopulationTestRig::breedPopulation(bad_parent, 1); },
    "Unsupported parent selection technique should throw");

  InspectablePopulation bad_operation(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
							      Population::ReproductionMode::AllowDuplicates,
							      Population::ParentSelectionMode::RouletteWheel,
							      Population::DeletionMode::DeleteAll,
							      Population::FitnessMode::Evaluation,
							      Population::VariableLengthMode::Fixed, 2));
  prepare_population(bad_operation, "1111", "0000");
  bad_operation.setOperationMode(static_cast<Population::OperationMode>(99));
  expect_throws<GAFatalException>(
    [&bad_operation]() {
      PopulationTestRig::selectParent(bad_operation, PopulationTestRig::selectFitnessWeights(bad_operation));
    },
    "Unsupported operation technique in selection should throw");

  std::vector<std::unique_ptr<Chromosome> > replacements;
  replacements.push_back(std::make_unique<Chromosome>(makeBinaryString("1111")));
  expect_throws<GAFatalException>(
    [&bad_operation, &replacements]() mutable {
      PopulationTestRig::insertNewPopulation(bad_operation, std::move(replacements), 1);
    },
    "Unsupported operation technique in replacement should throw");

  InspectablePopulation bad_operation_dup(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
								  Population::ReproductionMode::DisallowDuplicates,
								  Population::ParentSelectionMode::RouletteWheel,
								  Population::DeletionMode::DeleteAll,
								  Population::FitnessMode::Evaluation,
								  Population::VariableLengthMode::Fixed, 2));
  prepare_population(bad_operation_dup, "1111", "0000");
  bad_operation_dup.setOperationMode(static_cast<Population::OperationMode>(99));
  std::vector<std::unique_ptr<Chromosome> > dup_replacements;
  dup_replacements.push_back(std::make_unique<Chromosome>(makeBinaryString("0011")));
  expect_throws<GAFatalException>(
    [&bad_operation_dup, &dup_replacements]() mutable {
      PopulationTestRig::insertNewPopulation(bad_operation_dup, std::move(dup_replacements), 1);
    },
    "Unsupported operation technique in duplicate-filtered replacement should throw");
}

void test_population_verbose_path()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  std::ostringstream out;
  pop.run(out, PopulationRunReportOptions{true, true});
  expect_true(PopulationTestRig::isPopulationInitialized(pop), "Verbose run path should still complete successfully");
}

void test_population_verbose_minimize_path()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Minimize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  std::ostringstream out;
  pop.run(out, PopulationRunReportOptions{true, true});
  expect_true(PopulationTestRig::isPopulationInitialized(pop), "Verbose minimize path should still complete successfully");
}

void test_population_run_modes()
{
  InspectablePopulation delete_all(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  std::ostringstream out;
  delete_all.run(out, PopulationRunReportOptions{false, false});

  InspectablePopulation delete_half(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
							    Population::ReproductionMode::AllowDuplicates,
							    Population::ParentSelectionMode::RouletteWheel,
							    Population::DeletionMode::DeleteHalf,
							    Population::FitnessMode::Evaluation,
							    Population::VariableLengthMode::Fixed, 2));
  delete_half.run(out, PopulationRunReportOptions{false, false});

  InspectablePopulation delete_quarter(make_population_options(Population::OperationMode::Maximize, 8, 10, 1, 0.0, 0.0,
							       Population::ReproductionMode::AllowDuplicates,
							       Population::ParentSelectionMode::RouletteWheel,
							       Population::DeletionMode::DeleteQuarter,
							       Population::FitnessMode::Evaluation,
							       Population::VariableLengthMode::Fixed, 2));
  delete_quarter.run(out, PopulationRunReportOptions{false, false});

  InspectablePopulation delete_last_min(make_population_options(Population::OperationMode::Minimize, 6, 8, 1, 0.0, 0.0,
								Population::ReproductionMode::AllowDuplicates,
								Population::ParentSelectionMode::RouletteWheel,
								Population::DeletionMode::DeleteLast,
								Population::FitnessMode::Evaluation,
								Population::VariableLengthMode::Fixed, 2));
  delete_last_min.run(out, PopulationRunReportOptions{false, false});

  expect_true(PopulationTestRig::isPopulationInitialized(delete_last_min), "Additional run modes should complete successfully");
}

void test_population_execute_returns_structured_result()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  pop.setRandomSeed(123U);
  Population::RunResult result = pop.execute(true);

  expect_true(result.randomSeed == 123U,
	      "execute should report the active random seed");
  expect_true(!result.usedConfiguredSeed,
	      "execute should report when the seed came from explicit runtime injection");
  expect_true(result.generationsCompleted >= 1, "execute should report completed generations");
  expect_true(result.evaluations >= pop.settings().numberOfIndividuals,
	      "execute should report total evaluations");
  expect_true(!result.generationReports.empty(),
	      "execute(true) should capture per-generation reports");
  expect_true(!result.generationSummaries.empty(),
	      "execute(true) should capture per-generation summaries");
  expect_true(!result.finalSummary.mostFit.empty(),
	      "execute should capture the final most-fit summary");
  expect_true(!result.finalSummary.leastFit.empty(),
	      "execute should capture the final least-fit summary");
}

void test_population_execute_result_contents_are_consistent()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = pop.execute(true);

  expect_true(result.generationsCompleted == 1,
	      "execute should report the expected number of generations for this configuration");
  expect_true(result.evaluations == 12,
	      "execute should report the exact evaluation count for this configuration");
  expect_true(result.generationReports.size() == static_cast<std::size_t>(result.generationsCompleted),
	      "execute(true) should produce one generation report per completed generation");
  expect_true(result.generationSummaries.size() == static_cast<std::size_t>(result.generationsCompleted),
	      "execute(true) should produce one summary per completed generation");
  expect_true(result.generationReports.back().generation == 0,
	      "Generation reports should preserve the generation index");
  expect_true(result.generationReports.back().evaluations == result.evaluations,
	      "Generation reports should preserve the evaluation count");
  expect_true(result.finalSummary.mostFit.size() == 5,
	      "Final summary should cap most-fit entries at the configured summary count");
  expect_true(result.finalSummary.leastFit.size() == 5,
	      "Final summary should cap least-fit entries at the configured summary count");

  for (std::size_t i = 1 ; i < result.finalSummary.mostFit.size() ; i++)
    {
      expect_true(result.finalSummary.mostFit[i - 1] >= result.finalSummary.mostFit[i],
		  "Most-fit summary entries should be sorted from best to less-best");
    }

  for (std::size_t i = 1 ; i < result.finalSummary.leastFit.size() ; i++)
    {
      expect_true(result.finalSummary.leastFit[i - 1] <= result.finalSummary.leastFit[i],
		  "Least-fit summary entries should be sorted from worst to less-worst");
    }

  expect_true(result.generationSummaries.back().mostFit == result.finalSummary.mostFit,
	      "Final summary should match the last captured generation summary for this configuration");
  expect_true(result.generationSummaries.back().leastFit == result.finalSummary.leastFit,
	      "Final least-fit summary should match the last captured generation summary");
  expect_true(result.generationReports.back().summary.mostFit == result.finalSummary.mostFit,
	      "Generation reports should carry the same most-fit summary data");
  expect_true(result.generationReports.back().summary.leastFit == result.finalSummary.leastFit,
	      "Generation reports should carry the same least-fit summary data");
}

void test_population_fixed_seed_makes_runs_reproducible()
{
  Population::Settings options = make_population_options(Population::OperationMode::Maximize, 6, 20, 4, 0.05, 0.65,
							Population::ReproductionMode::AllowDuplicates,
							Population::ParentSelectionMode::RouletteWheel,
							Population::DeletionMode::DeleteAll,
							Population::FitnessMode::Evaluation,
							Population::VariableLengthMode::Fixed, 2);
  options.useFixedRandomSeed = true;
  options.randomSeed = 20260424U;

  InspectablePopulation first(options);
  InspectablePopulation second(options);
  Population::RunResult first_result = first.execute(true);
  Population::RunResult second_result = second.execute(true);

  expect_true(first_result.usedConfiguredSeed && second_result.usedConfiguredSeed,
	      "Configured seeds should be reported as configured");
  expect_true(first_result.randomSeed == second_result.randomSeed,
	      "Configured runs should report the same seed");
  expect_true(first_result.evaluations == second_result.evaluations,
	      "Configured runs should report the same evaluation count");
  expect_true(first_result.generationsCompleted == second_result.generationsCompleted,
	      "Configured runs should report the same generation count");
  expect_true(first_result.finalSummary.mostFit == second_result.finalSummary.mostFit,
	      "Configured runs should produce the same final most-fit summary");
  expect_true(first_result.finalSummary.leastFit == second_result.finalSummary.leastFit,
	      "Configured runs should produce the same final least-fit summary");
}

void test_population_stops_early_when_problem_reports_solution()
{
  EarlyStopProblem problem;
  Population pop(make_population_options(Population::OperationMode::Maximize, 6, 100, 1, 0.0, 0.0,
                                         Population::ReproductionMode::AllowDuplicates,
                                         Population::ParentSelectionMode::RouletteWheel,
                                         Population::DeletionMode::DeleteAll,
                                         Population::FitnessMode::Evaluation,
                                         Population::VariableLengthMode::Fixed, 2),
                 problem);

  Population::RunResult result = pop.execute(true);

  expect_true(result.solutionFound,
              "execute should report when a problem identifies a solution");
  expect_true(result.stoppedEarly,
              "execute should mark runs that stop because a solution was found");
  expect_true(result.generationsCompleted == 0,
              "execute should stop before breeding when the initial population already contains a solution");
  expect_true(result.evaluations == pop.settings().numberOfIndividuals,
              "execute should stop after the initial evaluation pass when a solution is already present");
  expect_true(result.generationReports.empty(),
              "execute should not record generation reports when it stops before the first breeding generation");
}

void test_population_run_output_contains_progress_and_final_summary()
{
  DefaultHookPopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  std::ostringstream output;
  pop.run(output, PopulationRunReportOptions{true, true});
  const std::string rendered = output.str();

  expect_true(rendered.find("Generation 0 Number of Evaluations 12") != std::string::npos,
	      "run should print final generation progress");
  expect_true(rendered.find("Chromosome:") != std::string::npos,
	      "run should print chromosome summaries through printCandidate");
  expect_true(rendered.find("1.000000") != std::string::npos || rendered.find("0.000000") != std::string::npos,
	      "run should print final summary fitness values");
}

void test_population_run_reporter_formats_execute_results()
{
  DefaultHookPopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = pop.execute(true);
  std::ostringstream out;
  PopulationReporter::write(out,
				 pop,
				 result,
				 PopulationRunReportOptions{true, true});
  const std::string output = out.str();

  expect_true(output.find("Operation             :: Maximize") != std::string::npos,
	      "RunReporter should be able to render settings");
  expect_true(output.find("Generation 0 Number of Evaluations 12") != std::string::npos,
	      "RunReporter should be able to render generation progress from execute results");
  expect_true(output.find("Chromosome:") != std::string::npos,
	      "RunReporter should render candidate output through printCandidate");
}

void test_population_run_reporter_writes_json()
{
  DefaultHookPopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = pop.execute(true);
  std::ostringstream out;
  PopulationReporter::writeJson(out, pop, result);
  const std::string json = out.str();

  expect_true(json.find("\"settings\"") != std::string::npos,
	      "RunReporter JSON should include settings");
  expect_true(json.find("\"chromosome_length\": 1") != std::string::npos,
	      "RunReporter JSON should include chromosome length");
  expect_true(json.find("\"result\"") != std::string::npos,
	      "RunReporter JSON should include result data");
  expect_true(json.find("\"generation_reports\"") != std::string::npos,
	      "RunReporter JSON should include generation reports");
  expect_true(json.find("\"best_candidate_genes\"") != std::string::npos,
	      "RunReporter JSON should include best-candidate gene snapshots");
  expect_true(json.find("\"visualization\"") != std::string::npos,
	      "RunReporter JSON should include visualization payloads");
}

void test_population_run_reporter_writes_csv()
{
  DefaultHookPopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = pop.execute(true);
  std::ostringstream out;
  PopulationReporter::writeGenerationCsv(out, pop, result);
  const std::string csv = out.str();

  expect_true(csv.find("generation,evaluations,best_fitness,worst_fitness") == 0,
	      "RunReporter CSV should include a header row");
  expect_true(csv.find("0,12,") != std::string::npos,
	      "RunReporter CSV should include generation progress rows");
}

void test_population_default_operator_hooks_are_explicitly_exercised()
{
  DefaultHookPopulation pop(make_population_options(Population::OperationMode::Maximize, 4, 4, 6, 1.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));

  std::unique_ptr<Chromosome> initial = PopulationTestRig::createInitialChromosome(pop);
  expect_true(initial.get() != nullptr, "Default createInitialChromosome should produce a chromosome");
  expect_true(initial->length() == pop.settings().chromosomeLength,
	      "Default createInitialChromosome should use the configured chromosome length");
  for (int i = 0 ; i < initial->length() ; i++)
    {
      int value = initial->genes().valueAt(i);
      expect_true(value == 0 || value == 1,
		  "Default createInitialChromosome should respect the configured base state range");
    }

  std::unique_ptr<Chromosome> mother = std::make_unique<Chromosome>(makeBinaryString("111100"));
  std::unique_ptr<Chromosome> father = std::make_unique<Chromosome>(makeBinaryString("000011"));
  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
    PopulationTestRig::mateChromosomes(pop, *mother, *father);

  expect_true(children.first.get() != nullptr && children.second.get() != nullptr,
	      "Default mateChromosomes should produce two children");
  expect_true(children.first->equals(*father),
	      "Default mateChromosomes should clone the father into the first child when crossover is disabled");
  expect_true(children.second->equals(*mother),
	      "Default mateChromosomes should clone the mother into the second child when crossover is disabled");

  PopulationTestRig::mutateChromosome(pop, *children.first);
  for (int i = 0 ; i < children.first->length() ; i++)
    {
      int value = children.first->genes().valueAt(i);
      expect_true(value == 0 || value == 1,
		  "Default mutateChromosome should keep binary genes in range");
    }
}

void test_population_operator_strategies_override_default_hooks()
{
  StrategyHookPopulation pop(make_population_options(Population::OperationMode::Maximize, 4, 4, 4, 1.0, 0.0,
						     Population::ReproductionMode::AllowDuplicates,
						     Population::ParentSelectionMode::RouletteWheel,
						     Population::DeletionMode::DeleteAll,
						     Population::FitnessMode::Evaluation,
						     Population::VariableLengthMode::Fixed, 2));

  std::unique_ptr<Chromosome> initial = PopulationTestRig::createInitialChromosome(pop);
  for (int i = 0 ; i < initial->length() ; i++)
    {
      expect_true(initial->genes().valueAt(i) == 1,
		  "Custom initialization strategies should control the created chromosome");
    }

  std::unique_ptr<Chromosome> mother = std::make_unique<Chromosome>(makeBinaryString("1111"));
  std::unique_ptr<Chromosome> father = std::make_unique<Chromosome>(makeBinaryString("0000"));
  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
    PopulationTestRig::mateChromosomes(pop, *mother, *father);

  expect_true(children.first->equals(*father),
	      "Custom mating strategies should control the first child");
  expect_true(children.second->equals(*mother),
	      "Custom mating strategies should control the second child");

  PopulationTestRig::mutateChromosome(pop, *children.first);
  for (int i = 0 ; i < children.first->length() ; i++)
    {
      expect_true(children.first->genes().valueAt(i) == 0,
		  "Custom mutation strategies should control mutation behavior");
    }
}

void test_delete_all_but_best_runs()
{
  // Fixed seed: with mutation and crossover off no new 1 bit can appear, so
  // an unseeded run failed whenever all 6 random initial members were 0
  // (about 1 run in 64). Seed 1 starts with at least one 1 on every platform.
  Population::Settings settings = make_population_options(Population::OperationMode::Maximize, 6, 20, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAllButBest,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2);
  settings.useFixedRandomSeed = true;
  settings.randomSeed = 1;
  InspectablePopulation pop(settings);
  std::ostringstream out;
  pop.run(out, PopulationRunReportOptions{false, false});

  expect_true(PopulationTestRig::isPopulationInitialized(pop) == true, "Population should initialize during run");
  expect_true(!PopulationTestRig::chromosomes(pop).empty(), "Population should retain its table after run");
  expect_true(PopulationTestRig::chromosomes(pop)[pop.settings().numberOfIndividuals - 1].get()->genes().valueAt(0) == 1,
	      "Best chromosome should remain present after DeleteAllButBest runs");
}

void test_windowed_minimize_selection_favours_best()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Minimize, 3, 3, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Windowed,
						    Population::VariableLengthMode::Fixed, 3));
  PopulationTestRig::initializePopulation(pop);
  for (int i = 0 ; i < 3 ; i++)
    {
      BaseString genes(1, 3);
      genes.setValue(0, i);
      PopulationTestRig::chromosomes(pop)[i] = std::make_unique<Chromosome>(std::move(genes), false, 3);
      PopulationTestRig::evaluatedFitness(pop)[i] = false;
    }
  PopulationTestRig::setPopulationInitialized(pop, true);
  PopulationTestRig::evaluatePopulation(pop);

  // Sorted ascending: index 0 holds fitness 0 (best when minimizing).
  int picks[3] = {0, 0, 0};
  for (int draw = 0 ; draw < 3000 ; draw++)
    {
      picks[PopulationTestRig::selectParent(pop, PopulationTestRig::selectFitnessWeights(pop))]++;
    }
  expect_true(picks[0] > picks[2],
	      "Windowed roulette selection should favour the best candidate when minimizing");
}

void test_deletion_mode_that_replaces_nobody_throws()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 3, 100, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteQuarter,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  expect_throws<GAFatalException>(
    [&pop]() { (void)pop.execute(); },
    "DeleteQuarter on a population smaller than 4 should throw instead of hanging");

  InspectablePopulation single(make_population_options(Population::OperationMode::Maximize, 1, 100, 1, 0.0, 0.0,
						       Population::ReproductionMode::AllowDuplicates,
						       Population::ParentSelectionMode::RouletteWheel,
						       Population::DeletionMode::DeleteLast,
						       Population::FitnessMode::Evaluation,
						       Population::VariableLengthMode::Fixed, 2));
  expect_throws<GAFatalException>(
    [&single]() { (void)single.execute(); },
    "DeleteLast on a single-member population should throw");
}

void test_duplicate_rejection_stall_terminates()
{
  // Only two distinct 1-bit chromosomes exist, so with duplicates disallowed
  // a population of 4 quickly stops accepting children.
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 4, 1000000, 1, 0.0, 0.0,
						    Population::ReproductionMode::DisallowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = pop.execute();
  expect_true(result.stalled, "A run that can no longer insert children should report a stall");
  expect_true(result.stoppedEarly, "A stalled run should be marked as stopped early");
  expect_true(!result.solutionFound, "A stalled run should not claim a solution");
  expect_true(result.evaluations < pop.settings().numberOfTrials,
	      "A stalled run should end before exhausting its trial budget");
}

void test_mutation_always_changes_genes()
{
  std::mt19937 random_generator(11);
  Chromosome binary(makeBinaryString("01100101"));
  const std::string before = binary.genes().bitString();
  binary.mutate(1.0, &random_generator);
  for (int i = 0 ; i < binary.length() ; i++)
    {
      expect_true(binary.genes().valueAt(i) != (before[i] - '0'),
		  "Binary mutation with probability one should flip every bit");
    }

  BaseString symbolic_genes(6, 5);
  for (int i = 0 ; i < 6 ; i++)
    {
      symbolic_genes.setValue(i, i % 5);
    }
  Chromosome symbolic(std::move(symbolic_genes), false, 5);
  symbolic.mutate(1.0, &random_generator);
  for (int i = 0 ; i < symbolic.length() ; i++)
    {
      const int value = symbolic.genes().valueAt(i);
      expect_true(value != i % 5, "Symbolic mutation with probability one should change every gene");
      expect_true(value >= 0 && value < 5, "Symbolic mutation should stay in range");
    }
}

void test_set_value_rejects_out_of_range_states()
{
  BaseString genes(3, 5);
  genes.setValue(0, 4);
  expect_true(genes.valueAt(0) == 4, "setValue should accept the largest valid state");
  expect_throws<GAFatalException>([&genes]() { genes.setValue(1, 5); },
				  "setValue should reject a value equal to the state count");
  expect_throws<GAFatalException>([&genes]() { genes.setValue(1, 7); },
				  "setValue should reject values the packed bits could hold but the alphabet cannot");
  expect_throws<GAFatalException>([&genes]() { genes.setValue(1, -1); },
				  "setValue should reject negative values");
}

void test_chromosome_rejects_mismatched_state_count()
{
  expect_throws<GAFatalException>([]() { Chromosome bad(BaseString(4, 3), false, 5); },
				  "Chromosome should reject a state count that differs from its BaseString");
}

void test_variable_length_crossover_is_capped()
{
  BaseString long_genes(2048, 2);
  Chromosome mother(BaseString(2048, 2), true, 2);
  Chromosome father(std::move(long_genes), true, 2);
  std::mt19937 random_generator(3);
  for (int trial = 0 ; trial < 50 ; trial++)
    {
      std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
	mother.mate(father, 1.0, Chromosome::CrossoverType::SinglePoint, &random_generator);
      expect_true(children.first->length() <= 2048 && children.second->length() <= 2048,
		  "Variable-length crossover should never exceed the 2048-gene limit");
    }
  expect_throws<GAFatalException>([]() { Chromosome too_long(BaseString(2049, 2), true, 2); },
				  "Chromosome built from an oversized BaseString should throw");
}

void test_maximize_roulette_rejects_negative_fitness()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  PopulationTestRig::initializePopulation(pop);
  std::vector<double> roulette = {-1.0, 2.0};
  expect_throws<GAFatalException>([&pop, &roulette]() { PopulationTestRig::selectParent(pop, roulette); },
				  "Maximize roulette on raw fitness should reject negative values");
}

void test_minimize_roulette_handles_all_negative_fitness()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Minimize, 2, 2, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  PopulationTestRig::initializePopulation(pop);
  // -10 is the best when minimizing; inverted around the true maximum (-1)
  // it should receive weight 10 against weight 1.
  std::vector<double> roulette = {-10.0, -1.0};
  int best = 0;
  for (int draw = 0 ; draw < 2000 ; draw++)
    {
      if (PopulationTestRig::selectParent(pop, roulette) == 0) ++best;
    }
  expect_true(best > 1600, "Minimize roulette should strongly favour the most negative fitness");
}

void test_minimize_summary_is_best_first()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Minimize, 8, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 8));
  PopulationTestRig::initializePopulation(pop);
  for (int i = 0 ; i < 8 ; i++)
    {
      BaseString genes(1, 8);
      genes.setValue(0, 7 - i);
      PopulationTestRig::chromosomes(pop)[i] = std::make_unique<Chromosome>(std::move(genes), false, 8);
      PopulationTestRig::evaluatedFitness(pop)[i] = false;
    }
  PopulationTestRig::setPopulationInitialized(pop, true);
  PopulationTestRig::evaluatePopulation(pop);

  Population::RunResult result;
  result.finalSummary = PopulationTestRig::buildPopulationSummary(pop);
  const std::vector<double> expected_best = {0.0, 1.0, 2.0, 3.0, 4.0};
  const std::vector<double> expected_worst = {7.0, 6.0, 5.0, 4.0, 3.0};
  expect_true(result.finalSummary.mostFit == expected_best,
	      "Minimize summaries should list the smallest fitness values as most fit, best first");
  expect_true(result.finalSummary.leastFit == expected_worst,
	      "Minimize summaries should list the largest fitness values as least fit, worst first");

  std::ostringstream csv;
  result.generationReports.resize(1);
  result.generationReports[0].summary = result.finalSummary;
  PopulationReporter::writeGenerationCsv(csv, pop, result);
  expect_true(csv.str().find("\n0,0,0.00000000,7.00000000\n") != std::string::npos,
	      "CSV best_fitness/worst_fitness for a minimize run should be the smallest/largest values");
}

class NonFiniteFitnessProblem : public PopulationProblem
{
public:
  explicit NonFiniteFitnessProblem(double value) : value_(value) {}
  double evaluateFitness(const BaseString&) override { return value_; }
  void printCandidate(const BaseString&, std::ostream&) const override {}
private:
  double value_;
};

void test_non_finite_fitness_throws()
{
  const double values[] = {std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::infinity(),
                           -std::numeric_limits<double>::infinity()};
  for (double value : values)
    {
      NonFiniteFitnessProblem problem(value);
      Population pop(make_population_options(Population::OperationMode::Maximize, 4, 8, 2, 0.0, 0.0,
                                             Population::ReproductionMode::AllowDuplicates,
                                             Population::ParentSelectionMode::RouletteWheel,
                                             Population::DeletionMode::DeleteAll,
                                             Population::FitnessMode::Evaluation,
                                             Population::VariableLengthMode::Fixed, 2),
                     problem);
      expect_throws<GAFatalException>([&pop]() { (void)pop.execute(); },
                                      "Non-finite fitness values should be rejected");
    }
}

void test_out_of_range_crossover_rate_throws()
{
  for (double rate : {1.5, -0.5, std::numeric_limits<double>::quiet_NaN()})
    {
      InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 4, 8, 2, 0.0, rate,
                                                        Population::ReproductionMode::AllowDuplicates,
                                                        Population::ParentSelectionMode::RouletteWheel,
                                                        Population::DeletionMode::DeleteAll,
                                                        Population::FitnessMode::Evaluation,
                                                        Population::VariableLengthMode::Fixed, 2));
      expect_throws<GAFatalException>([&pop]() { (void)pop.execute(); },
                                      "Population should reject crossover rates outside [0, 1]");
    }
  InspectablePopulation bad_mutation(make_population_options(Population::OperationMode::Maximize, 4, 8, 2, 1.5, 0.5,
                                                             Population::ReproductionMode::AllowDuplicates,
                                                             Population::ParentSelectionMode::RouletteWheel,
                                                             Population::DeletionMode::DeleteAll,
                                                             Population::FitnessMode::Evaluation,
                                                             Population::VariableLengthMode::Fixed, 2));
  expect_throws<GAFatalException>([&bad_mutation]() { (void)bad_mutation.execute(); },
                                  "Population should reject mutation rates outside [0, 1]");

  Chromosome mother(makeBinaryString("0000"));
  Chromosome father(makeBinaryString("1111"));
  expect_throws<GAFatalException>([&]() { (void)mother.mate(father, 1.5); },
                                  "Chromosome::mate should reject crossover rates above 1");
}

void test_json_report_accepts_extra_result_members()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 4, 8, 1, 0.0, 0.0,
                                                    Population::ReproductionMode::AllowDuplicates,
                                                    Population::ParentSelectionMode::RouletteWheel,
                                                    Population::DeletionMode::DeleteAll,
                                                    Population::FitnessMode::Evaluation,
                                                    Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = pop.execute(true);
  std::ostringstream plain;
  PopulationReporter::writeJson(plain, pop, result);
  std::ostringstream extended;
  PopulationReporter::writeJson(extended, pop, result, "\"selected\": { \"answer\": 42 }");
  const std::string text = extended.str();
  const std::size_t reports = text.find("\"generation_reports\"");
  const std::size_t extra = text.find("\"selected\": { \"answer\": 42 }");
  expect_true(plain.str().find("selected") == std::string::npos,
              "Without extra members the JSON report should be unchanged");
  expect_true(extra != std::string::npos && extra > reports &&
              text.find("],\n    \"selected\"") != std::string::npos &&
              text.size() >= 11 && text.compare(text.size() - 11, 11, "42 }\n  }\n}\n") == 0,
              "Extra members should be appended inside the result object after the generation reports");
}

void test_reporters_restore_stream_state()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 12, 1, 0.0, 0.0,
                                                    Population::ReproductionMode::AllowDuplicates,
                                                    Population::ParentSelectionMode::RouletteWheel,
                                                    Population::DeletionMode::DeleteAll,
                                                    Population::FitnessMode::Evaluation,
                                                    Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = pop.execute(true);
  std::ostringstream out;
  out.precision(3);
  const std::ios::fmtflags flags = out.flags();
  PopulationReporter::write(out, pop, result, PopulationRunReportOptions(true, true));
  PopulationReporter::writeJson(out, pop, result);
  PopulationReporter::writeGenerationCsv(out, pop, result);
  expect_true(out.precision() == 3 && out.flags() == flags,
              "Reporters should leave the caller's stream formatting unchanged");
}

void test_verbose_report_prints_generation_snapshot()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 24, 1, 0.0, 0.0,
                                                    Population::ReproductionMode::AllowDuplicates,
                                                    Population::ParentSelectionMode::RouletteWheel,
                                                    Population::DeletionMode::DeleteAll,
                                                    Population::FitnessMode::Evaluation,
                                                    Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = pop.execute(true);
  std::ostringstream out;
  PopulationReporter::write(out, pop, result, PopulationRunReportOptions(false, true));
  expect_true(out.str().find("Best Member:") != std::string::npos,
              "Verbose reports should print each generation's own best-member snapshot");
  expect_true(out.str().find("Members are:") == std::string::npos,
              "Verbose reports should not pair the final population with historical fitness values");
}

void test_portable_random_golden_values()
{
  // These values come from the portable helpers alone (raw mt19937 output is
  // fixed by the standard), so they must match on libstdc++ and libc++.
  std::mt19937 generator(1);
  const int expected_indices[5] = {5, 9, 4, 8, 3};
  for (int i = 0 ; i < 5 ; i++)
    {
      expect_true(gak::uniformIndex(generator, 10) == expected_indices[i],
                  "gak::uniformIndex should produce the documented sequence for seed 1");
    }
  const double unit = gak::uniformUnit(generator);
  expect_true(unit > 0.1281 && unit < 0.1282, "gak::uniformUnit should produce the documented value");
  expect_true(gak::chance(generator, 0.5) == false, "gak::chance should produce the documented value");
  expect_true(gak::uniformInt(generator, -5, 5) == -5, "gak::uniformInt should produce the documented value");
  std::vector<int> values = {0, 1, 2, 3, 4, 5};
  gak::shuffle(values.begin(), values.end(), generator);
  expect_true(values == std::vector<int>({3, 1, 5, 0, 4, 2}),
              "gak::shuffle should produce the documented permutation");
}

// Maximize the number of 1 bits; "solved" once every bit is set. The
// initial population is forced to be all zeros so the solution can only be
// found by breeding.
class OneMaxProblem : public PopulationProblem
{
public:
  double evaluateFitness(const BaseString& genes) override
  {
    int ones = 0;
    for (int i = 0 ; i < genes.length() ; i++) ones += genes.valueAt(i);
    return ones;
  }
  void printCandidate(const BaseString&, std::ostream&) const override {}
  bool hasReachedSolution(const Population&, const BaseString& genes, double fitness) const override
  {
    return fitness >= genes.length();
  }
  std::unique_ptr<Chromosome> initializeCandidate(Population& population) override
  {
    return std::make_unique<Chromosome>(BaseString(population.settings().chromosomeLength, 2), false, 2);
  }
};

void test_population_stops_when_solution_found_mid_run()
{
  OneMaxProblem problem;
  Population::Settings settings = make_population_options(Population::OperationMode::Maximize, 20, 200000, 6,
                                                          0.1, 0.7,
                                                          Population::ReproductionMode::AllowDuplicates,
                                                          Population::ParentSelectionMode::Tournament,
                                                          Population::DeletionMode::DeleteHalf,
                                                          Population::FitnessMode::Evaluation,
                                                          Population::VariableLengthMode::Fixed, 2);
  settings.useFixedRandomSeed = true;
  settings.randomSeed = 7;
  Population pop(settings, problem);
  Population::RunResult result = pop.execute(true);
  expect_true(result.solutionFound && result.stoppedEarly && !result.stalled,
              "execute should stop as soon as breeding produces a solution");
  expect_true(result.generationsCompleted > 0 && result.evaluations < settings.numberOfTrials,
              "a mid-run solution should end the run before the trial budget is spent");
  expect_true(!result.generationReports.empty() &&
              result.generationReports.back().bestCandidateFitness == 6.0 &&
              result.finalSummary.mostFit.front() == 6.0,
              "the final generation report should contain the solution");
}

void test_report_labels_cover_every_mode()
{
  struct Case { Population::DeletionMode deletion; Population::FitnessMode fitness;
                Population::ParentSelectionMode selection; const char* labels[3]; };
  const Case cases[] = {
    {Population::DeletionMode::DeleteAll, Population::FitnessMode::Evaluation,
     Population::ParentSelectionMode::RouletteWheel, {"delete_all", "evaluation", "roulette"}},
    {Population::DeletionMode::DeleteAllButBest, Population::FitnessMode::Windowed,
     Population::ParentSelectionMode::Random, {"delete_all_but_best", "windowed", "random"}},
    {Population::DeletionMode::DeleteHalf, Population::FitnessMode::LinearNormalized,
     Population::ParentSelectionMode::Tournament, {"delete_half", "linear_normalized", "tournament"}},
    {Population::DeletionMode::DeleteQuarter, Population::FitnessMode::Evaluation,
     Population::ParentSelectionMode::RouletteWheel, {"delete_quarter", "evaluation", "roulette"}},
    {Population::DeletionMode::DeleteLast, Population::FitnessMode::Evaluation,
     Population::ParentSelectionMode::RouletteWheel, {"delete_last", "evaluation", "roulette"}},
  };
  for (const Case& c : cases)
    {
      InspectablePopulation pop(make_population_options(Population::OperationMode::Minimize, 8, 16, 2, 0.1, 0.5,
                                                        Population::ReproductionMode::AllowDuplicates,
                                                        c.selection, c.deletion, c.fitness,
                                                        Population::VariableLengthMode::Fixed, 2));
      Population::RunResult result = pop.execute(true);
      std::ostringstream json;
      PopulationReporter::writeJson(json, pop, result);
      const std::string text = json.str();
      expect_true(text.find(std::string("\"deletion\": \"") + c.labels[0] + "\"") != std::string::npos &&
                  text.find(std::string("\"fitness\": \"") + c.labels[1] + "\"") != std::string::npos &&
                  text.find(std::string("\"parent_selection\": \"") + c.labels[2] + "\"") != std::string::npos,
                  std::string("JSON report should label deletion/fitness/selection modes: ") + c.labels[0]);
      expect_true(text.find("\"operation\": \"minimize\"") != std::string::npos,
                  "JSON report should label the operation mode");
    }
}

void test_population_public_helpers_and_validation()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
                                                    Population::ReproductionMode::AllowDuplicates,
                                                    Population::ParentSelectionMode::RouletteWheel,
                                                    Population::DeletionMode::DeleteAll,
                                                    Population::FitnessMode::Evaluation,
                                                    Population::VariableLengthMode::Fixed, 2));
  Population& population = pop;
  expect_true(population.evaluateFitness(makeBinaryString("1")) == 1.0,
              "Population::evaluateFitness should delegate to the problem");

  InspectablePopulation empty(make_population_options(Population::OperationMode::Maximize, 0, 2, 1, 0.0, 0.0,
                                                      Population::ReproductionMode::AllowDuplicates,
                                                      Population::ParentSelectionMode::RouletteWheel,
                                                      Population::DeletionMode::DeleteAll,
                                                      Population::FitnessMode::Evaluation,
                                                      Population::VariableLengthMode::Fixed, 2));
  expect_throws<GAFatalException>([&empty]() { (void)empty.execute(); },
                                  "A population with no individuals should be rejected");

  std::vector<std::unique_ptr<Chromosome> > replacements;
  expect_true(!PopulationTestRig::appendReplacement(pop, replacements, std::unique_ptr<Chromosome>(), 2, true),
              "appendReplacement should ignore a missing child");

  InspectablePopulation reporting(make_population_options(Population::OperationMode::Maximize, 4, 8, 1, 0.0, 0.0,
                                                          Population::ReproductionMode::AllowDuplicates,
                                                          Population::ParentSelectionMode::RouletteWheel,
                                                          Population::DeletionMode::DeleteAll,
                                                          Population::FitnessMode::Evaluation,
                                                          Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = reporting.execute(true);
  reporting.setOperationMode(static_cast<Population::OperationMode>(99));
  std::ostringstream out;
  expect_throws<GAFatalException>(
    [&]() { PopulationReporter::write(out, reporting, result, PopulationRunReportOptions()); },
    "The text reporter should reject an unsupported operation mode");
}

void test_tournament_selection()
{
  for (Population::OperationMode mode : {Population::OperationMode::Maximize, Population::OperationMode::Minimize})
    {
      Population::Settings settings = make_population_options(mode, 10, 10, 1, 0.0, 0.0,
                                                              Population::ReproductionMode::AllowDuplicates,
                                                              Population::ParentSelectionMode::Tournament,
                                                              Population::DeletionMode::DeleteAll,
                                                              Population::FitnessMode::Evaluation,
                                                              Population::VariableLengthMode::Fixed, 2);
      settings.tournamentSize = 4;
      InspectablePopulation pop(settings);
      PopulationTestRig::initializePopulation(pop);
      const int bestIndex = mode == Population::OperationMode::Maximize ? 9 : 0;
      int bestHalf = 0;
      int bestPicks = 0;
      for (int draw = 0 ; draw < 4000 ; draw++)
        {
          const int index = PopulationTestRig::selectTournamentParent(pop);
          const bool inBestHalf = mode == Population::OperationMode::Maximize ? index >= 5 : index < 5;
          bestHalf += inBestHalf ? 1 : 0;
          bestPicks += index == bestIndex ? 1 : 0;
        }
      // With k = 4 the best half wins 1 - 0.5^4 = 93.75% of tournaments and
      // the single best member 1 - 0.9^4 = 34.4%.
      expect_true(bestHalf > 3600 && bestHalf < 3900,
                  "Tournament selection should favour the fitter half at the expected rate");
      expect_true(bestPicks > 1200 && bestPicks < 1550,
                  "Tournament selection should pick the best member at the expected rate");
    }

  Population::Settings bad = make_population_options(Population::OperationMode::Maximize, 4, 8, 1, 0.0, 0.0,
                                                     Population::ReproductionMode::AllowDuplicates,
                                                     Population::ParentSelectionMode::Tournament,
                                                     Population::DeletionMode::DeleteAll,
                                                     Population::FitnessMode::Evaluation,
                                                     Population::VariableLengthMode::Fixed, 2);
  bad.tournamentSize = 0;
  InspectablePopulation invalid(bad);
  expect_throws<GAFatalException>([&invalid]() { (void)invalid.execute(); },
                                  "A tournament size below 1 should be rejected");

  Population::Settings run = bad;
  run.tournamentSize = 2;
  InspectablePopulation runnable(run);
  Population::RunResult result = runnable.execute(true);
  std::ostringstream json;
  PopulationReporter::writeJson(json, runnable, result);
  expect_true(json.str().find("\"parent_selection\": \"tournament\"") != std::string::npos &&
              json.str().find("\"tournament_size\": 2") != std::string::npos,
              "JSON reports should record tournament selection and its size");
}

void test_selection_wheel_matches_weights()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 3, 3, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  PopulationTestRig::initializePopulation(pop);
  std::vector<double> roulette = {0.0, 1.0, 3.0};
  int picks[3] = {0, 0, 0};
  for (int draw = 0 ; draw < 4000 ; draw++)
    {
      picks[PopulationTestRig::selectParent(pop, roulette)]++;
    }
  expect_true(picks[0] == 0, "Zero-weight candidates should never be selected");
  expect_true(picks[2] > 2 * picks[1], "Selection frequency should follow the roulette weights");
}

}

int main()
{
  test_base_string();
  test_population_options_round_trip();
  test_population_can_use_external_problem_object();
  test_base_string_error_paths();
  test_base_string_print_helpers();
  test_exception_helpers();
  test_mutation_rate_zero_preserves_chromosome();
  test_chromosome_constructor_and_compare_paths();
  test_invalid_mutation_probability_throws();
  test_non_binary_mutation_with_probability_one_stays_in_range();
  test_binary_mutation_with_probability_one_changes_only_bits();
  test_mate_without_crossover_clones_parents();
  test_uniform_crossover_is_reachable_and_safe();
  test_two_point_crossover_is_reachable_and_safe();
  test_mate_rejects_mismatched_fixed_lengths();
  test_variable_length_mating_supports_different_lengths();
  test_variable_length_uniform_crossover();
  test_variable_length_uniform_crossover_with_longer_mother();
  test_invalid_crossover_type_throws();
  test_base_string_decode_bits();
  test_population_random_helpers();
  test_population_zero_total_selection_falls_back_to_uniform();
  test_windowed_fitness_is_positive_for_maximize();
  test_windowed_fitness_is_positive_for_minimize();
  test_fitness_table_selection();
  test_negative_fitness_values_are_not_treated_as_unevaluated();
  test_population_selection_and_replacement_branches();
  test_population_append_replacement_helper();
  test_population_insert_new_population_rejects_overflow();
  test_population_selection_guard_and_random_fitness_modes();
  test_population_invalid_enum_paths();
  test_population_verbose_path();
  test_population_verbose_minimize_path();
  test_population_run_modes();
  test_population_execute_returns_structured_result();
  test_population_execute_result_contents_are_consistent();
  test_population_fixed_seed_makes_runs_reproducible();
  test_population_stops_early_when_problem_reports_solution();
  test_population_run_output_contains_progress_and_final_summary();
  test_population_run_reporter_formats_execute_results();
  test_population_run_reporter_writes_json();
  test_population_run_reporter_writes_csv();
  test_population_default_operator_hooks_are_explicitly_exercised();
  test_population_operator_strategies_override_default_hooks();
  test_delete_all_but_best_runs();
  test_windowed_minimize_selection_favours_best();
  test_deletion_mode_that_replaces_nobody_throws();
  test_duplicate_rejection_stall_terminates();
  test_mutation_always_changes_genes();
  test_set_value_rejects_out_of_range_states();
  test_chromosome_rejects_mismatched_state_count();
  test_variable_length_crossover_is_capped();
  test_maximize_roulette_rejects_negative_fitness();
  test_minimize_roulette_handles_all_negative_fitness();
  test_selection_wheel_matches_weights();
  test_minimize_summary_is_best_first();
  test_non_finite_fitness_throws();
  test_out_of_range_crossover_rate_throws();
  test_reporters_restore_stream_state();
  test_json_report_accepts_extra_result_members();
  test_verbose_report_prints_generation_snapshot();
  test_portable_random_golden_values();
  test_tournament_selection();
  test_population_stops_when_solution_found_mid_run();
  test_report_labels_cover_every_mode();
  test_population_public_helpers_and_validation();

  if (g_failures != 0)
    {
      std::cerr << g_failures << " test(s) failed" << std::endl;
      return 1;
    }

  std::cout << "All tests passed" << std::endl;
  return 0;
}
