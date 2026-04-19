#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

#include "base.hh"
#include "chromosome.hh"
#include "except.hh"
#include "population.hh"
#include "alpha.hh"
#include "dome.hh"
#include "f6.hh"
#include "knapsack.hh"
#include "latinsquare.hh"
#include "nqueens.hh"
#include "sudoku.hh"
#include "sudoku_constrained.hh"
#define private public
#include "ts.hh"
#undef private
#include "spell.hh"

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

class SilentOutput
{
public:
  SilentOutput()
    : saved_stdout_(-1),
      saved_stderr_(-1),
      null_(NULL)
  {
    fflush(stdout);
    fflush(stderr);
    saved_stdout_ = dup(fileno(stdout));
    saved_stderr_ = dup(fileno(stderr));
    null_ = fopen("/dev/null", "w");
    if (null_ != NULL)
      {
        dup2(fileno(null_), fileno(stdout));
        dup2(fileno(null_), fileno(stderr));
      }
  }

  ~SilentOutput()
  {
    fflush(stdout);
    fflush(stderr);
    if (saved_stdout_ >= 0)
      {
        dup2(saved_stdout_, fileno(stdout));
        close(saved_stdout_);
      }
    if (saved_stderr_ >= 0)
      {
        dup2(saved_stderr_, fileno(stderr));
        close(saved_stderr_);
      }
    if (null_ != NULL)
      {
        fclose(null_);
      }
  }

private:
  int saved_stdout_;
  int saved_stderr_;
  FILE *null_;
};

class InspectableSudokuConstrained : public SudokuConstrained
{
public:
  explicit InspectableSudokuConstrained(const Population::Options& options)
    : SudokuConstrained(options)
  {
  }

  using SudokuConstrained::createInitialChromosome;
  using SudokuConstrained::mateChromosomes;
  using SudokuConstrained::mutateChromosome;
};

BaseString *makeBinaryString(const std::string& bits)
{
  BaseString *b = new BaseString(bits.length(), 2);
  for (int i = 0 ; i < static_cast<int>(bits.length()) ; i++)
    {
      if (bits[i] == '1')
        {
          b->set(i);
        }
      else
        {
          b->clear(i);
        }
    }
  return b;
}

BaseString *makeSymbolicString(const std::string& letters)
{
  BaseString *b = new BaseString(letters.length(), 26);
  for (int i = 0 ; i < static_cast<int>(letters.length()) ; i++)
    {
      b->set(i, letters[i] - 'a');
    }
  return b;
}

Population::Options make_options(Population::OperationMode operation,
                                 int diversity,
                                 Population::VariableLengthMode variable_length,
                                 int base_states)
{
  Population::Options options;
  options.operation = operation;
  options.numberOfIndividuals = 6;
  options.numberOfTrials = 12;
  options.geneticDiversity = diversity;
  options.bitMutationRate = 0.0;
  options.crossOverRate = 0.0;
  options.reproduction = Population::ReproductionMode::AllowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteAll;
  options.fitness = Population::FitnessMode::Evaluation;
  options.variableLength = variable_length;
  options.baseStates = base_states;
  return options;
}

void test_dome_fitness()
{
  SilentOutput silence;
  Dome dome(make_options(Population::OperationMode::Minimize, 32,
                         Population::VariableLengthMode::Fixed, 2));
  BaseString *origin = makeBinaryString(std::string(32, '0'));

  expect_true(dome.FitnessFunction(origin) == 100,
              "Dome fitness at the origin should be 100");

  delete origin;
}

void test_f6_fitness_is_positive()
{
  SilentOutput silence;
  F6 f6(make_options(Population::OperationMode::Maximize, 44,
                     Population::VariableLengthMode::Fixed, 2));
  BaseString *origin = makeBinaryString(std::string(44, '0'));
  const double fitness = f6.FitnessFunction(origin);

  expect_true(fitness >= 1.0, "F6 fitness should stay positive");

  delete origin;
}

void test_spell_fitness_matches_target_word()
{
  SilentOutput silence;
  Spell spell(make_options(Population::OperationMode::Maximize, 7,
                           Population::VariableLengthMode::Variable, 26));
  BaseString *target = makeSymbolicString("egghead");
  BaseString *wrong = makeSymbolicString("aaaaaaa");

  expect_true(spell.FitnessFunction(target) == 7,
              "Spell fitness should reward the target word");
  expect_true(spell.FitnessFunction(wrong) < spell.FitnessFunction(target),
              "Spell fitness should rank the target word above a wrong word");

  delete target;
  delete wrong;
}

