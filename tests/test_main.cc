#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>

#include "base.hh"
#include "chromosome.hh"
#include "except.hh"
#include "stringutil.hh"

#define private public
#include "population.hh"
#undef private

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

class SilentStderr
{
public:
  SilentStderr()
  {
    fflush(stderr);
    saved_fd_ = dup(fileno(stderr));
    null_ = fopen("/dev/null","w");
    if (saved_fd_ >= 0 && null_ != NULL)
      {
        dup2(fileno(null_), fileno(stderr));
      }
  }

  ~SilentStderr()
  {
    fflush(stderr);
    if (saved_fd_ >= 0)
      {
        dup2(saved_fd_, fileno(stderr));
        close(saved_fd_);
      }
    if (null_ != NULL)
      {
        fclose(null_);
      }
  }

private:
  int saved_fd_;
  FILE *null_;
};

BaseString *makeBinaryString(const std::string& bits)
{
  BaseString *b = new BaseString(bits.length(), 2);
  for (int i = 0 ; i < (int)bits.length() ; i++)
    {
      if (bits[i] == '1') b->set(i);
      else b->clear(i);
    }
  return b;
}

Population::Options make_population_options(Population::OperationMode operation,
					    int individuals,
					    int trials,
					    int diversity,
					    double mutation,
					    double crossover,
					    Population::ReproductionMode reproduction,
					    Population::ParentSelectionMode selection,
					    Population::DeletionMode deletion,
					    Population::FitnessMode fitness,
					    Population::VariableLengthMode variable_length,
					    int states);

class InspectablePopulation : public Population
{
public:
  explicit InspectablePopulation(const Options& options)
    : Population(options)
  {
  }

  InspectablePopulation(OperationTechnique op,
			int individuals,
			int trials,
			int diversity,
			double mutation,
			double crossover,
			ReproductionTechniques reproduction,
			ParentSelectionTechnique selection,
			DeletionTechnique deletion,
			FitnessTechnique fitness,
			VariableLength variable,
			int states)
    : Population(make_population_options(op == Minimize ? Population::OperationMode::Minimize
						       : Population::OperationMode::Maximize,
					 individuals,
					 trials,
					 diversity,
					 mutation,
					 crossover,
					 reproduction == DuplicatesNotAllowed
					    ? Population::ReproductionMode::DisallowDuplicates
					    : Population::ReproductionMode::AllowDuplicates,
					 selection == Random
					    ? Population::ParentSelectionMode::Random
					    : Population::ParentSelectionMode::RouletteWheel,
					 deletion == DeleteAllButBest
					    ? Population::DeletionMode::DeleteAllButBest
					    : deletion == DeleteHalf
					       ? Population::DeletionMode::DeleteHalf
					       : deletion == DeleteQuarter
						  ? Population::DeletionMode::DeleteQuarter
						  : deletion == DeleteLast
						     ? Population::DeletionMode::DeleteLast
						     : Population::DeletionMode::DeleteAll,
					 fitness == WindowedFitness
					    ? Population::FitnessMode::Windowed
					    : fitness == LinearNormalizedFitness
					       ? Population::FitnessMode::LinearNormalized
					       : Population::FitnessMode::Evaluation,
					 variable == VariableLengthPermitted
					    ? Population::VariableLengthMode::Variable
					    : Population::VariableLengthMode::Fixed,
					 states))
  {
  }

  double FitnessFunction(BaseString *b) override
  {
    return b->test(0);
  }

  void FitnessPrint(BaseString *) override
  {
  }
};

Population::Options make_population_options(Population::OperationMode operation,
					    int individuals,
					    int trials,
					    int diversity,
					    double mutation,
					    double crossover,
					    Population::ReproductionMode reproduction,
					    Population::ParentSelectionMode selection,
					    Population::DeletionMode deletion,
					    Population::FitnessMode fitness,
					    Population::VariableLengthMode variable_length,
					    int states)
{
  Population::Options options;
  options.operation = operation;
  options.numberOfIndividuals = individuals;
  options.numberOfTrials = trials;
  options.geneticDiversity = diversity;
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
  binary.clear(0);
  binary.set(1);
  binary.clear(2);
  binary.set(3);

  expect_true(binary.length() == 4, "BaseString length should match constructor");
  expect_true(binary.test(0) == 0, "Binary base 0 should be cleared");
  expect_true(binary.test(1) == 1, "Binary base 1 should be set");
  expect_true(binary.test(2) == 0, "Binary base 2 should be cleared");
  expect_true(binary.test(3) == 1, "Binary base 3 should be set");

  BaseString symbolic(3, 5);
  symbolic.set(0, 4);
  symbolic.set(1, 2);
  symbolic.clear(2);

  BaseString byte_aligned(4, 4);
  byte_aligned.set(0, 3);
  byte_aligned.set(1, 2);
  byte_aligned.set(2, 1);
  byte_aligned.clear(3);

  expect_true(symbolic.test(0) == 4, "Symbolic base should preserve assigned values");
  expect_true(symbolic.test(1) == 2, "Symbolic base should preserve non-binary states");
  expect_true(symbolic.test(2) == 0, "Clear should reset symbolic base to zero");
  expect_true(byte_aligned.test(0) == 3, "Byte-aligned symbolic bases should store values correctly");
  expect_true(byte_aligned.test(3) == 0, "Byte-aligned clear should preserve zero values");
}

