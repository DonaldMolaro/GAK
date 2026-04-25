#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "base.hh"
#include "chromosome.hh"
#include "except.hh"

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

class CaptureStderr
{
public:
  CaptureStderr()
    : saved_fd_(-1),
      capture_(tmpfile())
  {
    fflush(stderr);
    saved_fd_ = dup(fileno(stderr));
    if (saved_fd_ >= 0 && capture_ != NULL)
      {
        dup2(fileno(capture_), fileno(stderr));
      }
  }

  ~CaptureStderr()
  {
    if (saved_fd_ >= 0)
      {
        fflush(stderr);
        dup2(saved_fd_, fileno(stderr));
        close(saved_fd_);
      }
    if (capture_ != NULL)
      {
        fclose(capture_);
      }
  }

  std::string str()
  {
    if (capture_ == NULL)
      {
        return "";
      }

    fflush(stderr);
    long current = ftell(capture_);
    rewind(capture_);

    std::string output;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), capture_) != NULL)
      {
        output += buffer;
      }

    if (current >= 0)
      {
        fseek(capture_, current, SEEK_SET);
      }
    return output;
  }

private:
  int saved_fd_;
  FILE *capture_;
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

Population::Settings make_population_options(Population::OperationMode operation,
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
  explicit InspectablePopulation(const Settings& settings)
    : Population(settings)
  {
  }

  double evaluateFitness(const BaseString& b) override
  {
    return b.test(0);
  }

  void printCandidate(const BaseString&, std::ostream&) override
  {
  }
};

class DefaultHookPopulation : public Population
{
public:
  explicit DefaultHookPopulation(const Settings& settings)
    : Population(settings)
  {
  }

  using Population::createInitialChromosome;
  using Population::mateChromosomes;
  using Population::mutateChromosome;

  double evaluateFitness(const BaseString& b) override
  {
    return b.test(0);
  }

  void printCandidate(const BaseString& b, std::ostream& out) override
  {
    out << "Chromosome:" << b.test(0) << '\n';
  }
};

Population::Settings make_population_options(Population::OperationMode operation,
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
  Population::Settings options;
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
  Population::Settings settings;
  settings.operation = Population::OperationMode::Minimize;
  settings.numberOfIndividuals = 17;
  settings.numberOfTrials = 99;
  settings.geneticDiversity = 23;
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
  expect_true(settings.geneticDiversity == 23,
              "Settings should preserve diversity");
  expect_true(settings.baseStates == 7,
              "Settings should preserve base state count");
  expect_true(settings.useFixedRandomSeed,
              "Settings should preserve fixed-seed mode");
  expect_true(settings.randomSeed == 4242U,
              "Settings should preserve the configured random seed");
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

void test_exception_helpers()
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
}

void test_mutation_rate_zero_preserves_chromosome()
{
  std::mt19937 random_generator(1);
  Chromosome chromosome(std::unique_ptr<BaseString>(makeBinaryString("10101100")));
  Chromosome original(std::unique_ptr<BaseString>(makeBinaryString("10101100")));

  chromosome.mutate(0.0, &random_generator);

  expect_true(chromosome.equals(original), "Mutation rate 0 should leave chromosome unchanged");
}

void test_chromosome_constructor_and_compare_paths()
{
  SilentStderr silence;
  std::mt19937 random_generator(2);

  expect_throws<GAFatalException>(
    []() {
      Chromosome too_long(2049, 0, 2);
    },
    "Overlong chromosome constructor should throw");

  Chromosome left(std::unique_ptr<BaseString>(makeBinaryString("1010")));
  Chromosome right(std::unique_ptr<BaseString>(makeBinaryString("0101")));
  Chromosome symbolic(5, 0, 4, &random_generator);
  expect_true(!left.equals(right), "equals should return false for different chromosomes");
  expect_true(!left.equals(symbolic), "equals should return false for mismatched lengths");
  for (int i = 0 ; i < symbolic.length() ; i++)
    {
      int value = symbolic.genes().test(i);
      expect_true(value >= 0 && value < 4, "Non-binary constructor should initialize values in range");
    }

  std::ostringstream out;
  left.print(out);
  expect_true(!out.str().empty(), "print should write chromosome contents");
}