void test_alpha_prefers_sorted_alphabet()
{
  SilentOutput silence;
  Alpha alpha(make_options(Population::OperationMode::Maximize, 13,
                           Population::VariableLengthMode::Variable, 13));
  BaseString *sorted = new BaseString(13, 13);
  BaseString *reversed = new BaseString(13, 13);
  for (int i = 0 ; i < 13 ; i++)
    {
      sorted->set(i, i);
      reversed->set(i, 12 - i);
    }

  expect_true(alpha.FitnessFunction(sorted) > alpha.FitnessFunction(reversed),
              "Alpha fitness should prefer sorted sequences");

  delete sorted;
  delete reversed;
}

void test_traveling_salesman_construction_and_validation()
{
  SilentOutput silence;
  TravelingSalesman tsp(make_options(Population::OperationMode::Minimize, 5,
                                     Population::VariableLengthMode::Variable, 5), 10);

  expect_true(static_cast<int>(tsp.xCoordinates.size()) == 5,
              "TravelingSalesman should allocate one X coordinate per city");
  expect_true(static_cast<int>(tsp.yCoordinates.size()) == 5,
              "TravelingSalesman should allocate one Y coordinate per city");

  for (int i = 0 ; i < tsp.numCities ; i++)
    {
      expect_true(tsp.xCoordinates[i] >= 0 && tsp.xCoordinates[i] < tsp.gridSize,
                  "TravelingSalesman X coordinates should be in range");
      expect_true(tsp.yCoordinates[i] >= 0 && tsp.yCoordinates[i] < tsp.gridSize,
                  "TravelingSalesman Y coordinates should be in range");
      for (int j = i + 1 ; j < tsp.numCities ; j++)
        {
          const bool unique = tsp.xCoordinates[i] != tsp.xCoordinates[j]
                           || tsp.yCoordinates[i] != tsp.yCoordinates[j];
          expect_true(unique,
                      "TravelingSalesman should not generate duplicate city coordinates");
        }
    }

  BaseString route(5, 5);
  for (int i = 0 ; i < 5 ; i++)
    {
      route.set(i, i);
    }
  expect_true(tsp.FitnessFunction(&route) > 0.0,
              "TravelingSalesman route fitness should be positive");

  expect_throws<GAFatalException>(
    []() {
      SilentOutput local_silence;
      TravelingSalesman impossible(make_options(Population::OperationMode::Minimize, 5,
                                                Population::VariableLengthMode::Variable, 5), 2);
    },
    "TravelingSalesman should reject grids that cannot hold unique cities");
}

void test_nqueens_rewards_non_attacking_layouts()
{
  SilentOutput silence;
  NQueens queens(make_options(Population::OperationMode::Maximize, 8,
                              Population::VariableLengthMode::Fixed, 8));

  BaseString *solution = new BaseString(8, 8);
  const int solved_rows[8] = {0, 4, 7, 5, 2, 6, 1, 3};
  BaseString *bad = new BaseString(8, 8);
  for (int i = 0 ; i < 8 ; i++)
    {
      solution->set(i, solved_rows[i]);
      bad->set(i, 0);
    }

  expect_true(queens.FitnessFunction(solution) == 28,
              "NQueens should score a solved 8-queen layout with all non-attacking pairs");
  expect_true(queens.FitnessFunction(solution) > queens.FitnessFunction(bad),
              "NQueens should rank a solved layout above a conflicting one");

  delete solution;
  delete bad;
}

void test_knapsack_prefers_feasible_high_value_selections()
{
  SilentOutput silence;
  Knapsack knapsack(make_options(Population::OperationMode::Maximize, 12,
                                 Population::VariableLengthMode::Fixed, 2));

  BaseString *feasible = makeBinaryString("111101100000");
  BaseString *overweight = makeBinaryString("111111111111");

  expect_true(knapsack.FitnessFunction(feasible) > 0.0,
              "Knapsack should assign positive fitness to a feasible selection");
  expect_true(knapsack.FitnessFunction(feasible) > knapsack.FitnessFunction(overweight),
              "Knapsack should penalize overweight selections");

  delete feasible;
  delete overweight;
}