void test_population_options_round_trip()
{
  Population::Configuration configuration;
  configuration.operation = Population::Minimize;
  configuration.numberOfIndividuals = 17;
  configuration.numberOfTrials = 99;
  configuration.geneticDiversity = 23;
  configuration.bitMutationRate = 0.125;
  configuration.crossOverRate = 0.75;
  configuration.reproduction = Population::DuplicatesNotAllowed;
  configuration.parentSelection = Population::Random;
  configuration.deletion = Population::DeleteQuarter;
  configuration.fitness = Population::LinearNormalizedFitness;
  configuration.variableLength = Population::VariableLengthPermitted;
  configuration.baseStates = 7;

  Population::Options options = configuration.toOptions();
  Population::Configuration round_trip = options.toConfiguration();

  expect_true(options.operation == Population::OperationMode::Minimize,
              "Configuration::toOptions should convert operation");
  expect_true(options.reproduction == Population::ReproductionMode::DisallowDuplicates,
              "Configuration::toOptions should convert reproduction");
  expect_true(options.parentSelection == Population::ParentSelectionMode::Random,
              "Configuration::toOptions should convert parent selection");
  expect_true(options.deletion == Population::DeletionMode::DeleteQuarter,
              "Configuration::toOptions should convert deletion mode");
  expect_true(options.fitness == Population::FitnessMode::LinearNormalized,
              "Configuration::toOptions should convert fitness mode");
  expect_true(options.variableLength == Population::VariableLengthMode::Variable,
              "Configuration::toOptions should convert variable-length mode");
  expect_true(round_trip.numberOfIndividuals == configuration.numberOfIndividuals,
              "Options round trip should preserve individual count");
  expect_true(round_trip.numberOfTrials == configuration.numberOfTrials,
              "Options round trip should preserve trial count");
  expect_true(round_trip.geneticDiversity == configuration.geneticDiversity,
              "Options round trip should preserve diversity");
  expect_true(round_trip.bitMutationRate == configuration.bitMutationRate,
              "Options round trip should preserve mutation rate");
  expect_true(round_trip.crossOverRate == configuration.crossOverRate,
              "Options round trip should preserve crossover rate");
  expect_true(round_trip.baseStates == configuration.baseStates,
              "Options round trip should preserve base state count");
  expect_true(round_trip.operation == configuration.operation,
              "Options round trip should preserve legacy operation");
  expect_true(round_trip.reproduction == configuration.reproduction,
              "Options round trip should preserve legacy reproduction");
  expect_true(round_trip.parentSelection == configuration.parentSelection,
              "Options round trip should preserve legacy parent selection");
  expect_true(round_trip.deletion == configuration.deletion,
              "Options round trip should preserve legacy deletion");
  expect_true(round_trip.fitness == configuration.fitness,
              "Options round trip should preserve legacy fitness");
  expect_true(round_trip.variableLength == configuration.variableLength,
              "Options round trip should preserve legacy variable-length setting");
}

void test_base_string_error_paths()
{
  SilentStderr silence;

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
    [&binary]() { binary.test(3); },
    "Out-of-range test should throw");
  expect_throws<GAFatalException>(
    [&binary]() { binary.set(3); },
    "Out-of-range set should throw");
  expect_throws<GAFatalException>(
    [&binary]() { binary.clear(3); },
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
  binary.set(0);
  binary.clear(1);
  binary.set(2);
  binary.clear(3);

  std::ostringstream bits_out;
  binary.printBits(bits_out);
  expect_true(bits_out.str() == "1010", "printBits should render the packed bit string");

  BaseString symbolic(3, 3);
  symbolic.set(0, 0);
  symbolic.set(1, 1);
  symbolic.set(2, 2);
  char zero[] = "a";
  char one[] = "b";
  char two[] = "c";
  char *values[] = { zero, one, two };
  std::ostringstream text_out;
  symbolic.print(values, text_out);
  expect_true(text_out.str() == "abc", "print should render symbolic values");
}