void test_invalid_mutation_probability_throws()
{
  SilentStderr silence;
  std::mt19937 random_generator(3);
  Chromosome chromosome(std::unique_ptr<BaseString>(makeBinaryString("10101100")));

  expect_throws<GANonFatalException>(
    [&chromosome, &random_generator]() { chromosome.mutate(2.0, &random_generator); },
    "Impossible mutation probability should throw a non-fatal exception");
}

void test_non_binary_mutation_with_probability_one_stays_in_range()
{
  std::mt19937 random_generator(4);
  Chromosome chromosome(std::make_unique<BaseString>(5, 4), 0, 4);
  chromosome.mutate(1.0, &random_generator);

  for (int i = 0 ; i < chromosome.length() ; i++)
    {
      int value = chromosome.genes().test(i);
      expect_true(value >= 0 && value < 4, "Non-binary mutation should keep values within range");
    }
}

void test_binary_mutation_with_probability_one_changes_only_bits()
{
  std::mt19937 random_generator(5);
  Chromosome chromosome(std::unique_ptr<BaseString>(makeBinaryString("000000")));
  chromosome.mutate(1.0, &random_generator);

  for (int i = 0 ; i < chromosome.length() ; i++)
    {
      int value = chromosome.genes().test(i);
      expect_true(value == 0 || value == 1, "Binary mutation should keep values binary");
    }
}

void test_mate_without_crossover_clones_parents()
{
  std::mt19937 random_generator(6);
  Chromosome mother(std::unique_ptr<BaseString>(makeBinaryString("11110000")));
  Chromosome father(std::unique_ptr<BaseString>(makeBinaryString("00001111")));
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
  Chromosome mother(std::unique_ptr<BaseString>(makeBinaryString("11110000")));
  Chromosome father(std::unique_ptr<BaseString>(makeBinaryString("00001111")));

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
  Chromosome mother(std::unique_ptr<BaseString>(makeBinaryString("11110000")));
  Chromosome father(std::unique_ptr<BaseString>(makeBinaryString("00001111")));

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
  SilentStderr silence;
  std::mt19937 random_generator(9);
  Chromosome mother(std::unique_ptr<BaseString>(makeBinaryString("1111")));
  Chromosome father(std::unique_ptr<BaseString>(makeBinaryString("00001111")));

  expect_throws<GAFatalException>(
    [&mother, &father, &random_generator]() {
      mother.mate(father, 1.0, Chromosome::CrossoverType::SinglePoint, &random_generator);
    },
    "Fixed-length mating should reject mismatched parent lengths");
}