void test_latin_square_rewards_unique_rows_and_columns()
{
  SilentOutput silence;
  LatinSquare latin_square(make_options(Population::OperationMode::Maximize, 16,
                                        Population::VariableLengthMode::Fixed, 4));

  BaseString *solution = new BaseString(16, 4);
  const int solved_values[16] = {
    0, 1, 2, 3,
    1, 2, 3, 0,
    2, 3, 0, 1,
    3, 0, 1, 2
  };
  BaseString *bad = new BaseString(16, 4);
  for (int i = 0 ; i < 16 ; i++)
    {
      solution->set(i, solved_values[i]);
      bad->set(i, 0);
    }

  expect_true(latin_square.FitnessFunction(solution) == 32,
              "LatinSquare should score a solved 4x4 latin square at the maximum");
  expect_true(latin_square.FitnessFunction(solution) > latin_square.FitnessFunction(bad),
              "LatinSquare should rank a solved square above a degenerate one");

  delete solution;
  delete bad;
}

void test_latin_square_run_path()
{
  SilentOutput silence;
  Population::Options options = make_options(Population::OperationMode::Maximize, 16,
                                             Population::VariableLengthMode::Fixed, 4);
  options.numberOfIndividuals = 24;
  options.numberOfTrials = 80;
  options.bitMutationRate = 0.02;
  options.crossOverRate = 0.70;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;

  LatinSquare latin_square(options);
  Population::RunResult result = latin_square.execute(false);

  expect_true(result.evaluations >= options.numberOfIndividuals,
              "LatinSquare execute should complete a run without throwing");
}

void test_sudoku_rewards_valid_solution_and_givens()
{
  SilentOutput silence;
  Sudoku sudoku(make_options(Population::OperationMode::Maximize, 81,
                             Population::VariableLengthMode::Fixed, 9));

  BaseString *solution = new BaseString(81, 9);
  const int actual_solution[81] = {
    4, 2, 3, 5, 6, 7, 8, 0, 1,
    5, 6, 1, 0, 8, 4, 2, 3, 7,
    0, 8, 7, 2, 3, 1, 4, 5, 6,
    7, 4, 8, 6, 5, 0, 3, 1, 2,
    3, 1, 5, 7, 4, 2, 6, 8, 0,
    6, 0, 2, 8, 1, 3, 7, 4, 5,
    8, 5, 0, 4, 2, 6, 1, 7, 3,
    1, 7, 6, 3, 0, 8, 5, 2, 4,
    2, 3, 4, 1, 7, 5, 0, 6, 8
  };
  BaseString *bad = new BaseString(81, 9);
  for (int i = 0 ; i < 81 ; i++)
    {
      solution->set(i, actual_solution[i]);
      bad->set(i, 0);
    }

  expect_true(sudoku.FitnessFunction(solution) == 513,
              "Sudoku should score a solved board at the maximum");
  expect_true(sudoku.FitnessFunction(solution) > sudoku.FitnessFunction(bad),
              "Sudoku should rank a valid solved board above a degenerate one");

  delete solution;
  delete bad;
}

void test_sudoku_run_path()
{
  SilentOutput silence;
  Population::Options options = make_options(Population::OperationMode::Maximize, 81,
                                             Population::VariableLengthMode::Fixed, 9);
  options.numberOfIndividuals = 36;
  options.numberOfTrials = 100;
  options.bitMutationRate = 0.02;
  options.crossOverRate = 0.70;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;

  Sudoku sudoku(options);
  Population::RunResult result = sudoku.execute(false);

  expect_true(result.evaluations >= options.numberOfIndividuals,
              "Sudoku execute should complete a run without throwing");
}