void test_exception_and_stringutil_helpers()
{
  SilentStderr silence;
  GAFatalException fatal(__FILE__, __LINE__, "fatal-reason");
  GANonFatalException nonfatal(__FILE__, __LINE__, "nonfatal-reason");
  GAComplete complete(__FILE__, __LINE__, "complete-reason");
  std::ostringstream out;

  out << fatal.what();
  fatal << out;

  expect_true(std::string(fatal.what()) == "fatal-reason", "Fatal exception should preserve its reason");
  expect_true(std::string(nonfatal.what()) == "nonfatal-reason", "Non-fatal exception should preserve its reason");
  expect_true(std::string(complete.what()) == "complete-reason", "Complete exception should preserve its reason");
  expect_true(out.str().find("fatal-reason") != std::string::npos, "Exception stream operator should print the reason");
  expect_true(tostring(42) == "42", "tostring should format integers");
}

void test_mutation_rate_zero_preserves_chromosome()
{
  Chromosome::seedRandom(1);
  Chromosome chromosome(makeBinaryString("10101100"));
  Chromosome original(makeBinaryString("10101100"));

  chromosome.SingleBitMutate(0.0);

  expect_true(chromosome.compare(&original), "Mutation rate 0 should leave chromosome unchanged");
}

void test_chromosome_constructor_and_compare_paths()
{
  SilentStderr silence;
  Chromosome::seedRandom(2);

  expect_throws<GAFatalException>(
    []() {
      Chromosome too_long(2049, 0, 2);
    },
    "Overlong chromosome constructor should throw");

  Chromosome left(makeBinaryString("1010"));
  Chromosome right(makeBinaryString("0101"));
  Chromosome symbolic(5, 0, 4);
  expect_true(!left.compare(&right), "compare should return false for different chromosomes");
  expect_true(!left.compare(&symbolic), "compare should return false for mismatched lengths");
  for (int i = 0 ; i < symbolic.ChromosomeLen() ; i++)
    {
      int value = symbolic.ChromosomeStr()->test(i);
      expect_true(value >= 0 && value < 4, "Non-binary constructor should initialize values in range");
    }

  std::ostringstream out;
  left.print(out);
  expect_true(!out.str().empty(), "print should write chromosome contents");
}

void test_invalid_mutation_probability_throws()
{
  SilentStderr silence;
  Chromosome::seedRandom(3);
  Chromosome chromosome(makeBinaryString("10101100"));

  expect_throws<GANonFatalException>(
    [&chromosome]() { chromosome.SingleBitMutate(2.0); },
    "Impossible mutation probability should throw a non-fatal exception");
}

void test_non_binary_mutation_with_probability_one_stays_in_range()
{
  Chromosome::seedRandom(4);
  Chromosome chromosome(new BaseString(5, 4), 0, 4);
  chromosome.SingleBitMutate(1.0);

  for (int i = 0 ; i < chromosome.ChromosomeLen() ; i++)
    {
      int value = chromosome.ChromosomeStr()->test(i);
      expect_true(value >= 0 && value < 4, "Non-binary mutation should keep values within range");
    }
}

void test_binary_mutation_with_probability_one_changes_only_bits()
{
  Chromosome::seedRandom(5);
  Chromosome chromosome(makeBinaryString("000000"));
  chromosome.SingleBitMutate(1.0);

  for (int i = 0 ; i < chromosome.ChromosomeLen() ; i++)
    {
      int value = chromosome.ChromosomeStr()->test(i);
      expect_true(value == 0 || value == 1, "Binary mutation should keep values binary");
    }
}

void test_mate_without_crossover_clones_parents()
{
  Chromosome::seedRandom(6);
  Chromosome mother(makeBinaryString("11110000"));
  Chromosome father(makeBinaryString("00001111"));
  Chromosome *son = 0;
  Chromosome *daughter = 0;

  mother.Mate(&father, &son, &daughter, 0.0, Chromosome::SinglePoint);

  expect_true(son != 0, "Mate should create a son");
  expect_true(daughter != 0, "Mate should create a daughter");
  expect_true(son->compare(&father), "Son should clone father when crossover rate is zero");
  expect_true(daughter->compare(&mother), "Daughter should clone mother when crossover rate is zero");

  delete son;
  delete daughter;
}

void test_uniform_crossover_is_reachable_and_safe()
{
  Chromosome::seedRandom(7);
  Chromosome mother(makeBinaryString("11110000"));
  Chromosome father(makeBinaryString("00001111"));

  for (int i = 0 ; i < 20 ; i++)
    {
      Chromosome *son = 0;
      Chromosome *daughter = 0;
      mother.Mate(&father, &son, &daughter, 1.0, Chromosome::Uniform);

      expect_true(son != 0, "Uniform crossover should produce a son");
      expect_true(daughter != 0, "Uniform crossover should produce a daughter");
      expect_true(son->ChromosomeLen() == father.ChromosomeLen(),
		  "Uniform crossover should preserve fixed-length son length");
      expect_true(daughter->ChromosomeLen() == mother.ChromosomeLen(),
		  "Uniform crossover should preserve fixed-length daughter length");

      delete son;
      delete daughter;
    }
}