void test_variable_length_mating_supports_different_lengths()
{
  std::mt19937 random_generator(10);
  Chromosome mother(std::unique_ptr<BaseString>(makeBinaryString("1111")), 1, 2);
  Chromosome father(std::unique_ptr<BaseString>(makeBinaryString("000011")), 1, 2);

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
  Chromosome mother(std::unique_ptr<BaseString>(makeBinaryString("1111")), 1, 2);
  Chromosome father(std::unique_ptr<BaseString>(makeBinaryString("000011")), 1, 2);

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
  Chromosome mother(std::unique_ptr<BaseString>(makeBinaryString("111100")), 1, 2);
  Chromosome father(std::unique_ptr<BaseString>(makeBinaryString("0000")), 1, 2);

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
  SilentStderr silence;
  std::mt19937 random_generator(12);
  Chromosome mother(std::unique_ptr<BaseString>(makeBinaryString("1111")));
  Chromosome father(std::unique_ptr<BaseString>(makeBinaryString("0000")));

  expect_throws<GANonFatalException>(
    [&mother, &father, &random_generator]() {
      mother.mate(father, 1.0, static_cast<Chromosome::CrossoverType>(99), &random_generator);
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

  expect_true(pop.decode(bits, 0, 4) == 11, "Decode should interpret bits in big-endian order");
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
  expect_true(selected >= 0 && selected < pop.settings().numberOfIndividuals,
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
      pop.populationTable[i].reset(new Chromosome(std::unique_ptr<BaseString>(makeBinaryString(i == 0 ? "0" : "1"))));
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
      pop.populationTable[i].reset(new Chromosome(std::unique_ptr<BaseString>(makeBinaryString(i == 0 ? "0" : "1"))));
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
  pop.populationTable[0].reset(new Chromosome(std::unique_ptr<BaseString>(makeBinaryString(first_bits))));
  pop.populationTable[1].reset(new Chromosome(std::unique_ptr<BaseString>(makeBinaryString(second_bits))));
  for (int i = 0 ; i < pop.settings().numberOfIndividuals ; i++)
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
  expect_true(selected >= 0 && selected < min_pop.settings().numberOfIndividuals,
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
  replacements_max.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("1111"))));
  replacements_max.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("0000"))));
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
  replacements_min.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("1111"))));
  replacements_min.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("0000"))));
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
  replacements_new.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("0011"))));
  replacements_new.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("1100"))));
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
  replacements_new_min.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("0011"))));
  replacements_new_min.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("1100"))));
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
				    new Chromosome(std::unique_ptr<BaseString>(makeBinaryString("1111"))),
				    generated, 2, false),
	      "appendReplacement should keep the first unique child");
  expect_true(!pop.appendReplacement(replacement_list,
				     new Chromosome(std::unique_ptr<BaseString>(makeBinaryString("1111"))),
				     generated, 2, false),
	      "appendReplacement should reject duplicate unique-only children");
  expect_true(pop.appendReplacement(replacement_list,
				    new Chromosome(std::unique_ptr<BaseString>(makeBinaryString("0000"))),
				    generated, 2, false),
	      "appendReplacement should keep a second unique child");
  expect_true(!pop.appendReplacement(replacement_list,
				     new Chromosome(std::unique_ptr<BaseString>(makeBinaryString("0011"))),
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
  replacements.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("0011"))));
  replacements.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("1100"))));
  replacements.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("0101"))));

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
  bad_delete.settings_.deletion = static_cast<Population::DeletionMode>(99);
  expect_throws<GAFatalException>(
    [&bad_delete]() { bad_delete.run(); },
    "Unsupported deletion technique should throw");

  InspectablePopulation bad_fitness(make_population_options(Population::OperationMode::Maximize, 2, 2, 1, 0.0, 0.0,
							    Population::ReproductionMode::AllowDuplicates,
							    Population::ParentSelectionMode::RouletteWheel,
							    Population::DeletionMode::DeleteAll,
							    Population::FitnessMode::Evaluation,
							    Population::VariableLengthMode::Fixed, 2));
  bad_fitness.settings_.fitness = static_cast<Population::FitnessMode>(99);
  expect_throws<GAFatalException>(
    [&bad_fitness]() { bad_fitness.selectFitnessWeights(); },
    "Unsupported fitness technique should throw");

  InspectablePopulation bad_parent(make_population_options(Population::OperationMode::Maximize, 2, 2, 4, 0.0, 0.0,
							   Population::ReproductionMode::AllowDuplicates,
							   Population::ParentSelectionMode::RouletteWheel,
							   Population::DeletionMode::DeleteAll,
							   Population::FitnessMode::Evaluation,
							   Population::VariableLengthMode::Fixed, 2));
  bad_parent.settings_.parentSelection = static_cast<Population::ParentSelectionMode>(99);
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
  bad_operation.settings_.operation = static_cast<Population::OperationMode>(99);
  expect_throws<GAFatalException>(
    [&bad_operation]() {
      int selected = -1;
      bad_operation.selectParent(&selected, bad_operation.selectFitnessWeights());
    },
    "Unsupported operation technique in selection should throw");

  std::vector<std::unique_ptr<Chromosome> > replacements;
  replacements.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("1111"))));
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
  bad_operation_dup.settings_.operation = static_cast<Population::OperationMode>(99);
  std::vector<std::unique_ptr<Chromosome> > dup_replacements;
  dup_replacements.push_back(std::make_unique<Chromosome>(std::unique_ptr<BaseString>(makeBinaryString("0011"))));
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
  SilentStderr silence;

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
  SilentStderr silence;

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

