#include <iostream>
#include <sstream>
#include <string>

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
#include "ts.hh"
#include "spell.hh"

class PopulationTestRig
{
public:
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
  for (int i = 0 ; i < static_cast<int>(bits.length()) ; i++)
    {
      if (bits[i] == '1')
        {
          b.setValue(i);
        }
      else
        {
          b.clearValue(i);
        }
    }
  return b;
}

BaseString makeSymbolicString(const std::string& letters)
{
  BaseString b(letters.length(), 26);
  for (int i = 0 ; i < static_cast<int>(letters.length()) ; i++)
    {
      b.setValue(i, letters[i] - 'a');
    }
  return b;
}

Population::Settings make_options(Population::OperationMode operation,
                                 int diversity,
                                 Population::VariableLengthMode variable_length,
                                 int base_states)
{
  Population::Settings options;
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
  Dome dome;
  BaseString origin = makeBinaryString(std::string(32, '0'));

  expect_true(dome.evaluateFitness(origin) == 100,
              "Dome fitness at the origin should be 100");
}

void test_f6_fitness_is_positive()
{
  F6 f6;
  BaseString origin = makeBinaryString(std::string(44, '0'));
  const double fitness = f6.evaluateFitness(origin);

  expect_true(fitness >= 1.0, "F6 fitness should stay positive");
}

void test_spell_fitness_matches_target_word()
{
  Spell spell;
  BaseString target = makeSymbolicString("egghead");
  BaseString wrong = makeSymbolicString("aaaaaaa");

  expect_true(spell.evaluateFitness(target) == 7,
              "Spell fitness should reward the target word");
  expect_true(spell.evaluateFitness(wrong) < spell.evaluateFitness(target),
              "Spell fitness should rank the target word above a wrong word");
}

void test_alpha_prefers_sorted_alphabet()
{
  Alpha alpha(make_options(Population::OperationMode::Maximize, 13,
                           Population::VariableLengthMode::Variable, 13));
  BaseString sorted(13, 13);
  BaseString reversed(13, 13);
  for (int i = 0 ; i < 13 ; i++)
    {
      sorted.setValue(i, i);
      reversed.setValue(i, 12 - i);
    }

  expect_true(alpha.evaluateFitness(sorted) > alpha.evaluateFitness(reversed),
              "Alpha fitness should prefer sorted sequences");
}

void test_traveling_salesman_construction_and_validation()
{
  Population::Settings options = make_options(Population::OperationMode::Minimize, 5,
                                              Population::VariableLengthMode::Variable, 5);
  TravelingSalesman tsp(options, 10);
  Population population(options, tsp);

  expect_true(static_cast<int>(tsp.cityCoordinates().size()) == 5,
              "TravelingSalesman should allocate one coordinate per city");

  for (int i = 0 ; i < tsp.cityCount() ; i++)
    {
      expect_true(tsp.cityCoordinates()[i].first >= 0 && tsp.cityCoordinates()[i].first < tsp.cityGridSize(),
                  "TravelingSalesman X coordinates should be in range");
      expect_true(tsp.cityCoordinates()[i].second >= 0 && tsp.cityCoordinates()[i].second < tsp.cityGridSize(),
                  "TravelingSalesman Y coordinates should be in range");
      for (int j = i + 1 ; j < tsp.cityCount() ; j++)
        {
          const bool unique = tsp.cityCoordinates()[i] != tsp.cityCoordinates()[j];
          expect_true(unique,
                      "TravelingSalesman should not generate duplicate city coordinates");
        }
    }

  BaseString route(5, 5);
  for (int i = 0 ; i < 5 ; i++)
    {
      route.setValue(i, i);
    }
  expect_true(tsp.evaluateFitness(route) > 0.0,
              "TravelingSalesman route fitness should be positive");

  expect_throws<GAFatalException>(
    []() {
      Population::Settings bad_options = make_options(Population::OperationMode::Minimize, 5,
                                                      Population::VariableLengthMode::Variable, 5);
      TravelingSalesman impossible(bad_options, 2);
    },
    "TravelingSalesman should reject grids that cannot hold unique cities");
}