void test_two_point_crossover_is_reachable_and_safe()
{
  Chromosome::seedRandom(8);
  Chromosome mother(makeBinaryString("11110000"));
  Chromosome father(makeBinaryString("00001111"));

  for (int i = 0 ; i < 20 ; i++)
    {
      Chromosome *son = 0;
      Chromosome *daughter = 0;
      mother.Mate(&father, &son, &daughter, 1.0, Chromosome::TwoPoint);

      expect_true(son != 0, "Two-point crossover should produce a son");
      expect_true(daughter != 0, "Two-point crossover should produce a daughter");
      expect_true(son->ChromosomeLen() == father.ChromosomeLen(),
		  "Fixed-length two-point crossover should preserve son length");
      expect_true(daughter->ChromosomeLen() == mother.ChromosomeLen(),
		  "Fixed-length two-point crossover should preserve daughter length");

      delete son;
      delete daughter;
    }
}

void test_mate_rejects_mismatched_fixed_lengths()
{
  SilentStderr silence;
  Chromosome::seedRandom(9);
  Chromosome mother(makeBinaryString("1111"));
  Chromosome father(makeBinaryString("00001111"));

  expect_throws<GAFatalException>(
    [&mother, &father]() {
      Chromosome *son = 0;
      Chromosome *daughter = 0;
      mother.Mate(&father, &son, &daughter, 1.0, Chromosome::SinglePoint);
    },
    "Fixed-length mating should reject mismatched parent lengths");
}

void test_variable_length_mating_supports_different_lengths()
{
  Chromosome::seedRandom(10);
  Chromosome mother(makeBinaryString("1111"), 1, 2);
  Chromosome father(makeBinaryString("000011"), 1, 2);

  for (int i = 0 ; i < 20 ; i++)
    {
      Chromosome *son = 0;
      Chromosome *daughter = 0;
      mother.Mate(&father, &son, &daughter, 1.0, Chromosome::SinglePoint);

      expect_true(son != 0, "Variable-length mating should produce a son");
      expect_true(daughter != 0, "Variable-length mating should produce a daughter");
      expect_true(son->ChromosomeLen() >= 1, "Variable-length son should have a valid length");
      expect_true(daughter->ChromosomeLen() >= 1, "Variable-length daughter should have a valid length");

      delete son;
      delete daughter;
    }
}

void test_variable_length_uniform_crossover()
{
  Chromosome::seedRandom(11);
  Chromosome mother(makeBinaryString("1111"), 1, 2);
  Chromosome father(makeBinaryString("000011"), 1, 2);

  Chromosome *son = 0;
  Chromosome *daughter = 0;
  mother.Mate(&father, &son, &daughter, 1.0, Chromosome::Uniform);

  expect_true(son != 0 && daughter != 0, "Variable-length uniform crossover should produce children");
  expect_true(son->ChromosomeLen() == father.ChromosomeLen(),
	      "Variable-length uniform crossover should preserve son length");
  expect_true(daughter->ChromosomeLen() == mother.ChromosomeLen(),
	      "Variable-length uniform crossover should preserve daughter length");

  delete son;
  delete daughter;
}

void test_variable_length_uniform_crossover_with_longer_mother()
{
  Chromosome::seedRandom(13);
  Chromosome mother(makeBinaryString("111100"), 1, 2);
  Chromosome father(makeBinaryString("0000"), 1, 2);

  Chromosome *son = 0;
  Chromosome *daughter = 0;
  mother.Mate(&father, &son, &daughter, 1.0, Chromosome::Uniform);

  expect_true(son != 0 && daughter != 0, "Variable-length uniform crossover should handle longer mothers");
  expect_true(son->ChromosomeLen() == father.ChromosomeLen(),
	      "Longer-mother uniform crossover should preserve son length");
  expect_true(daughter->ChromosomeLen() == mother.ChromosomeLen(),
	      "Longer-mother uniform crossover should preserve daughter length");

  delete son;
  delete daughter;
}

void test_invalid_crossover_type_throws()
{
  SilentStderr silence;
  Chromosome::seedRandom(12);
  Chromosome mother(makeBinaryString("1111"));
  Chromosome father(makeBinaryString("0000"));

  expect_throws<GANonFatalException>(
    [&mother, &father]() {
      Chromosome *son = 0;
      Chromosome *daughter = 0;
      mother.Mate(&father, &son, &daughter, 1.0,
		  static_cast<Chromosome::CrossOverType>(99));
    },
    "Unsupported crossover type should throw");
}

void test_population_decode()
{
  SilentStderr silence;
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 4, 4, 4, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  BaseString bits(4, 2);
  bits.set(0);
  bits.clear(1);
  bits.set(2);
  bits.set(3);

  expect_true(pop.decode(&bits, 0, 4) == 11, "Decode should interpret bits in big-endian order");
}