void test_population_run_output_contains_progress_and_final_summary()
{
  DefaultHookPopulation pop(make_population_options(Population::OperationMode::Maximize, 6, 8, 1, 0.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));
  CaptureStderr capture;
  pop.run();
  std::string output = capture.str();

  expect_true(output.find("Generation 1 Number of Evaluations 12") != std::string::npos,
	      "run should print final generation progress");
  expect_true(output.find("Chromosome:") != std::string::npos,
	      "run should print chromosome summaries through FitnessPrint");
  expect_true(output.find("1.000000") != std::string::npos || output.find("0.000000") != std::string::npos,
	      "run should print final summary fitness values");
}

void test_population_default_operator_hooks_are_explicitly_exercised()
{
  DefaultHookPopulation pop(make_population_options(Population::OperationMode::Maximize, 4, 4, 6, 1.0, 0.0,
						    Population::ReproductionMode::AllowDuplicates,
						    Population::ParentSelectionMode::RouletteWheel,
						    Population::DeletionMode::DeleteAll,
						    Population::FitnessMode::Evaluation,
						    Population::VariableLengthMode::Fixed, 2));

  std::unique_ptr<Chromosome> initial = pop.createInitialChromosome();
  expect_true(initial.get() != NULL, "Default createInitialChromosome should produce a chromosome");
  expect_true(initial->length() == pop.settings().geneticDiversity,
	      "Default createInitialChromosome should use the configured diversity as chromosome length");
  for (int i = 0 ; i < initial->length() ; i++)
    {
      int value = initial->genes().test(i);
      expect_true(value == 0 || value == 1,
		  "Default createInitialChromosome should respect the configured base state range");
    }

  std::unique_ptr<Chromosome> mother(new Chromosome(std::unique_ptr<BaseString>(makeBinaryString("111100"))));
  std::unique_ptr<Chromosome> father(new Chromosome(std::unique_ptr<BaseString>(makeBinaryString("000011"))));
  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
    pop.mateChromosomes(*mother, *father);

  expect_true(children.first.get() != NULL && children.second.get() != NULL,
	      "Default mateChromosomes should produce two children");
  expect_true(children.first->equals(*father),
	      "Default mateChromosomes should clone the father into the first child when crossover is disabled");
  expect_true(children.second->equals(*mother),
	      "Default mateChromosomes should clone the mother into the second child when crossover is disabled");

  pop.mutateChromosome(*children.first);
  for (int i = 0 ; i < children.first->length() ; i++)
    {
      int value = children.first->genes().test(i);
      expect_true(value == 0 || value == 1,
		  "Default mutateChromosome should keep binary genes in range");
    }
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
  expect_true(pop.populationTable[pop.settings().numberOfIndividuals - 1].get()->genes().test(0) == 1,
	      "Best chromosome should remain present after DeleteAllButBest runs");
}

}

int main()
{
  test_base_string();
  test_population_options_round_trip();
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
  test_population_execute_result_contents_are_consistent();
  test_population_fixed_seed_makes_runs_reproducible();
  test_population_run_output_contains_progress_and_final_summary();
  test_population_default_operator_hooks_are_explicitly_exercised();
  test_delete_all_but_best_runs();

  if (g_failures != 0)
    {
      std::cerr << g_failures << " test(s) failed" << std::endl;
      return 1;
    }

  std::cout << "All tests passed" << std::endl;
  return 0;
}
