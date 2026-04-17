#include <cstdio>
#include <cstdlib>
#include <iostream>
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

class InspectablePopulation : public Population
{
public:
  InspectablePopulation(OperationTechnique op,
			int individuals,
			int trials,
			int diversity,
			double mutation,
			double crossover,
			ReproductionTechniques reproduction,
			ParrentSelectionTechnique selection,
			DeletetionTechnique deletion,
			FitnessTechnique fitness,
			VariableLength variable,
			int states)
    : Population(op, individuals, trials, diversity, mutation, crossover,
		 reproduction, selection, deletion, fitness, variable, states)
  {
  }

  virtual double FitnessFunction(BaseString *b)
  {
    return b->test(0);
  }

  virtual void FitnessPrint(BaseString *)
  {
  }
};

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

  expect_true(symbolic.test(0) == 4, "Symbolic base should preserve assigned values");
  expect_true(symbolic.test(1) == 2, "Symbolic base should preserve non-binary states");
  expect_true(symbolic.test(2) == 0, "Clear should reset symbolic base to zero");
}

void test_base_string_error_paths()
{
  SilentStderr silence;

  expect_throws<GAFatalException>(
    []() {
      BaseString invalid(0, 2);
    },
    "Invalid BaseString constructor arguments should throw");

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
  Chromosome chromosome(makeBinaryString("10101100"));
  Chromosome original(makeBinaryString("10101100"));

  chromosome.SingleBitMutate(0.0);

  expect_true(chromosome.compare(&original), "Mutation rate 0 should leave chromosome unchanged");
}

void test_invalid_mutation_probability_throws()
{
  SilentStderr silence;
  Chromosome chromosome(makeBinaryString("10101100"));

  expect_throws<GANonFatalException>(
    [&chromosome]() { chromosome.SingleBitMutate(2.0); },
    "Impossible mutation probability should throw a non-fatal exception");
}

void test_mate_without_crossover_clones_parents()
{
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

void test_population_decode()
{
  SilentStderr silence;
  InspectablePopulation pop(Population::Maximize, 4, 4, 4, 0.0, 0.0,
			    Population::DuplicatesAllowed, Population::RouletteWheel,
			    Population::DeleteAll, Population::FitnessIsEvaluation,
			    Population::VariableLengthNotPermitted, 2);
  BaseString bits(4, 2);
  bits.set(0);
  bits.clear(1);
  bits.set(2);
  bits.set(3);

  expect_true(pop.decode(&bits, 0, 4) == 11, "Decode should interpret bits in big-endian order");
}

void test_windowed_fitness_is_positive_for_maximize()
{
  SilentStderr silence;
  InspectablePopulation pop(Population::Maximize, 3, 3, 1, 0.0, 0.0,
			    Population::DuplicatesAllowed, Population::RouletteWheel,
			    Population::DeleteAll, Population::WindowedFitness,
			    Population::VariableLengthNotPermitted, 2);

  pop.initializePopulation();
  for (int i = 0 ; i < 3 ; i++)
    {
      delete pop.populationTable[i];
      pop.populationTable[i] = new Chromosome(makeBinaryString(i == 0 ? "0" : "1"));
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
  InspectablePopulation pop(Population::Minimize, 3, 3, 1, 0.0, 0.0,
			    Population::DuplicatesAllowed, Population::RouletteWheel,
			    Population::DeleteAll, Population::WindowedFitness,
			    Population::VariableLengthNotPermitted, 2);

  pop.initializePopulation();
  for (int i = 0 ; i < 3 ; i++)
    {
      delete pop.populationTable[i];
      pop.populationTable[i] = new Chromosome(makeBinaryString(i == 0 ? "0" : "1"));
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
  InspectablePopulation evaluation(Population::Maximize, 3, 3, 1, 0.0, 0.0,
				   Population::DuplicatesAllowed, Population::RouletteWheel,
				   Population::DeleteAll, Population::FitnessIsEvaluation,
				   Population::VariableLengthNotPermitted, 2);
  InspectablePopulation windowed(Population::Maximize, 3, 3, 1, 0.0, 0.0,
				 Population::DuplicatesAllowed, Population::RouletteWheel,
				 Population::DeleteAll, Population::WindowedFitness,
				 Population::VariableLengthNotPermitted, 2);
  InspectablePopulation normalized(Population::Maximize, 3, 3, 1, 0.0, 0.0,
				   Population::DuplicatesAllowed, Population::RouletteWheel,
				   Population::DeleteAll, Population::LinearNormalizedFitness,
				   Population::VariableLengthNotPermitted, 2);

  expect_true(evaluation.selectFitnessTable() == evaluation.fitnessTable,
	      "FitnessIsEvaluation should select the raw fitness table");
  expect_true(windowed.selectFitnessTable() == windowed.windowedFitnessTable,
	      "WindowedFitness should select the windowed fitness table");
  expect_true(normalized.selectFitnessTable() == normalized.linearNormalizedfitnessTable,
	      "LinearNormalizedFitness should select the normalized fitness table");
}

void test_delete_all_but_best_runs()
{
  InspectablePopulation pop(Population::Maximize, 6, 20, 1, 0.0, 0.0,
			    Population::DuplicatesAllowed, Population::RouletteWheel,
			    Population::DeleteAllButBest, Population::FitnessIsEvaluation,
			    Population::VariableLengthNotPermitted, 2);
  SilentStderr silence;
  pop.run();

  expect_true(pop.populationInitialized == true, "Population should initialize during run");
  expect_true(pop.populationTable != 0, "Population should retain its table after run");
  expect_true(pop.populationTable[pop.numberofIndividuals - 1]->ChromosomeStr()->test(0) == 1,
	      "Best chromosome should remain present after DeleteAllButBest runs");
}

}

int main()
{
  test_base_string();
  test_base_string_error_paths();
  test_exception_and_stringutil_helpers();
  test_mutation_rate_zero_preserves_chromosome();
  test_invalid_mutation_probability_throws();
  test_mate_without_crossover_clones_parents();
  test_uniform_crossover_is_reachable_and_safe();
  test_two_point_crossover_is_reachable_and_safe();
  test_mate_rejects_mismatched_fixed_lengths();
  test_variable_length_mating_supports_different_lengths();
  test_population_decode();
  test_windowed_fitness_is_positive_for_maximize();
  test_windowed_fitness_is_positive_for_minimize();
  test_fitness_table_selection();
  test_delete_all_but_best_runs();

  if (g_failures != 0)
    {
      std::cerr << g_failures << " test(s) failed" << std::endl;
      return 1;
    }

  std::cout << "All tests passed" << std::endl;
  return 0;
}