void test_population_random_helpers()
{
  SilentStderr silence;
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 4, 4, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));

  for (int i = 0 ; i < 20 ; i++)
    {
      int index = pop.randomIndex(3);
      expect_true(index >= 0 && index < 3, "randomIndex should stay within bounds");
    }

  for (int i = 0 ; i < 20 ; i++)
    {
      long value = pop.randomBelow(7);
      expect_true(value >= 0 && value < 7, "randomBelow should stay within bounds");
    }

  expect_throws<GAFatalException>(
    [&pop]() { pop.randomIndex(0); },
    "randomIndex should reject non-positive bounds");
  expect_throws<GAFatalException>(
    [&pop]() { pop.randomBelow(0); },
    "randomBelow should reject non-positive bounds");
}

void test_population_zero_total_selection_falls_back_to_uniform()
{
  SilentStderr silence;
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  pop.initializePopulation();
  double roulette[2] = {0.0, 0.0};
  int selected = -1;

  expect_true(pop.selectParent(&selected, roulette) != 0,
	      "Roulette selection should fall back to uniform choice when all weights are zero");
  expect_true(selected >= 0 && selected < pop.configuration().numberOfIndividuals,
	      "Uniform fallback selection should still return an in-range index");
}

void test_windowed_fitness_is_positive_for_maximize()
{
  SilentStderr silence;
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 3, 3, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Windowed,
						    Population::VariableLengthMode::Fixed, 2));

  pop.initializePopulation();
  for (int i = 0 ; i < 3 ; i++)
    {
      pop.populationTable[i].reset(new Chromosome(makeBinaryString(i == 0 ? "0" : "1")));
      pop.fitnessTable[i] = -1.0;
    }
  pop.populationInitialized = true;
  pop.evaluatePopulation();

  expect_true(pop.windowedFitnessTable[0] > 0.0, "Windowed fitness should stay positive");
  expect_true(pop.windowedFitnessTable[1] > 0.0, "Windowed fitness should stay positive for middle members");
  expect_true(pop.windowedFitnessTable[2] > pop.windowedFitnessTable[0],
	      "Best maximizing member should receive more windowed fitness than the worst");
}

void test_windowed_fitness_is_positive_for_minimize()
{
  SilentStderr silence;
  InspectablePopulation pop(make_population_options(Population::OperationMode::Minimize, 3, 3, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Windowed,
						    Population::VariableLengthMode::Fixed, 2));

  pop.initializePopulation();
  for (int i = 0 ; i < 3 ; i++)
    {
      pop.populationTable[i].reset(new Chromosome(makeBinaryString(i == 0 ? "0" : "1")));
      pop.fitnessTable[i] = -1.0;
    }
  pop.populationInitialized = true;
  pop.evaluatePopulation();

  expect_true(pop.windowedFitnessTable[0] > 0.0, "Minimizing windowed fitness should stay positive");
  expect_true(pop.windowedFitnessTable[0] > pop.windowedFitnessTable[2],
	      "Best minimizing member should receive more windowed fitness than the worst");
}

void test_fitness_table_selection()
{
  SilentStderr silence;
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

  expect_true(evaluation.selectFitnessWeights() == evaluation.fitnessTable.data(),
	      "FitnessIsEvaluation should select the raw fitness table");
  expect_true(windowed.selectFitnessWeights() == windowed.windowedFitnessTable.data(),
	      "WindowedFitness should select the windowed fitness table");
  expect_true(normalized.selectFitnessWeights() == normalized.linearNormalizedfitnessTable.data(),
	      "LinearNormalizedFitness should select the normalized fitness table");
}

void prepare_population(InspectablePopulation& pop, const std::string& first_bits,
			const std::string& second_bits)
{
  pop.initializePopulation();
  pop.populationTable[0].reset(new Chromosome(makeBinaryString(first_bits)));
  pop.populationTable[1].reset(new Chromosome(makeBinaryString(second_bits)));
  for (int i = 0 ; i < pop.configuration().numberOfIndividuals ; i++)
    {
      pop.fitnessTable[i] = -1.0;
    }
  pop.populationInitialized = true;
  pop.evaluatePopulation();
}