void test_constrained_sudoku_preserves_row_structure_and_givens()
{
  SilentOutput silence;
  Population::Options options = make_options(Population::OperationMode::Maximize, 81,
                                             Population::VariableLengthMode::Fixed, 9);
  options.bitMutationRate = 0.05;
  options.crossOverRate = 0.80;
  InspectableSudokuConstrained sudoku(options);

  std::unique_ptr<Chromosome> first = sudoku.createInitialChromosome();
  std::unique_ptr<Chromosome> second = sudoku.createInitialChromosome();

  const int givens[81] = {
    5, 3, 0, 0, 7, 0, 0, 0, 0,
    6, 0, 0, 1, 9, 5, 0, 0, 0,
    0, 9, 8, 0, 0, 0, 0, 6, 0,
    8, 0, 0, 0, 6, 0, 0, 0, 3,
    4, 0, 0, 8, 0, 3, 0, 0, 1,
    7, 0, 0, 0, 2, 0, 0, 0, 6,
    0, 6, 0, 0, 0, 0, 2, 8, 0,
    0, 0, 0, 4, 1, 9, 0, 0, 5,
    0, 0, 0, 0, 8, 0, 0, 7, 9
  };

  auto row_is_permutation = [](BaseString *board, int row) {
    int seen[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int column = 0 ; column < 9 ; column++)
      {
        int value = board->test((row * 9) + column);
        if (value < 0 || value >= 9 || seen[value])
          {
            return false;
          }
        seen[value] = 1;
      }
    return true;
  };

  for (int cell = 0 ; cell < 81 ; cell++)
    {
      if (givens[cell] != 0)
        {
          expect_true(first->ChromosomeStr()->test(cell) + 1 == givens[cell],
                      "Constrained Sudoku initialization should preserve givens");
          expect_true(second->ChromosomeStr()->test(cell) + 1 == givens[cell],
                      "Constrained Sudoku initialization should preserve givens across all individuals");
        }
    }

  for (int row = 0 ; row < 9 ; row++)
    {
      expect_true(row_is_permutation(first->ChromosomeStr(), row),
                  "Constrained Sudoku initialization should make each row a permutation");
      expect_true(row_is_permutation(second->ChromosomeStr(), row),
                  "Constrained Sudoku initialization should make each row a permutation for every individual");
    }

  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
    sudoku.mateChromosomes(first.get(), second.get());
  sudoku.mutateChromosome(children.first.get());
  sudoku.mutateChromosome(children.second.get());

  for (int row = 0 ; row < 9 ; row++)
    {
      expect_true(row_is_permutation(children.first->ChromosomeStr(), row),
                  "Constrained Sudoku crossover/mutation should preserve row permutations");
      expect_true(row_is_permutation(children.second->ChromosomeStr(), row),
                  "Constrained Sudoku crossover/mutation should preserve row permutations in both children");
    }

  for (int cell = 0 ; cell < 81 ; cell++)
    {
      if (givens[cell] != 0)
        {
          expect_true(children.first->ChromosomeStr()->test(cell) + 1 == givens[cell],
                      "Constrained Sudoku operators should keep givens locked in the first child");
          expect_true(children.second->ChromosomeStr()->test(cell) + 1 == givens[cell],
                      "Constrained Sudoku operators should keep givens locked in the second child");
        }
    }
}

void test_constrained_sudoku_run_path()
{
  SilentOutput silence;
  Population::Options options = make_options(Population::OperationMode::Maximize, 81,
                                             Population::VariableLengthMode::Fixed, 9);
  options.numberOfIndividuals = 40;
  options.numberOfTrials = 140;
  options.bitMutationRate = 0.05;
  options.crossOverRate = 0.80;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;

  SudokuConstrained sudoku(options);
  Population::RunResult result = sudoku.execute(false);

  expect_true(result.evaluations >= options.numberOfIndividuals,
              "Constrained Sudoku execute should complete a run without throwing");
}

}  // namespace

int main()
{
  test_dome_fitness();
  test_f6_fitness_is_positive();
  test_spell_fitness_matches_target_word();
  test_alpha_prefers_sorted_alphabet();
  test_traveling_salesman_construction_and_validation();
  test_nqueens_rewards_non_attacking_layouts();
  test_knapsack_prefers_feasible_high_value_selections();
  test_latin_square_rewards_unique_rows_and_columns();
  test_latin_square_run_path();
  test_sudoku_rewards_valid_solution_and_givens();
  test_sudoku_run_path();
  test_constrained_sudoku_preserves_row_structure_and_givens();
  test_constrained_sudoku_run_path();

  if (g_failures != 0)
    {
      std::cerr << g_failures << " example test(s) failed" << std::endl;
      return 1;
    }

  std::cout << "All example tests passed" << std::endl;
  return 0;
}