void test_traveling_salesman_fixed_seed_is_reproducible()
{
  Population::Settings options = make_options(Population::OperationMode::Minimize, 5,
                                             Population::VariableLengthMode::Variable, 5);
  options.useFixedRandomSeed = true;
  options.randomSeed = 314159U;

  TravelingSalesman first(options, 10);
  TravelingSalesman second(options, 10);
  Population first_population(options, first);
  Population second_population(options, second);

  expect_true(first.cityCoordinates() == second.cityCoordinates(),
              "TravelingSalesman should generate the same city coordinates for a fixed seed");
}

void test_nqueens_rewards_non_attacking_layouts()
{
  NQueens queens;

  BaseString solution(8, 8);
  const int solved_rows[8] = {0, 4, 7, 5, 2, 6, 1, 3};
  BaseString bad(8, 8);
  for (int i = 0 ; i < 8 ; i++)
    {
      solution.setValue(i, solved_rows[i]);
      bad.setValue(i, 0);
    }

  expect_true(queens.evaluateFitness(solution) == 28,
              "NQueens should score a solved 8-queen layout with all non-attacking pairs");
  expect_true(queens.evaluateFitness(solution) > queens.evaluateFitness(bad),
              "NQueens should rank a solved layout above a conflicting one");
}

void test_knapsack_prefers_feasible_high_value_selections()
{
  Knapsack knapsack;

  BaseString feasible = makeBinaryString("111101100000");
  BaseString overweight = makeBinaryString("111111111111");

  expect_true(knapsack.evaluateFitness(feasible) > 0.0,
              "Knapsack should assign positive fitness to a feasible selection");
  expect_true(knapsack.evaluateFitness(feasible) > knapsack.evaluateFitness(overweight),
              "Knapsack should penalize overweight selections");
}

void test_latin_square_rewards_unique_rows_and_columns()
{
  LatinSquare latin_square;

  BaseString solution(16, 4);
  const int solved_values[16] = {
    0, 1, 2, 3,
    1, 2, 3, 0,
    2, 3, 0, 1,
    3, 0, 1, 2
  };
  BaseString bad(16, 4);
  for (int i = 0 ; i < 16 ; i++)
    {
      solution.setValue(i, solved_values[i]);
      bad.setValue(i, 0);
    }

  expect_true(latin_square.evaluateFitness(solution) == 32,
              "LatinSquare should score a solved 4x4 latin square at the maximum");
  expect_true(latin_square.evaluateFitness(solution) > latin_square.evaluateFitness(bad),
              "LatinSquare should rank a solved square above a degenerate one");
}

void test_latin_square_run_path()
{
  Population::Settings options = make_options(Population::OperationMode::Maximize, 16,
                                             Population::VariableLengthMode::Fixed, 4);
  options.numberOfIndividuals = 24;
  options.numberOfTrials = 80;
  options.bitMutationRate = 0.02;
  options.crossOverRate = 0.70;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;

  LatinSquare latin_square;
  Population population(options, latin_square);
  Population::RunResult result = population.execute(false);

  expect_true(result.evaluations >= options.numberOfIndividuals,
              "LatinSquare execute should complete a run without throwing");
}

void test_sudoku_rewards_valid_solution_and_givens()
{
  Sudoku sudoku;

  BaseString solution(81, 9);
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
  BaseString bad(81, 9);
  for (int i = 0 ; i < 81 ; i++)
    {
      solution.setValue(i, actual_solution[i]);
      bad.setValue(i, 0);
    }

  expect_true(sudoku.evaluateFitness(solution) == 513,
              "Sudoku should score a solved board at the maximum");
  expect_true(sudoku.evaluateFitness(solution) > sudoku.evaluateFitness(bad),
              "Sudoku should rank a valid solved board above a degenerate one");
}

void test_sudoku_run_path()
{
  Population::Settings options = make_options(Population::OperationMode::Maximize, 81,
                                             Population::VariableLengthMode::Fixed, 9);
  options.numberOfIndividuals = 36;
  options.numberOfTrials = 100;
  options.bitMutationRate = 0.02;
  options.crossOverRate = 0.70;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;

  Sudoku sudoku;
  Population population(options, sudoku);
  Population::RunResult result = population.execute(false);

  expect_true(result.evaluations >= options.numberOfIndividuals,
              "Sudoku execute should complete a run without throwing");
}