void test_population_selection_and_replacement_branches()
{
  SilentStderr silence;

  InspectablePopulation min_pop(make_population_options(Population::OperationMode::Minimize, 2, 2, 4, 0.0, 0.0,
							Population::ReproductionMode::AllowDuplicates,
							Population::ParentSelectionMode::RouletteWheel,
							Population::DeletionMode::DeleteAll,
							Population::FitnessMode::Evaluation,
							Population::VariableLengthMode::Fixed, 2));
  prepare_population(min_pop, "1111", "0000");
  int selected = -1;
  Chromosome *chosen = min_pop.selectParent(&selected, min_pop.selectFitnessWeights());
  expect_true(chosen != 0, "Minimize roulette selection should return a chromosome");
  expect_true(selected >= 0 && selected < min_pop.configuration().numberOfIndividuals,
	      "Minimize roulette selection should produce an in-range index");

  InspectablePopulation dup_not_allowed(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
								Population::ReproductionMode::DisallowDuplicates,
								Population::ParentSelectionMode::RouletteWheel,
								Population::DeletionMode::DeleteAll,
								Population::FitnessMode::Evaluation,
								Population::VariableLengthMode::Fixed, 2));
  prepare_population(dup_not_allowed, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > bred = dup_not_allowed.breedPopulation(1);
  expect_true(bred.size() == 1, "DuplicatesNotAllowed breeding should still generate requested children");
  expect_true(dup_not_allowed.containsChromosome(bred[0].get(), bred, 1),
	      "containsChromosome should report a present chromosome");

  std::vector<std::unique_ptr<Chromosome> > replacements_max;
  replacements_max.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("1111"))));
  replacements_max.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("0000"))));
  int replaced_max = dup_not_allowed.insertNewPopulation(std::move(replacements_max), 2);
  expect_true(replaced_max >= 0, "insertNewPopulation should handle duplicates-not-allowed maximize path");

  InspectablePopulation dup_not_allowed_min(make_population_options(Population::OperationMode::Minimize, 2, 2, 4, 0.0, 0.0,
								    Population::ReproductionMode::DisallowDuplicates,
								    Population::ParentSelectionMode::RouletteWheel,
								    Population::DeletionMode::DeleteAll,
								    Population::FitnessMode::Evaluation,
								    Population::VariableLengthMode::Fixed, 2));
  prepare_population(dup_not_allowed_min, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > replacements_min;
  replacements_min.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("1111"))));
  replacements_min.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("0000"))));
  int replaced_min = dup_not_allowed_min.insertNewPopulation(std::move(replacements_min), 2);
  expect_true(replaced_min >= 0, "insertNewPopulation should handle duplicates-not-allowed minimize path");

  InspectablePopulation dup_not_allowed_new(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
								    Population::ReproductionMode::DisallowDuplicates,
								    Population::ParentSelectionMode::RouletteWheel,
								    Population::DeletionMode::DeleteAll,
								    Population::FitnessMode::Evaluation,
								    Population::VariableLengthMode::Fixed, 2));
  prepare_population(dup_not_allowed_new, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > replacements_new;
  replacements_new.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("0011"))));
  replacements_new.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("1100"))));
  int replaced_new = dup_not_allowed_new.insertNewPopulation(std::move(replacements_new), 2);
  expect_true(replaced_new > 0, "Duplicates-not-allowed maximize path should insert new chromosomes");

  InspectablePopulation dup_not_allowed_new_min(make_population_options(Population::OperationMode::Minimize, 2, 2, 4, 0.0, 0.0,
										Population::ReproductionMode::DisallowDuplicates,
										Population::ParentSelectionMode::RouletteWheel,
										Population::DeletionMode::DeleteAll,
										Population::FitnessMode::Evaluation,
										Population::VariableLengthMode::Fixed, 2));
  prepare_population(dup_not_allowed_new_min, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > replacements_new_min;
  replacements_new_min.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("0011"))));
  replacements_new_min.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("1100"))));
  int replaced_new_min = dup_not_allowed_new_min.insertNewPopulation(std::move(replacements_new_min), 2);
  expect_true(replaced_new_min > 0, "Duplicates-not-allowed minimize path should insert new chromosomes");

}

void test_population_append_replacement_helper()
{
  SilentStderr silence;
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
						    Population::ReproductionMode::DisallowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));

  std::vector<std::unique_ptr<Chromosome> > replacement_list;
  int generated = 0;

  expect_true(pop.appendReplacement(replacement_list,
				    new Chromosome(makeBinaryString("1111")),
				    generated, 2, false),
	      "appendReplacement should keep the first unique child");
  expect_true(!pop.appendReplacement(replacement_list,
				     new Chromosome(makeBinaryString("1111")),
				     generated, 2, false),
	      "appendReplacement should reject duplicate unique-only children");
  expect_true(pop.appendReplacement(replacement_list,
				    new Chromosome(makeBinaryString("0000")),
				    generated, 2, false),
	      "appendReplacement should keep a second unique child");
  expect_true(!pop.appendReplacement(replacement_list,
				     new Chromosome(makeBinaryString("0011")),
				     generated, 2, true),
	      "appendReplacement should delete overflow children when the replacement list is full");
}

void test_population_insert_new_population_rejects_overflow()
{
  SilentStderr silence;
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  prepare_population(pop, "1111", "0000");

  std::vector<std::unique_ptr<Chromosome> > replacements;
  replacements.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("0011"))));
  replacements.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("1100"))));
  replacements.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("0101"))));

  expect_throws<GAFatalException>(
    [&pop, &replacements]() mutable {
      pop.insertNewPopulation(std::move(replacements), 3);
    },
    "insertNewPopulation should reject replacement counts larger than the population");
}

void test_population_selection_guard_and_random_fitness_modes()
{
  SilentStderr silence;

  InspectablePopulation fractional(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  fractional.initializePopulation();
  int selected = -1;
  double roulette[2] = {0.4, 0.4};
  expect_throws<GAFatalException>(
    [&fractional, &selected, &roulette]() { fractional.selectParent(&selected, roulette); },
    "Roulette selection should throw when rounded weights produce an invalid index");

  InspectablePopulation windowed(make_population_options(Population::OperationMode::Maximize, 4, 4, 1, 0.0, 0.0,
							 Population::ReproductionMode::AllowDuplicates,
							 Population::ParentSelectionMode::RouletteWheel,
							 Population::DeletionMode::DeleteAll,
							 Population::FitnessMode::Windowed,
							 Population::VariableLengthMode::Fixed, 2));
  prepare_population(windowed, "1", "0");
  selected = -1;
  expect_true(windowed.selectParent(&selected, windowed.selectFitnessWeights()) != 0,
	      "Windowed fitness selection should return a chromosome");

  InspectablePopulation normalized(make_population_options(Population::OperationMode::Maximize, 4, 4, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::LinearNormalized,
							   Population::VariableLengthMode::Fixed, 2));
  prepare_population(normalized, "1", "0");
  selected = -1;
  expect_true(normalized.selectParent(&selected, normalized.selectFitnessWeights()) != 0,
	      "Linear normalized fitness selection should return a chromosome");

  InspectablePopulation random_selection(make_population_options(Population::OperationMode::Maximize, 2, 4, 4, 0.0, 0.0,
								 Population::ReproductionMode::AllowDuplicates,
								 Population::ParentSelectionMode::Random,
								 Population::DeletionMode::DeleteAll,
								 Population::FitnessMode::Evaluation,
								 Population::VariableLengthMode::Fixed, 2));
  prepare_population(random_selection, "1111", "0000");
  std::vector<std::unique_ptr<Chromosome> > random_bred = random_selection.breedPopulation(2);
  expect_true(random_bred.size() == 2,
	      "Random parent selection should generate the requested number of children");
}

void test_population_invalid_enum_paths()
{
  SilentStderr silence;

  InspectablePopulation bad_delete(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  bad_delete.config_.deletion = static_cast<Population::DeletionTechnique>(99);
  expect_throws<GAFatalException>(
    [&bad_delete]() { bad_delete.run(); },
    "Unsupported deletion technique should throw");

  InspectablePopulation bad_fitness(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
							    Population::ReproductionMode::AllowDuplicates,
							    Population::ParentSelectionMode::RouletteWheel,
							    Population::DeletionMode::DeleteAll,
							    Population::FitnessMode::Evaluation,
							    Population::VariableLengthMode::Fixed, 2));
  bad_fitness.config_.fitness = static_cast<Population::FitnessTechnique>(99);
  expect_throws<GAFatalException>(
    [&bad_fitness]() { bad_fitness.selectFitnessWeights(); },
    "Unsupported fitness technique should throw");

  InspectablePopulation bad_parent(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  bad_parent.config_.parentSelection = static_cast<Population::ParentSelectionTechnique>(99);
  prepare_population(bad_parent, "1111", "0000");
  expect_throws<GAFatalException>(
    [&bad_parent]() { bad_parent.breedPopulation(1); },
    "Unsupported parent selection technique should throw");

  InspectablePopulation bad_operation(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
							      Population::ReproductionMode::AllowDuplicates,
							      Population::ParentSelectionMode::RouletteWheel,
							      Population::DeletionMode::DeleteAll,
							      Population::FitnessMode::Evaluation,
							      Population::VariableLengthMode::Fixed, 2));
  prepare_population(bad_operation, "1111", "0000");
  bad_operation.config_.operation = static_cast<Population::OperationTechnique>(99);
  expect_throws<GAFatalException>(
    [&bad_operation]() {
      int selected = -1;
      bad_operation.selectParent(&selected, bad_operation.selectFitnessWeights());
    },
    "Unsupported operation technique in selection should throw");

  std::vector<std::unique_ptr<Chromosome> > replacements;
  replacements.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("1111"))));
  expect_throws<GAFatalException>(
    [&bad_operation, &replacements]() mutable {
      bad_operation.insertNewPopulation(std::move(replacements), 1);
    },
    "Unsupported operation technique in replacement should throw");

  InspectablePopulation bad_operation_dup(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
								  Population::ReproductionMode::DisallowDuplicates,
								  Population::ParentSelectionMode::RouletteWheel,
								  Population::DeletionMode::DeleteAll,
								  Population::FitnessMode::Evaluation,
								  Population::VariableLengthMode::Fixed, 2));
  prepare_population(bad_operation_dup, "1111", "0000");
  bad_operation_dup.config_.operation = static_cast<Population::OperationTechnique>(99);
  std::vector<std::unique_ptr<Chromosome> > dup_replacements;
  dup_replacements.push_back(std::unique_ptr<Chromosome>(new Chromosome(makeBinaryString("0011"))));
  expect_throws<GAFatalException>(
    [&bad_operation_dup, &dup_replacements]() mutable {
      bad_operation_dup.insertNewPopulation(std::move(dup_replacements), 1);
    },
    "Unsupported operation technique in duplicate-filtered replacement should throw");
}