void test_constrained_sudoku_preserves_row_structure_and_givens()
{
  Population::Settings options = make_options(Population::OperationMode::Maximize, 81,
                                             Population::VariableLengthMode::Fixed, 9);
  options.bitMutationRate = 0.05;
  options.crossOverRate = 0.80;
  SudokuConstrained sudoku(options);

  std::unique_ptr<Chromosome> first = PopulationTestRig::createInitialChromosome(sudoku);
  std::unique_ptr<Chromosome> second = PopulationTestRig::createInitialChromosome(sudoku);

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

  auto row_is_permutation = [](const BaseString& board, int row) {
    int seen[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int column = 0 ; column < 9 ; column++)
      {
        int value = board.valueAt((row * 9) + column);
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
          expect_true(first->genes().valueAt(cell) + 1 == givens[cell],
                      "Constrained Sudoku initialization should preserve givens");
          expect_true(second->genes().valueAt(cell) + 1 == givens[cell],
                      "Constrained Sudoku initialization should preserve givens across all individuals");
        }
    }

  for (int row = 0 ; row < 9 ; row++)
    {
      expect_true(row_is_permutation(first->genes(), row),
                  "Constrained Sudoku initialization should make each row a permutation");
      expect_true(row_is_permutation(second->genes(), row),
                  "Constrained Sudoku initialization should make each row a permutation for every individual");
    }

  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
    PopulationTestRig::mateChromosomes(sudoku, *first, *second);
  PopulationTestRig::mutateChromosome(sudoku, *children.first);
  PopulationTestRig::mutateChromosome(sudoku, *children.second);

  for (int row = 0 ; row < 9 ; row++)
    {
      expect_true(row_is_permutation(children.first->genes(), row),
                  "Constrained Sudoku crossover/mutation should preserve row permutations");
      expect_true(row_is_permutation(children.second->genes(), row),
                  "Constrained Sudoku crossover/mutation should preserve row permutations in both children");
    }

  for (int cell = 0 ; cell < 81 ; cell++)
    {
      if (givens[cell] != 0)
        {
          expect_true(children.first->genes().valueAt(cell) + 1 == givens[cell],
                      "Constrained Sudoku operators should keep givens locked in the first child");
          expect_true(children.second->genes().valueAt(cell) + 1 == givens[cell],
                      "Constrained Sudoku operators should keep givens locked in the second child");
        }
    }
}

void test_constrained_sudoku_run_path()
{
  Population::Settings options = make_options(Population::OperationMode::Maximize, 81,
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

void test_constrained_sudoku_fixed_seed_is_reproducible()
{
  Population::Settings options = make_options(Population::OperationMode::Maximize, 81,
                                             Population::VariableLengthMode::Fixed, 9);
  options.useFixedRandomSeed = true;
  options.randomSeed = 271828U;

  SudokuConstrained first(options);
  SudokuConstrained second(options);
  std::unique_ptr<Chromosome> first_board = PopulationTestRig::createInitialChromosome(first);
  std::unique_ptr<Chromosome> second_board = PopulationTestRig::createInitialChromosome(second);

  expect_true(first_board->equals(*second_board),
              "Constrained Sudoku should generate the same initial chromosome for a fixed seed");
}

}  // namespace

int main()
{
  test_dome_fitness();
  test_f6_fitness_is_positive();
  test_spell_fitness_matches_target_word();
  test_alpha_prefers_sorted_alphabet();
  test_traveling_salesman_construction_and_validation();
  test_traveling_salesman_fixed_seed_is_reproducible();
  test_nqueens_rewards_non_attacking_layouts();
  test_knapsack_prefers_feasible_high_value_selections();
  test_latin_square_rewards_unique_rows_and_columns();
  test_latin_square_run_path();
  test_sudoku_rewards_valid_solution_and_givens();
  test_sudoku_run_path();
  test_constrained_sudoku_preserves_row_structure_and_givens();
  test_constrained_sudoku_run_path();
  test_constrained_sudoku_fixed_seed_is_reproducible();

  if (g_failures != 0)
    {
      std::cerr << g_failures << " example test(s) failed" << std::endl;
      return 1;
    }

  std::cout << "All example tests passed" << std::endl;
  return 0;
}