void test_population_verbose_path()
{
  setenv("GAK_VERBOSE", "1", 1);
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  pop.run();
  unsetenv("GAK_VERBOSE");
  expect_true(pop.populationInitialized, "Verbose run path should still complete successfully");
}

void test_population_verbose_minimize_path()
{
  setenv("GAK_VERBOSE", "1", 1);
  InspectablePopulation pop(make_population_options(Population::OperationMode::Minimize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  pop.run();
  unsetenv("GAK_VERBOSE");
  expect_true(pop.populationInitialized, "Verbose minimize path should still complete successfully");
}

void test_population_run_modes()
{
  SilentStderr silence;

  InspectablePopulation delete_all(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  delete_all.run();

  InspectablePopulation delete_half(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
							    Population::ReproductionMode::AllowDuplicates,
							    Population::ParentSelectionMode::RouletteWheel,
							    Population::DeletionMode::DeleteHalf,
							    Population::FitnessMode::Evaluation,
							    Population::VariableLengthMode::Fixed, 2));
  delete_half.run();

  InspectablePopulation delete_quarter(make_population_options(Population::OperationMode::Maximize, 8, 10, 1, 0.0, 0.0,
							       Population::ReproductionMode::AllowDuplicates,
							       Population::ParentSelectionMode::RouletteWheel,
							       Population::DeletionMode::DeleteQuarter,
							       Population::FitnessMode::Evaluation,
							       Population::VariableLengthMode::Fixed, 2));
  delete_quarter.run();

  InspectablePopulation delete_last_min(make_population_options(Population::OperationMode::Minimize, 6, 8, 1, 0.0, 0.0,
								Population::ReproductionMode::AllowDuplicates,
								Population::ParentSelectionMode::RouletteWheel,
								Population::DeletionMode::DeleteLast,
								Population::FitnessMode::Evaluation,
								Population::VariableLengthMode::Fixed, 2));
  delete_last_min.run();

  expect_true(delete_last_min.populationInitialized, "Additional run modes should complete successfully");
}

void test_population_execute_returns_structured_result()
{
  SilentStderr silence;

  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  Population::RunResult result = pop.execute(true);

  expect_true(result.generationsCompleted >= 1, "execute should report completed generations");
  expect_true(result.evaluations >= pop.configuration().numberOfIndividuals,
	      "execute should report total evaluations");
  expect_true(!result.generationSummaries.empty(),
	      "execute(true) should capture per-generation summaries");
  expect_true(!result.finalSummary.mostFit.empty(),
	      "execute should capture the final most-fit summary");
  expect_true(!result.finalSummary.leastFit.empty(),
	      "execute should capture the final least-fit summary");
}

void test_delete_all_but_best_runs()
{
  InspectablePopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 20, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAllButBest,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  SilentStderr silence;
  pop.run();

  expect_true(pop.populationInitialized == true, "Population should initialize during run");
  expect_true(!pop.populationTable.empty(), "Population should retain its table after run");
  expect_true(pop.populationTable[pop.configuration().numberOfIndividuals - 1].get()->ChromosomeStr()->test(0) == 1,
	      "Best chromosome should remain present after DeleteAllButBest runs");
}

}

int main()
{
  test_base_string();
  test_population_options_round_trip();
  test_base_string_error_paths();
  test_base_string_print_helpers();
  test_exception_and_stringutil_helpers();
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
  test_population_decode();
  test_population_random_helpers();
  test_population_zero_total_selection_falls_back_to_uniform();
  test_windowed_fitness_is_positive_for_maximize();
  test_windowed_fitness_is_positive_for_minimize();
  test_fitness_table_selection();
  test_population_selection_and_replacement_branches();
  test_population_append_replacement_helper();
  test_population_insert_new_population_rejects_overflow();
  test_population_selection_guard_and_random_fitness_modes();
  test_population_invalid_enum_paths();
  test_population_verbose_path();
  test_population_verbose_minimize_path();
  test_population_run_modes();
  test_population_execute_returns_structured_result();
  test_delete_all_but_best_runs();

  if (g_failures != 0)
    {
      std::cerr << g_failures << " test(s) failed" << std::endl;
      return 1;
    }

  std::cout << "All tests passed" << std::endl;
  return 0;
}
