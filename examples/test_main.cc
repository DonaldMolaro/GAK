#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "base.hh"
#include "chromosome.hh"
#include "except.hh"
#include "population.hh"
#include "alpha.hh"
#include "dome.hh"
#include "f6.hh"
#include "graphcoloring.hh"
#include "knapsack.hh"
#include "latinsquare.hh"
#include "nqueens.hh"
#include "sudoku.hh"
#include "sudoku_constrained.hh"
#include "timetable.hh"
#include "ts.hh"
#include "spell.hh"
#include "dataset.hh"
#include "expression.hh"
#include "regression.hh"
#include "random.hh"

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

void expect_contains(const std::string& haystack,
                     const std::string& needle,
                     const std::string& message)
{
  expect_true(haystack.find(needle) != std::string::npos, message);
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
                                 int chromosome_length,
                                 Population::VariableLengthMode variable_length,
                                 int base_states)
{
  Population::Settings options;
  options.operation = operation;
  options.numberOfIndividuals = 6;
  options.numberOfTrials = 12;
  options.chromosomeLength = chromosome_length;
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
  std::ostringstream out;

  expect_true(dome.evaluateFitness(origin) == 100,
              "Dome fitness at the origin should be 100");
  dome.printCandidate(origin, out);
  expect_contains(out.str(), "X ( 0 ) Y ( 0 )",
                  "Dome should print decoded coordinates");
}

void test_f6_fitness_is_positive()
{
  F6 f6;
  BaseString origin = makeBinaryString(std::string(44, '0'));
  const double fitness = f6.evaluateFitness(origin);
  std::ostringstream out;

  expect_true(fitness >= 1.0, "F6 fitness should stay positive");
  f6.printCandidate(origin, out);
  expect_contains(out.str(), "X ( -100 ) Y ( -100 )",
                  "F6 should print shifted coordinates");
}

void test_spell_fitness_matches_target_word()
{
  Spell spell;
  Population::Settings settings = make_options(Population::OperationMode::Maximize, 7,
                                               Population::VariableLengthMode::Fixed, 26);
  Population population(settings, spell);
  BaseString target = makeSymbolicString("egghead");
  BaseString wrong = makeSymbolicString("aaaaaaa");
  const double target_fitness = spell.evaluateFitness(target);

  expect_true(target_fitness == 7,
              "Spell fitness should reward the target word");
  expect_true(spell.evaluateFitness(wrong) < spell.evaluateFitness(target),
              "Spell fitness should rank the target word above a wrong word");
  expect_true(spell.hasReachedSolution(population, target, target_fitness),
              "Spell should report when the target word has been found");
}

void test_alpha_prefers_sorted_alphabet()
{
  Population::Settings settings = make_options(Population::OperationMode::Maximize, 13,
                                               Population::VariableLengthMode::Variable, 13);
  Alpha alpha(settings);
  Population population(settings, alpha);
  BaseString sorted(13, 13);
  BaseString reversed(13, 13);
  for (int i = 0 ; i < 13 ; i++)
    {
      sorted.setValue(i, i);
      reversed.setValue(i, 12 - i);
    }

  const double sorted_fitness = alpha.evaluateFitness(sorted);
  std::ostringstream out;
  expect_true(sorted_fitness > alpha.evaluateFitness(reversed),
              "Alpha fitness should prefer sorted sequences");
  expect_true(alpha.hasReachedSolution(population, sorted, sorted_fitness),
              "Alpha should report when a fully sorted alphabet has been found");
  alpha.printCandidate(sorted, out);
  expect_contains(out.str(), "abcdefghijklm ::",
                  "Alpha should print symbolic candidates as letters");
}

void test_alpha_rejects_mismatched_population_settings()
{
  Population::Settings alpha_settings = make_options(Population::OperationMode::Maximize, 13,
                                                     Population::VariableLengthMode::Variable, 13);
  Population::Settings mismatched = alpha_settings;
  mismatched.baseStates = 12;
  Alpha alpha(alpha_settings);

  expect_throws<GAFatalException>(
    [&]() {
      Population invalid_population(mismatched, alpha);
    },
    "Alpha should reject mismatched population settings");
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
  std::ostringstream route_out;
  tsp.printCandidate(route, route_out);
  expect_contains(route_out.str(), "abcde ::",
                  "TravelingSalesman should print routes as symbolic city names");

  std::ostringstream city_list_out;
  tsp.writeCityList(city_list_out);
  expect_contains(city_list_out.str(), "Problem input:",
                  "TravelingSalesman should print an input section heading");
  expect_contains(city_list_out.str(), "City list:",
                  "TravelingSalesman should label its city list");
  expect_contains(city_list_out.str(), "a:(",
                  "TravelingSalesman should print named city coordinates");

  expect_throws<GAFatalException>(
    []() {
      Population::Settings bad_options = make_options(Population::OperationMode::Minimize, 5,
                                                      Population::VariableLengthMode::Variable, 5);
      TravelingSalesman impossible(bad_options, 2);
    },
    "TravelingSalesman should reject grids that cannot hold unique cities");

  expect_throws<GAFatalException>(
    []() {
      Population::Settings bad_options = make_options(Population::OperationMode::Minimize, 5,
                                                      Population::VariableLengthMode::Variable, 5);
      TravelingSalesman impossible(bad_options, 0);
      Population::Settings mismatched = bad_options;
      mismatched.baseStates = 4;
      Population invalid_population(mismatched, impossible);
    },
    "TravelingSalesman should reject mismatched population settings");
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

void test_traveling_salesman_visualization_json()
{
  Population::Settings options = make_options(Population::OperationMode::Minimize, 5,
                                              Population::VariableLengthMode::Variable, 5);
  TravelingSalesman tsp(options, 10);
  BaseString route(5, 5);
  for (int i = 0 ; i < 5 ; i++)
  {
    route.setValue(i, i);
  }

  std::ostringstream out;
  tsp.writeVisualizationJson(route, out);
  expect_contains(out.str(), "\"type\": \"traveling_salesman\"",
                  "TravelingSalesman should emit a typed visualization payload");
  expect_contains(out.str(), "\"cities\":",
                  "TravelingSalesman visualization should include city coordinates");
  expect_contains(out.str(), "\"route\":",
                  "TravelingSalesman visualization should include the route order");
}

void test_nqueens_rewards_non_attacking_layouts()
{
  NQueens queens;
  Population::Settings settings = make_options(Population::OperationMode::Maximize, 8,
                                               Population::VariableLengthMode::Fixed, 8);
  Population population(settings, queens);

  BaseString solution(8, 8);
  const int solved_rows[8] = {0, 4, 7, 5, 2, 6, 1, 3};
  BaseString bad(8, 8);
  std::ostringstream out;
  for (int i = 0 ; i < 8 ; i++)
    {
      solution.setValue(i, solved_rows[i]);
      bad.setValue(i, 0);
    }

  expect_true(queens.evaluateFitness(solution) == 28,
              "NQueens should score a solved 8-queen layout with all non-attacking pairs");
  expect_true(queens.evaluateFitness(solution) > queens.evaluateFitness(bad),
              "NQueens should rank a solved layout above a conflicting one");
  expect_true(queens.hasReachedSolution(population, solution, queens.evaluateFitness(solution)),
              "NQueens should report when a non-attacking layout has been found");
  queens.printCandidate(solution, out);
  expect_contains(out.str(), "Queens:",
                  "NQueens should print row assignments");
  expect_contains(out.str(), "Q",
                  "NQueens should print a board view");
}

void test_nqueens_visualization_json()
{
  NQueens queens;
  BaseString solution(8, 8);
  const int solved_rows[8] = {0, 4, 7, 5, 2, 6, 1, 3};
  for (int i = 0 ; i < 8 ; i++)
  {
    solution.setValue(i, solved_rows[i]);
  }

  std::ostringstream out;
  queens.writeVisualizationJson(solution, out);
  expect_contains(out.str(), "\"type\": \"nqueens\"",
                  "NQueens should emit a typed visualization payload");
  expect_contains(out.str(), "\"rows\":",
                  "NQueens visualization should include row assignments");
  expect_contains(out.str(), "\"conflicts\":",
                  "NQueens visualization should include conflict relationships");
}

void test_knapsack_prefers_feasible_high_value_selections()
{
  Knapsack knapsack;

  BaseString feasible = makeBinaryString("111101100000");
  BaseString overweight = makeBinaryString("111111111111");
  BaseString zeroed = makeBinaryString("000000000000");
  std::ostringstream out;

  expect_true(knapsack.evaluateFitness(feasible) > 0.0,
              "Knapsack should assign positive fitness to a feasible selection");
  expect_true(knapsack.evaluateFitness(feasible) > knapsack.evaluateFitness(overweight),
              "Knapsack should penalize overweight selections");
  expect_true(knapsack.evaluateFitness(zeroed) == 0.0,
              "Knapsack should return zero for an empty selection");
  knapsack.printCandidate(feasible, out);
  expect_contains(out.str(), "Items:",
                  "Knapsack should print selected item labels");
  expect_contains(out.str(), "weight=",
                  "Knapsack should print total weight");
  expect_contains(out.str(), "value=",
                  "Knapsack should print total value");
}

void test_knapsack_visualization_json()
{
  Knapsack knapsack;
  BaseString feasible = makeBinaryString("111101100000");
  std::ostringstream out;

  knapsack.writeVisualizationJson(feasible, out);
  expect_contains(out.str(), "\"type\": \"knapsack\"",
                  "Knapsack should emit a typed visualization payload");
  expect_contains(out.str(), "\"items\":",
                  "Knapsack visualization should include item state");
  expect_contains(out.str(), "\"capacity\": 35",
                  "Knapsack visualization should include capacity");
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
  std::ostringstream out;
  for (int i = 0 ; i < 16 ; i++)
    {
      solution.setValue(i, solved_values[i]);
      bad.setValue(i, 0);
    }

  expect_true(latin_square.evaluateFitness(solution) == 32,
              "LatinSquare should score a solved 4x4 latin square at the maximum");
  expect_true(latin_square.evaluateFitness(solution) > latin_square.evaluateFitness(bad),
              "LatinSquare should rank a solved square above a degenerate one");
  latin_square.printCandidate(solution, out);
  expect_contains(out.str(), "Latin square (4x4):",
                  "LatinSquare should print the board size");
  expect_contains(out.str(), "0 1 2 3",
                  "LatinSquare should print row values");
}

void test_latin_square_validation_rejects_bad_settings()
{
  LatinSquare latin_square;

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = make_options(Population::OperationMode::Maximize, 15,
                                              Population::VariableLengthMode::Fixed, 4);
      Population invalid_population(bad, latin_square);
    },
    "LatinSquare should reject non-square chromosome lengths");

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = make_options(Population::OperationMode::Maximize, 16,
                                              Population::VariableLengthMode::Fixed, 5);
      Population invalid_population(bad, latin_square);
    },
    "LatinSquare should reject base-state counts that do not match the square width");
}

void test_latin_square_visualization_json()
{
  LatinSquare latin_square;
  BaseString solution(16, 4);
  const int solved_values[16] = {
    0, 1, 2, 3,
    1, 2, 3, 0,
    2, 3, 0, 1,
    3, 0, 1, 2
  };
  for (int i = 0 ; i < 16 ; i++)
  {
    solution.setValue(i, solved_values[i]);
  }

  std::ostringstream out;
  latin_square.writeVisualizationJson(solution, out);
  expect_contains(out.str(), "\"type\": \"latin_square\"",
                  "LatinSquare should emit a typed visualization payload");
  expect_contains(out.str(), "\"size\": 4",
                  "LatinSquare visualization should include the board size");
  expect_contains(out.str(), "\"cells\":",
                  "LatinSquare visualization should include cell values");
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

void test_graph_coloring_rewards_proper_colorings()
{
  GraphColoring graph;
  Population::Settings settings = make_options(Population::OperationMode::Maximize, 16,
                                               Population::VariableLengthMode::Fixed, 3);
  Population population(settings, graph);

  BaseString solved(16, 3);
  const int solved_colors[16] = {0, 1, 2, 0, 1, 2, 1, 1, 0, 2, 2, 0, 1, 0, 2, 1};
  BaseString bad(16, 3);
  std::ostringstream out;
  for (int i = 0 ; i < 16 ; i++)
  {
    solved.setValue(i, solved_colors[i]);
    bad.setValue(i, 0);
  }

  const double solved_fitness = graph.evaluateFitness(solved);
  expect_true(solved_fitness == GraphColoring::kEdgeCount,
              "GraphColoring should score a proper 3-coloring at the maximum");
  expect_true(solved_fitness > graph.evaluateFitness(bad),
              "GraphColoring should rank a proper coloring above a conflicting one");
  expect_true(graph.hasReachedSolution(population, solved, solved_fitness),
              "GraphColoring should report an exact coloring when all edges are satisfied");
  graph.printCandidate(solved, out);
  expect_contains(out.str(), "Coloring:",
                  "GraphColoring should print node assignments");
  expect_contains(out.str(), "conflicts=0",
                  "GraphColoring should report zero conflicts for a solved coloring");
}

void test_graph_coloring_validation_rejects_bad_settings()
{
  GraphColoring graph;

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = make_options(Population::OperationMode::Maximize, 15,
                                              Population::VariableLengthMode::Fixed, 3);
      Population invalid_population(bad, graph);
    },
    "GraphColoring should reject chromosome lengths other than the node count");

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = make_options(Population::OperationMode::Maximize, 16,
                                              Population::VariableLengthMode::Fixed, 4);
      Population invalid_population(bad, graph);
    },
    "GraphColoring should reject base-state counts other than three colors");
}

void test_graph_coloring_visualization_json()
{
  GraphColoring graph;
  BaseString coloring(16, 3);
  const int solved_colors[16] = {0, 1, 2, 0, 1, 2, 1, 1, 0, 2, 2, 0, 1, 0, 2, 1};
  for (int i = 0 ; i < 16 ; i++)
  {
    coloring.setValue(i, solved_colors[i]);
  }

  std::ostringstream out;
  graph.writeVisualizationJson(coloring, out);
  expect_contains(out.str(), "\"type\": \"graph_coloring\"",
                  "GraphColoring should emit a typed visualization payload");
  expect_contains(out.str(), "\"nodes\":",
                  "GraphColoring visualization should include node assignments");
  expect_contains(out.str(), "\"edges\":",
                  "GraphColoring visualization should include edge conflict state");
}

void test_timetable_rewards_conflict_free_preferred_assignments()
{
  Timetable timetable;
  Population::Settings settings = make_options(Population::OperationMode::Maximize, 8,
                                               Population::VariableLengthMode::Fixed, 4);
  Population population(settings, timetable);

  BaseString solved(8, 4);
  const int solved_slots[8] = {0, 1, 2, 3, 0, 1, 3, 2};
  BaseString bad(8, 4);
  std::ostringstream out;
  for (int i = 0 ; i < 8 ; i++)
  {
    solved.setValue(i, solved_slots[i]);
    bad.setValue(i, 0);
  }

  const double solved_fitness = timetable.evaluateFitness(solved);
  expect_true(solved_fitness == 108,
              "Timetable should score a conflict-free preferred schedule at the maximum");
  expect_true(solved_fitness > timetable.evaluateFitness(bad),
              "Timetable should rank a good schedule above a conflicting one");
  expect_true(timetable.hasReachedSolution(population, solved, solved_fitness),
              "Timetable should report when the hard and soft scheduling goals are all met");
  timetable.printCandidate(solved, out);
  expect_contains(out.str(), "Timetable:",
                  "Timetable should print course assignments");
  expect_contains(out.str(), "preferred=8/8",
                  "Timetable should report the number of preferences satisfied");
}

void test_timetable_validation_rejects_bad_settings()
{
  Timetable timetable;

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = make_options(Population::OperationMode::Maximize, 7,
                                              Population::VariableLengthMode::Fixed, 4);
      Population invalid_population(bad, timetable);
    },
    "Timetable should reject chromosome lengths other than the course count");

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = make_options(Population::OperationMode::Maximize, 8,
                                              Population::VariableLengthMode::Fixed, 3);
      Population invalid_population(bad, timetable);
    },
    "Timetable should reject base-state counts other than the slot count");
}

void test_timetable_visualization_json()
{
  Timetable timetable;
  BaseString schedule(8, 4);
  const int solved_slots[8] = {0, 1, 2, 3, 0, 1, 3, 2};
  for (int i = 0 ; i < 8 ; i++)
  {
    schedule.setValue(i, solved_slots[i]);
  }

  std::ostringstream out;
  timetable.writeVisualizationJson(schedule, out);
  expect_contains(out.str(), "\"type\": \"timetable\"",
                  "Timetable should emit a typed visualization payload");
  expect_contains(out.str(), "\"courses\":",
                  "Timetable visualization should include course assignments");
  expect_contains(out.str(), "\"conflicts\":",
                  "Timetable visualization should include conflict relationships");
}

void test_sudoku_rewards_valid_solution_and_givens()
{
  Sudoku sudoku;
  Population::Settings settings = make_options(Population::OperationMode::Maximize, 81,
                                               Population::VariableLengthMode::Fixed, 9);
  Population population(settings, sudoku);

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
  std::ostringstream out;
  for (int i = 0 ; i < 81 ; i++)
    {
      solution.setValue(i, actual_solution[i]);
      bad.setValue(i, 0);
    }

  const double solution_fitness = sudoku.evaluateFitness(solution);
  expect_true(solution_fitness == 513,
              "Sudoku should score a solved board at the maximum");
  expect_true(sudoku.evaluateFitness(solution) > sudoku.evaluateFitness(bad),
              "Sudoku should rank a valid solved board above a degenerate one");
  expect_true(sudoku.hasReachedSolution(population, solution, solution_fitness),
              "Sudoku should report when a solved board has been found");
  sudoku.printCandidate(solution, out);
  expect_contains(out.str(), "Sudoku candidate:",
                  "Sudoku should print a labeled board");
  expect_contains(out.str(), "|",
                  "Sudoku should print subgrid separators");
}

void test_sudoku_validation_rejects_bad_settings()
{
  Sudoku sudoku;

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = make_options(Population::OperationMode::Maximize, 80,
                                              Population::VariableLengthMode::Fixed, 9);
      Population invalid_population(bad, sudoku);
    },
    "Sudoku should reject chromosome lengths other than 81");

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = make_options(Population::OperationMode::Maximize, 81,
                                              Population::VariableLengthMode::Fixed, 8);
      Population invalid_population(bad, sudoku);
    },
    "Sudoku should reject base-state counts other than 9");
}

void test_sudoku_visualization_json()
{
  Sudoku sudoku;
  BaseString board(81, 9);
  for (int i = 0 ; i < 81 ; i++)
  {
    board.setValue(i, i % 9);
  }

  std::ostringstream out;
  sudoku.writeVisualizationJson(board, out);
  expect_contains(out.str(), "\"type\": \"sudoku\"",
                  "Sudoku should emit a typed visualization payload");
  expect_contains(out.str(), "\"variant\": \"generic\"",
                  "Sudoku visualization should identify the generic variant");
  expect_contains(out.str(), "\"givens\":",
                  "Sudoku visualization should include givens");
  expect_contains(out.str(), "\"board\":",
                  "Sudoku visualization should include board values");
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
  Population population(options, sudoku);

  std::unique_ptr<Chromosome> first = PopulationTestRig::createInitialChromosome(population);
  std::unique_ptr<Chromosome> second = PopulationTestRig::createInitialChromosome(population);

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
    PopulationTestRig::mateChromosomes(population, *first, *second);
  PopulationTestRig::mutateChromosome(population, *children.first);
  PopulationTestRig::mutateChromosome(population, *children.second);

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
  Population population(options, sudoku);
  Population::RunResult result = population.execute(false);

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
  Population first_population(options, first);
  Population second_population(options, second);
  std::unique_ptr<Chromosome> first_board = PopulationTestRig::createInitialChromosome(first_population);
  std::unique_ptr<Chromosome> second_board = PopulationTestRig::createInitialChromosome(second_population);

  expect_true(first_board->equals(*second_board),
              "Constrained Sudoku should generate the same initial chromosome for a fixed seed");
}

void test_constrained_sudoku_visualization_json()
{
  Population::Settings options = make_options(Population::OperationMode::Maximize, 81,
                                              Population::VariableLengthMode::Fixed, 9);
  SudokuConstrained sudoku(options);
  Population population(options, sudoku);
  std::unique_ptr<Chromosome> board = PopulationTestRig::createInitialChromosome(population);

  std::ostringstream out;
  sudoku.writeVisualizationJson(board->genes(), out);
  expect_contains(out.str(), "\"type\": \"sudoku\"",
                  "Constrained Sudoku should emit a typed visualization payload");
  expect_contains(out.str(), "\"variant\": \"constraint_aware\"",
                  "Constrained Sudoku visualization should identify the variant");
  expect_contains(out.str(), "\"givens\":",
                  "Constrained Sudoku visualization should include givens");
  expect_contains(out.str(), "\"board\":",
                  "Constrained Sudoku visualization should include the board values");
}

void test_constrained_sudoku_validation_rejects_bad_settings()
{
  Population::Settings options = make_options(Population::OperationMode::Maximize, 81,
                                             Population::VariableLengthMode::Fixed, 9);
  SudokuConstrained sudoku(options);

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = options;
      bad.chromosomeLength = 80;
      Population invalid_population(bad, sudoku);
    },
    "Constrained Sudoku should reject chromosome lengths other than 81");

  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings bad = options;
      bad.baseStates = 8;
      Population invalid_population(bad, sudoku);
    },
    "Constrained Sudoku should reject base-state counts other than 9");
}

}  // namespace

namespace {

bool isPermutation(const BaseString& genes)
{
  std::vector<int> seen(genes.length(), 0);
  for (int i = 0 ; i < genes.length() ; i++)
    {
      const int value = genes.valueAt(i);
      if (value < 0 || value >= genes.length() || seen[value]) return false;
      seen[value] = 1;
    }
  return true;
}

void test_traveling_salesman_operators_keep_tours_valid()
{
  Population::Settings options = make_options(Population::OperationMode::Minimize, 12,
                                              Population::VariableLengthMode::Fixed, 12);
  options.crossOverRate = 1.0;
  options.bitMutationRate = 0.3;
  TravelingSalesman tsp(options, 20);
  Population population(options, tsp);

  for (int trial = 0 ; trial < 200 ; trial++)
    {
      std::unique_ptr<Chromosome> mother = PopulationTestRig::createInitialChromosome(population);
      std::unique_ptr<Chromosome> father = PopulationTestRig::createInitialChromosome(population);
      expect_true(isPermutation(mother->genes()) && isPermutation(father->genes()),
                  "TravelingSalesman should initialize candidates as permutations");
      std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
        PopulationTestRig::mateChromosomes(population, *mother, *father);
      PopulationTestRig::mutateChromosome(population, *children.first);
      PopulationTestRig::mutateChromosome(population, *children.second);
      expect_true(isPermutation(children.first->genes()) && isPermutation(children.second->genes()),
                  "TravelingSalesman crossover and mutation should keep every tour a permutation");
    }

  // A tour with a repeated city must score worse than the valid tour it
  // was derived from, whatever the city layout.
  BaseString valid(12, 12);
  BaseString repeated(12, 12);
  for (int i = 0 ; i < 12 ; i++)
    {
      valid.setValue(i, i);
      repeated.setValue(i, i);
    }
  repeated.setValue(5, 0);
  expect_true(tsp.evaluateFitness(repeated) > tsp.evaluateFitness(valid),
              "TravelingSalesman should always penalize a missing city more than any edge saves");
}

void test_traveling_salesman_rejects_mismatched_alphabet()
{
  expect_throws<GAFatalException>(
    []() {
      Population::Settings options = make_options(Population::OperationMode::Minimize, 10,
                                                  Population::VariableLengthMode::Fixed, 26);
      TravelingSalesman tsp(options, 20);
    },
    "TravelingSalesman should reject gene alphabets larger than the city table");
}

void test_f6_uses_standard_domain()
{
  F6 f6;
  // The all-ones code is the top of the domain on both axes.
  BaseString top = makeBinaryString(std::string(44, '1'));
  std::ostringstream out;
  f6.printCandidate(top, out);
  expect_contains(out.str(), "X ( 100 ) Y ( 100 )",
                  "F6 should map the largest code onto the top of [-100, 100]");

  // The code nearest the centre is the near-optimal point.
  BaseString centre = makeBinaryString("0111111111111111111111" "0111111111111111111111");
  expect_true(f6.evaluateFitness(centre) < 1.01,
              "F6 should score the centre of the domain near the optimum of 1");

  expect_throws<GAFatalException>(
    [&f6]() {
      Population::Settings bad = make_options(Population::OperationMode::Minimize, 43,
                                              Population::VariableLengthMode::Fixed, 2);
      Population invalid(bad, f6);
    },
    "F6 should reject chromosomes too short for two 22-bit axes");
}

void test_dome_reports_exact_optimum()
{
  Dome dome;
  Population::Settings options = make_options(Population::OperationMode::Minimize, 32,
                                              Population::VariableLengthMode::Fixed, 2);
  Population population(options, dome);
  BaseString origin = makeBinaryString(std::string(32, '0'));
  BaseString near = makeBinaryString(std::string(15, '0') + "1" + std::string(16, '0'));
  expect_true(dome.hasReachedSolution(population, origin, dome.evaluateFitness(origin)),
              "Dome should report the origin as the exact optimum");
  expect_true(dome.evaluateFitness(near) > 100.0 &&
              !dome.hasReachedSolution(population, near, dome.evaluateFitness(near)),
              "Dome should distinguish points near the origin from the optimum");

  expect_throws<GAFatalException>(
    [&dome]() {
      Population::Settings bad = make_options(Population::OperationMode::Minimize, 31,
                                              Population::VariableLengthMode::Fixed, 2);
      Population invalid(bad, dome);
    },
    "Dome should reject chromosomes shorter than 32 bits");
}

void test_latin_square_reports_solution()
{
  LatinSquare latin_square;
  Population::Settings options = make_options(Population::OperationMode::Maximize, 16,
                                              Population::VariableLengthMode::Fixed, 4);
  Population population(options, latin_square);
  BaseString solution(16, 4);
  BaseString almost(16, 4);
  const int solved_values[16] = {0, 1, 2, 3, 1, 2, 3, 0, 2, 3, 0, 1, 3, 0, 1, 2};
  for (int i = 0 ; i < 16 ; i++)
    {
      solution.setValue(i, solved_values[i]);
      almost.setValue(i, solved_values[i]);
    }
  almost.setValue(0, 1);
  expect_true(latin_square.hasReachedSolution(population, solution, latin_square.evaluateFitness(solution)),
              "LatinSquare should report a perfect square as solved");
  expect_true(!latin_square.hasReachedSolution(population, almost, latin_square.evaluateFitness(almost)),
              "LatinSquare should not report an imperfect square as solved");
}

void test_knapsack_validation_and_output()
{
  Knapsack knapsack;
  expect_throws<GAFatalException>(
    [&knapsack]() {
      Population::Settings bad = make_options(Population::OperationMode::Maximize, 13,
                                              Population::VariableLengthMode::Fixed, 2);
      Population invalid(bad, knapsack);
    },
    "Knapsack should reject chromosomes with more genes than items");

  BaseString genes = makeBinaryString("110000000000");
  std::ostringstream out;
  knapsack.printCandidate(genes, out);
  expect_true(out.str().size() >= 3 && out.str().compare(out.str().size() - 3, 3, " ::") == 0,
              "Knapsack output should end with a separator before the fitness value");
}

}

namespace {

// Gene index of an instruction in a one-input InstructionSet.
int geneFor(const InstructionSet& set, Operation operation, double constant = 0.0)
{
  for (int gene = 0 ; gene < set.size() ; gene++)
    {
      const Instruction& instruction = set.at(gene);
      if (instruction.operation == operation &&
          (operation != Operation::Constant || instruction.constant == constant))
        {
          return gene;
        }
    }
  return -1;
}

BaseString program(const InstructionSet& set, const std::vector<int>& genes)
{
  BaseString result(static_cast<int>(genes.size()), set.size());
  for (int i = 0 ; i < static_cast<int>(genes.size()) ; i++)
    {
      result.setValue(i, genes[i]);
    }
  return result;
}

Dataset makeDataset(double (*f)(double), int rows, double low, double high)
{
  Dataset data;
  data.source = "synthetic";
  data.inputNames.push_back("x");
  data.targetName = "y";
  for (int i = 0 ; i < rows ; i++)
    {
      const double x = low + (high - low) * i / (rows - 1);
      data.inputs.push_back(std::vector<double>(1, x));
      data.targets.push_back(f(x));
    }
  return data;
}

double quadraticTarget(double x) { return 3.0 * x * x + 2.0; }

void test_csv_dataset_loading()
{
  std::istringstream in("time_s, counts ,temp\n\n1, 10, 20\n2,20,21\n3,30,22\n");
  Dataset data = loadCsvDataset(in, "inline.csv");
  expect_true(data.rowCount() == 3 && data.inputCount() == 2,
              "CSV loader should read every row and treat all but the last column as inputs");
  expect_true(data.targetName == "temp" && data.inputNames[0] == "time_s" && data.inputNames[1] == "counts",
              "CSV loader should trim column names and default the target to the last column");
  expect_true(data.targets[2] == 22.0 && data.inputs[1][1] == 20.0,
              "CSV loader should parse numeric values into rows");

  std::istringstream named("a,b,c\n1,2,3\n4,5,6\n7,8,9\n");
  Dataset byName = loadCsvDataset(named, "named.csv", "a");
  expect_true(byName.targetName == "a" && byName.targets[1] == 4.0 &&
              byName.inputNames[0] == "b" && byName.inputs[1][0] == 5.0,
              "CSV loader should use the named target column and keep the rest as inputs");
}

void test_csv_dataset_errors()
{
  auto failsWith = [](const std::string& text, const std::string& needle, const std::string& target) {
    std::istringstream in(text);
    try
      {
        loadCsvDataset(in, "bad.csv", target);
      }
    catch (const GAFatalException& error)
      {
        return std::string(error.what()).find(needle) != std::string::npos;
      }
    return false;
  };
  expect_true(failsWith("x,y\n1,2\n3,abc\n5,6\n", "bad.csv:3", ""),
              "CSV loader should report the line of a non-numeric value");
  expect_true(failsWith("x,y\n1,2\n3\n5,6\n", "expected 2 values", ""),
              "CSV loader should reject ragged rows");
  expect_true(failsWith("x,y\n1,2\n3,4\n5,6\n", "'z' is not in the header", "z"),
              "CSV loader should reject an unknown target column");
  expect_true(failsWith("x,y\n1,2\n", "at least 3 data rows", ""),
              "CSV loader should reject files with too few rows");
  expect_true(failsWith("x\n1\n2\n3\n", "at least one input column", ""),
              "CSV loader should require an input and a target column");
}

void test_expression_compiles_postfix_programs()
{
  InstructionSet set(1);
  const int x = geneFor(set, Operation::Variable);
  const int two = geneFor(set, Operation::Constant, 2.0);
  const int three = geneFor(set, Operation::Constant, 3.0);
  const int add = geneFor(set, Operation::Add);
  const int mul = geneFor(set, Operation::Multiply);
  const int sinGene = geneFor(set, Operation::Sin);

  // "add" first underflows and is skipped; the program is x*x + 2.
  Expression expression = Expression::compile(program(set, {add, x, x, mul, two, add}), set);
  expect_true(expression.size() == 5, "Expression should keep only nodes that feed the result");
  expect_true(expression.toString({"x"}) == "x*x + 2", "Expression should print postfix code in infix");
  std::vector<double> values;
  expect_true(expression.evaluate({{3.0}, {-1.0}}, values) && values[0] == 11.0 && values[1] == 3.0,
              "Expression should evaluate every row");

  // Values not feeding the top of the stack are dropped: "3 x sin" leaves sin(x).
  Expression dangling = Expression::compile(program(set, {three, x, sinGene}), set);
  expect_true(dangling.size() == 2 && dangling.toString({"x"}) == "sin(x)",
              "Expression should ignore values left below the result");

  // Variable-free subtrees fold to a number.
  Expression folded = Expression::compile(program(set, {x, two, three, mul, add}), set);
  expect_true(folded.toString({"x"}) == "x + 6", "Expression should fold constant subtrees");

  Expression empty = Expression::compile(program(set, {add, mul}), set);
  expect_true(empty.empty() && !empty.evaluate({{1.0}}, values),
              "A program that never pushes a value should be empty and undefined");
}

void test_expression_rejects_undefined_values()
{
  InstructionSet set(1);
  const int x = geneFor(set, Operation::Variable);
  const int divide = geneFor(set, Operation::Divide);
  const int logGene = geneFor(set, Operation::Log);
  const int one = geneFor(set, Operation::Constant, 1.0);
  std::vector<double> values;

  Expression reciprocal = Expression::compile(program(set, {one, x, divide}), set);
  expect_true(reciprocal.evaluate({{2.0}}, values) && values[0] == 0.5,
              "Division should work away from zero");
  expect_true(!reciprocal.evaluate({{2.0}, {0.0}}, values),
              "Division by zero on any row should make the formula undefined");
  Expression logarithm = Expression::compile(program(set, {x, logGene}), set);
  expect_true(!logarithm.evaluate({{-1.0}}, values),
              "log of a non-positive value should make the formula undefined");
}

void test_regression_scaling_and_fit()
{
  SymbolicRegression::Config config;
  config.holdoutFraction = 0.25;
  SymbolicRegression regression(makeDataset(quadraticTarget, 40, -3.0, 3.0), config);
  expect_true(regression.trainRowCount() == 30 && regression.testRowCount() == 10,
              "Regression should hold out the configured share of rows");

  InstructionSet set(1);
  const int x = geneFor(set, Operation::Variable);
  const int square = geneFor(set, Operation::Square);
  // The bare shape x^2: linear scaling should supply 3 and 2 exactly.
  SymbolicRegression::Fit fit = regression.describe(program(set, {x, square}));
  expect_true(fit.valid && std::fabs(fit.slope - 3.0) < 1e-9 && std::fabs(fit.intercept - 2.0) < 1e-9,
              "Linear scaling should recover the slope and intercept of the right shape");
  expect_true(fit.trainR2 > 0.999999 && fit.testR2 > 0.999999,
              "The right shape should fit noiseless data perfectly");
  expect_true(std::fabs(fit.fitness - config.parsimony * 2) < 1e-9,
              "A perfect fit should cost only its parsimony penalty");
  expect_true(regression.scaledFormula(fit) == "2 + 3*(x^2)",
              "Scaled formula should show the fitted intercept and slope");

  // A wrong shape scores worse, and an undefined formula gets the penalty.
  SymbolicRegression::Fit line = regression.describe(program(set, {x}));
  expect_true(line.valid && line.fitness > fit.fitness, "A wrong shape should score worse");
  const int one = geneFor(set, Operation::Constant, 1.0);
  const int divide = geneFor(set, Operation::Divide);
  const int subtract = geneFor(set, Operation::Subtract);
  SymbolicRegression::Fit undefined = regression.describe(program(set, {one, x, x, subtract, divide}));
  expect_true(!undefined.valid && undefined.fitness == SymbolicRegression::kInvalidFitness,
              "A formula undefined on the data should get the invalid-fitness penalty");
}

void test_regression_operators_respect_length_limits()
{
  SymbolicRegression::Config config;
  config.maximumLength = 30;
  SymbolicRegression regression(makeDataset(quadraticTarget, 20, -1.0, 1.0), config);
  Population::Settings settings = make_options(Population::OperationMode::Minimize, 24,
                                               Population::VariableLengthMode::Variable,
                                               regression.alphabetSize());
  settings.crossOverRate = 1.0;
  Population population(settings, regression);
  for (int trial = 0 ; trial < 300 ; trial++)
    {
      std::unique_ptr<Chromosome> mother = PopulationTestRig::createInitialChromosome(population);
      std::unique_ptr<Chromosome> father = PopulationTestRig::createInitialChromosome(population);
      expect_true(mother->length() >= config.minimumInitialLength &&
                  mother->length() <= config.maximumInitialLength,
                  "Initial programs should have a length within the configured range");
      std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
        PopulationTestRig::mateChromosomes(population, *mother, *father);
      expect_true(children.first->length() <= config.maximumLength &&
                  children.second->length() <= config.maximumLength,
                  "Offspring should be truncated to the maximum program length");
    }

  expect_throws<GAFatalException>(
    [&regression]() {
      Population::Settings fixed = make_options(Population::OperationMode::Minimize, 24,
                                                Population::VariableLengthMode::Fixed,
                                                regression.alphabetSize());
      Population invalid(fixed, regression);
    },
    "Regression should require variable-length chromosomes");
}

double decayTarget(double x) { return 5.0 * std::exp(-0.3 * x); }

void test_regression_tunes_constants()
{
  InstructionSet set(1);
  const int x = geneFor(set, Operation::Variable);
  const int half = geneFor(set, Operation::Constant, 0.5);
  const int mul = geneFor(set, Operation::Multiply);
  const int negate = geneFor(set, Operation::Negate);
  const int expGene = geneFor(set, Operation::Exp);
  // The structure exp(-(x*0.5)) is right but its rate constant is not.
  const BaseString genes = program(set, {x, half, mul, negate, expGene});

  SymbolicRegression::Config untuned;
  untuned.constantTuningBudget = 0;
  SymbolicRegression plain(makeDataset(decayTarget, 40, 0.0, 10.0), untuned);
  SymbolicRegression::Config tunedConfig;
  tunedConfig.constantTuningBudget = 60;
  SymbolicRegression tuned(makeDataset(decayTarget, 40, 0.0, 10.0), tunedConfig);

  const SymbolicRegression::Fit before = plain.describe(genes);
  const SymbolicRegression::Fit after = tuned.describe(genes);
  expect_true(before.valid && before.trainR2 < 0.99,
              "Without tuning, the wrong rate constant should fit noticeably worse");
  expect_true(after.valid && after.trainR2 > 0.99999 && after.fitness < before.fitness,
              "Constant tuning should find the rate constant and lower the fitness");
  expect_true(after.formula.find("0.3") != std::string::npos,
              "The tuned formula should print the recovered constant 0.3");
  const SymbolicRegression::Fit again = tuned.describe(genes);
  expect_true(again.fitness == after.fitness && again.formula == after.formula,
              "Constant tuning should be deterministic");
}

double linearTarget(double x) { return 2.0 * x + 1.0; }

void test_regression_archive_and_cross_validated_selection()
{
  InstructionSet set(1);
  const int x = geneFor(set, Operation::Variable);
  const int square = geneFor(set, Operation::Square);
  const int two = geneFor(set, Operation::Constant, 2.0);
  const int multiply = geneFor(set, Operation::Multiply);
  const int sinGene = geneFor(set, Operation::Sin);
  const int add = geneFor(set, Operation::Add);
  SymbolicRegression::Config config;
  config.constantTuningBudget = 0;

  // Nothing evaluated yet: nothing to select.
  SymbolicRegression empty(makeDataset(linearTarget, 40, -3.0, 3.0), config);
  std::vector<int> genes;
  SymbolicRegression::Fit fit;
  expect_true(empty.crossValidationFolds() == 5 && !empty.selectByCrossValidation(genes, fit),
              "Selection needs at least one evaluated formula");

  // The archive keeps the best formula of each size.
  SymbolicRegression line(makeDataset(linearTarget, 40, -3.0, 3.0), config);
  line.evaluateFitness(program(set, {x, square}));       // size 2, a poor fit
  line.evaluateFitness(program(set, {x}));               // size 1, exact
  line.evaluateFitness(program(set, {two, x, multiply})); // size 3, also exact
  line.evaluateFitness(program(set, {x, sinGene}));       // size 2, better than x^2 on a line
  expect_true(line.archivedSizeCount() == 3, "The archive should hold one formula per size");

  // Equally good formulas: the smallest wins.
  expect_true(line.selectByCrossValidation(genes, fit) && fit.size == 1 && fit.formula == "x",
              "Cross-validation should prefer the smallest of equally good formulas");
  expect_true(fit.crossValidatedR2 > 0.999999 && fit.crossValidatedRmse < 1e-6,
              "The selected exact formula should have a perfect cross-validated score");

  // A clearly better, larger formula wins over a simpler one.
  SymbolicRegression curve(makeDataset(quadraticTarget, 40, -3.0, 3.0), config);
  curve.evaluateFitness(program(set, {x}));
  curve.evaluateFitness(program(set, {x, square}));
  expect_true(curve.selectByCrossValidation(genes, fit) && fit.size == 2 && fit.formula == "x^2",
              "Cross-validation should pick a larger formula that predicts clearly better");

  // A wiggle that fits training noise loses to the line it decorates. The
  // targets are a line plus deterministic noise, and sin(x*5) tracks none of
  // it on held-out folds.
  Dataset noisy = makeDataset(linearTarget, 60, -3.0, 3.0);
  std::mt19937 noiseGenerator(5);
  for (double& target : noisy.targets)
    {
      target += gak::uniformReal(noiseGenerator, -2.0, 2.0);
    }
  SymbolicRegression wiggly(noisy, config);
  const int five = geneFor(set, Operation::Constant, 5.0);
  wiggly.evaluateFitness(program(set, {x}));
  wiggly.evaluateFitness(program(set, {x, x, five, multiply, sinGene, add}));
  expect_true(wiggly.selectByCrossValidation(genes, fit) && fit.size == 1,
              "Cross-validation should reject a wiggle that does not predict held-out rows");

  // Disabled, and misconfigured.
  SymbolicRegression::Config off = config;
  off.crossValidationFolds = 0;
  SymbolicRegression disabled(makeDataset(linearTarget, 40, -3.0, 3.0), off);
  disabled.evaluateFitness(program(set, {x}));
  expect_true(disabled.crossValidationFolds() == 0 && !disabled.selectByCrossValidation(genes, fit),
              "Zero folds should disable selection");
  SymbolicRegression::Config negative = config;
  negative.crossValidationFolds = -1;
  expect_throws<GAFatalException>([&]() { SymbolicRegression bad(makeDataset(linearTarget, 40, -3.0, 3.0), negative); },
                                  "Negative fold counts should be rejected");
  SymbolicRegression::Config tooMany = config;
  tooMany.crossValidationFolds = 20;
  expect_throws<GAFatalException>([&]() { SymbolicRegression bad(makeDataset(linearTarget, 20, -3.0, 3.0), tooMany); },
                                  "More folds than the training rows support should be rejected");
  SymbolicRegression::Config badMargin = config;
  badMargin.selectionStandardErrors = -0.5;
  expect_throws<GAFatalException>([&]() { SymbolicRegression bad(makeDataset(linearTarget, 40, -3.0, 3.0), badMargin); },
                                  "A negative standard-error margin should be rejected");
}

void test_regression_sample_json_is_compact()
{
  SymbolicRegression::Config config;
  config.constantTuningBudget = 0;
  SymbolicRegression regression(makeDataset(quadraticTarget, 40, -3.0, 3.0), config);
  InstructionSet set(1);
  const BaseString genes = program(set, {geneFor(set, Operation::Variable), geneFor(set, Operation::Square)});
  std::ostringstream sample;
  regression.writeSampleJson(genes, sample);
  std::ostringstream full;
  regression.writeVisualizationJson(genes, full);
  expect_contains(sample.str(), "\"type\": \"regression_sample\"", "Sample payloads should be typed");
  expect_contains(sample.str(), "\"formula\": \"2 + 3*(x^2)\"", "Sample payloads should carry the formula");
  int separators = 0;
  for (std::size_t at = sample.str().find("], [") ; at != std::string::npos ; at = sample.str().find("], [", at + 1))
    {
      ++separators;
    }
  expect_true(sample.str().find("\"points\"") == std::string::npos && separators == 59,
              "Sample payloads should have a 60-point curve and no data points");
  expect_true(sample.str().size() * 3 < full.str().size(),
              "Sample payloads should be much smaller than the full visualization");
}

void test_regression_run_recovers_formula()
{
  SymbolicRegression regression(makeDataset(quadraticTarget, 40, -3.0, 3.0), SymbolicRegression::Config());
  Population::Settings settings;
  settings.operation = Population::OperationMode::Minimize;
  settings.numberOfIndividuals = 200;
  settings.numberOfTrials = 20000;
  settings.chromosomeLength = 24;
  settings.bitMutationRate = 0.04;
  settings.crossOverRate = 0.8;
  settings.reproduction = Population::ReproductionMode::DisallowDuplicates;
  settings.deletion = Population::DeletionMode::DeleteHalf;
  settings.fitness = Population::FitnessMode::LinearNormalized;
  settings.variableLength = Population::VariableLengthMode::Variable;
  settings.baseStates = regression.alphabetSize();
  settings.useFixedRandomSeed = true;
  settings.randomSeed = 1;
  Population population(settings, regression);
  Population::RunResult result = population.execute();
  expect_true(result.finalSummary.mostFit[0] < 0.02,
              "A seeded run should rediscover 3x^2 + 2 on noiseless data");
}

}

namespace {

int countOccurrences(const std::string& text, const std::string& needle)
{
  int count = 0;
  for (std::size_t at = text.find(needle) ; at != std::string::npos ; at = text.find(needle, at + 1)) ++count;
  return count;
}

void test_regression_output_and_visualization()
{
  SymbolicRegression::Config config;
  config.constantTuningBudget = 0;
  // These checks are about output, not selection; the 12-row two-input
  // dataset below is too small for 5-fold cross-validation.
  config.crossValidationFolds = 0;
  Dataset data = makeDataset(quadraticTarget, 20, -2.0, 2.0);
  data.targetName = "y \"quoted\"\tname";  // exercises JSON string escaping
  SymbolicRegression regression(data, config);
  InstructionSet set(1);
  const int x = geneFor(set, Operation::Variable);
  const int square = geneFor(set, Operation::Square);
  const BaseString genes = program(set, {x, square});

  std::ostringstream printed;
  regression.printCandidate(genes, printed);
  expect_contains(printed.str(), "~= 2 + 3*(x^2)", "printCandidate should show the scaled formula");
  expect_contains(printed.str(), "nodes=2", "printCandidate should show the formula size");
  expect_contains(printed.str(), "| test R2=1.0000", "printCandidate should show the held-out score");
  expect_true(printed.str().size() >= 3 && printed.str().compare(printed.str().size() - 3, 3, " ::") == 0,
              "printCandidate should end with the fitness separator");

  std::ostringstream json;
  regression.writeVisualizationJson(genes, json);
  const std::string text = json.str();
  expect_contains(text, "\"type\": \"regression\"", "Regression visualization should be typed");
  expect_contains(text, "\"target\": \"y \\\"quoted\\\"\\u0009name\"",
                  "Regression visualization should escape quotes and control characters");
  expect_contains(text, "\"formula\": \"2 + 3*(x^2)\"", "Regression visualization should include the formula");
  expect_true(countOccurrences(text, "\"set\": ") == 20 &&
              countOccurrences(text, "\"set\": \"test\"") == regression.testRowCount(),
              "Regression visualization should label every point as train or test");
  expect_true(countOccurrences(text, "\"predicted\": ") == 20,
              "Regression visualization should include a prediction for every point");
  expect_true(countOccurrences(text, "{ \"x\": ") == 200,
              "Regression visualization should sample the fitted curve for one input");

  // An undefined formula still produces a valid payload: no predictions, no curve.
  const int one = geneFor(set, Operation::Constant, 1.0);
  const int subtract = geneFor(set, Operation::Subtract);
  const int divide = geneFor(set, Operation::Divide);
  const BaseString undefined = program(set, {one, x, x, subtract, divide});
  std::ostringstream badJson;
  regression.writeVisualizationJson(undefined, badJson);
  expect_true(countOccurrences(badJson.str(), "\"predicted\"") == 0 &&
              badJson.str().find("\"curve\": [] }") != std::string::npos,
              "An undefined formula should have no predictions and no curve");
  std::ostringstream badPrinted;
  regression.printCandidate(undefined, badPrinted);
  expect_contains(badPrinted.str(), "(undefined on the data) ::",
                  "printCandidate should say when a formula is undefined");

  // Two inputs: points carry both inputs and there is no curve.
  Dataset twoInputs;
  twoInputs.source = "two";
  twoInputs.inputNames = {"a", "b"};
  twoInputs.targetName = "y";
  for (int i = 0 ; i < 12 ; i++)
    {
      twoInputs.inputs.push_back({static_cast<double>(i), static_cast<double>(i % 3)});
      twoInputs.targets.push_back(i * (i % 3) + 1.0);
    }
  SymbolicRegression multi(twoInputs, config);
  InstructionSet multiSet(2);
  std::ostringstream multiJson;
  multi.writeVisualizationJson(program(multiSet, {0, 1, geneFor(multiSet, Operation::Multiply)}), multiJson);
  expect_contains(multiJson.str(), "\"inputs\": [\"a\", \"b\"]", "Visualization should list every input");
  expect_true(multiJson.str().find("\"curve\": [] }") != std::string::npos,
              "Multi-input visualizations should not include a curve");
}

void test_regression_formula_variants_and_holdout()
{
  SymbolicRegression::Config config;
  config.constantTuningBudget = 0;
  InstructionSet set(1);
  const int x = geneFor(set, Operation::Variable);
  const int negate = geneFor(set, Operation::Negate);
  const int two = geneFor(set, Operation::Constant, 2.0);

  // y = -x exactly: no intercept, negative slope on f = x.
  Dataset line = makeDataset([](double v) { return -v; }, 20, -3.0, 3.0);
  SymbolicRegression lineRegression(line, config);
  SymbolicRegression::Fit fit = lineRegression.describe(program(set, {x}));
  expect_true(lineRegression.scaledFormula(fit) == "-(x)",
              "A pure negative slope should print without an intercept");
  fit = lineRegression.describe(program(set, {x, negate}));
  expect_true(lineRegression.scaledFormula(fit) == "(-x)",
              "A unit slope should print without a factor");

  // A constant formula carries no shape: it falls back to the mean.
  SymbolicRegression::Fit constant = lineRegression.describe(program(set, {two}));
  expect_true(constant.valid && constant.slope == 0.0 && constant.trainR2 < 1e-9,
              "A constant formula should reduce to predicting the mean");
  expect_true(lineRegression.scaledFormula(constant).find('(') == std::string::npos,
              "A constant formula should print as just a number");

  // No held-out rows: everything trains and the test score is not printed.
  SymbolicRegression::Config noHoldout = config;
  noHoldout.holdoutFraction = 0.0;
  SymbolicRegression all(line, noHoldout);
  expect_true(all.testRowCount() == 0 && all.trainRowCount() == 20, "Holdout 0 should train on every row");
  std::ostringstream printed;
  all.printCandidate(program(set, {x}), printed);
  expect_true(printed.str().find("test R2") == std::string::npos,
              "With no held-out rows the test score should not be printed");

  // A formula defined on every training row but undefined on a test row
  // reports no predictive value on the test split. The split depends only
  // on the row count, so find a test row, then shift x so 1/(x - 2) is
  // undefined exactly there.
  SymbolicRegression probe(makeDataset(quadraticTarget, 24, 0.0, 23.0), config);
  std::ostringstream probeJson;
  probe.writeVisualizationJson(program(set, {x}), probeJson);
  const std::size_t firstTest = probeJson.str().find("\"set\": \"test\"");
  const int testRow = countOccurrences(probeJson.str().substr(0, firstTest), "\"set\": ");
  Dataset shifted;
  shifted.source = "shifted";
  shifted.inputNames = {"x"};
  shifted.targetName = "y";
  for (int i = 0 ; i < 24 ; i++)
    {
      const double value = i - testRow + 2.0;
      shifted.inputs.push_back(std::vector<double>(1, value));
      shifted.targets.push_back(value * value);
    }
  SymbolicRegression testOnly(shifted, config);
  const int one = geneFor(set, Operation::Constant, 1.0);
  const int subtract = geneFor(set, Operation::Subtract);
  const int divide = geneFor(set, Operation::Divide);
  SymbolicRegression::Fit testUndefined = testOnly.describe(program(set, {one, x, two, subtract, divide}));
  expect_true(testUndefined.valid && testUndefined.testR2 == 0.0 && testUndefined.testRmse > 0.0,
              "A formula undefined only on held-out rows should report no test predictive value");
}

void test_regression_configuration_errors()
{
  Dataset data = makeDataset(quadraticTarget, 20, -2.0, 2.0);
  auto rejects = [&](SymbolicRegression::Config config, const Dataset& dataset, const std::string& message) {
    expect_throws<GAFatalException>([&]() { SymbolicRegression regression(dataset, config); }, message);
  };
  SymbolicRegression::Config holdout;
  holdout.holdoutFraction = 1.0;
  rejects(holdout, data, "A holdout fraction of 1 should be rejected");
  SymbolicRegression::Config lengths;
  lengths.maximumInitialLength = 2;
  lengths.minimumInitialLength = 5;
  rejects(lengths, data, "Inconsistent program lengths should be rejected");
  SymbolicRegression::Config growth;
  growth.maximumLength = 10;
  rejects(growth, data, "A maximum length below the initial length should be rejected");
  SymbolicRegression::Config mostlyHeldOut;
  mostlyHeldOut.holdoutFraction = 0.9;
  rejects(mostlyHeldOut, makeDataset(quadraticTarget, 10, 0.0, 1.0), "Fewer than 3 training rows should be rejected");
  rejects(SymbolicRegression::Config(), makeDataset([](double) { return 4.0; }, 10, 0.0, 1.0),
          "A constant target should be rejected");

  SymbolicRegression regression(data, SymbolicRegression::Config());
  expect_throws<GAFatalException>(
    [&]() {
      Population::Settings settings = make_options(Population::OperationMode::Minimize, 10,
                                                   Population::VariableLengthMode::Variable, 5);
      Population population(settings, regression);
    },
    "Regression should reject a gene alphabet of the wrong size");
}

void test_dataset_file_and_format_edges()
{
  Dataset file = loadCsvDataset("data/linear_noisy.csv");
  expect_true(file.rowCount() == 60 && file.targetName == "y" && file.source == "data/linear_noisy.csv",
              "CSV loader should read a file from disk");
  auto failsWith = [](const std::function<void()>& load, const std::string& needle) {
    try { load(); }
    catch (const GAFatalException& error) { return std::string(error.what()).find(needle) != std::string::npos; }
    return false;
  };
  expect_true(failsWith([]() { loadCsvDataset("data/does_not_exist.csv"); }, "cannot open file"),
              "CSV loader should report a missing file");
  expect_true(failsWith([]() { std::istringstream in("\n  \n"); loadCsvDataset(in, "blank.csv"); },
                        "no header row"),
              "CSV loader should reject a file with no header");
  expect_true(failsWith([]() { std::istringstream in("x,,y\n1,2,3\n"); loadCsvDataset(in, "e.csv"); },
                        "empty column name"),
              "CSV loader should reject an empty column name");
  expect_true(failsWith([]() { std::istringstream in("x,y\n1,2,\n3,4\n5,6\n"); loadCsvDataset(in, "t.csv"); },
                        "t.csv:2: expected 2 values, found 3"),
              "CSV loader should count a trailing comma as an extra empty value");
  expect_true(failsWith([]() { std::istringstream in("x,y\n1,\n3,4\n5,6\n"); loadCsvDataset(in, "m.csv"); },
                        "non-numeric value ''"),
              "CSV loader should reject an empty value");
  expect_true(failsWith([]() { std::istringstream in("x,y\n1,inf\n3,4\n5,6\n"); loadCsvDataset(in, "i.csv"); },
                        "non-numeric value 'inf'"),
              "CSV loader should reject non-finite values");
}

void test_expression_construction_errors()
{
  expect_throws<GAFatalException>([]() { InstructionSet none(0); },
                                  "An instruction set needs at least one input");
  InstructionSet set(1);
  expect_throws<GAFatalException>([&set]() { Expression::compile(BaseString(4, set.size() + 1), set); },
                                  "Compiling genes from a different alphabet should be rejected");
}

void test_example_printers_and_edges()
{
  Spell spell;
  expect_true(spell.evaluateFitness(makeSymbolicString("eggheadx")) < spell.evaluateFitness(makeSymbolicString("egghead")),
              "Spell should penalize letters beyond the target length");
  std::ostringstream spelled;
  spell.printCandidate(makeSymbolicString("egghead"), spelled);
  expect_true(spelled.str() == "egghead ::", "Spell should print the candidate word");

  NQueens queens;
  BaseString diagonal(8, 8);
  for (int column = 0 ; column < 8 ; column++) diagonal.setValue(column, column);
  std::ostringstream queensJson;
  queens.writeVisualizationJson(diagonal, queensJson);
  expect_true(countOccurrences(queensJson.str(), "\"same_diagonal\": true") == 28,
              "N-Queens visualization should list every diagonal conflict");

  Population::Settings sudokuOptions = make_options(Population::OperationMode::Maximize, 81,
                                                    Population::VariableLengthMode::Fixed, 9);
  SudokuConstrained sudoku(sudokuOptions);
  Population sudokuPopulation(sudokuOptions, sudoku);
  std::unique_ptr<Chromosome> board = PopulationTestRig::createInitialChromosome(sudokuPopulation);
  std::ostringstream boardOut;
  sudoku.printCandidate(board->genes(), boardOut);
  expect_contains(boardOut.str(), "Constraint-aware Sudoku candidate:", "Constrained Sudoku should label its board");
  expect_true(countOccurrences(boardOut.str(), "------+-------+------") == 2 &&
              countOccurrences(boardOut.str(), " | ") == 18,
              "Constrained Sudoku should print box separators");

  // TSP: without crossover, children are copies of the parents.
  Population::Settings tspOptions = make_options(Population::OperationMode::Minimize, 6,
                                                 Population::VariableLengthMode::Fixed, 6);
  tspOptions.crossOverRate = 0.0;
  TravelingSalesman tsp(tspOptions, 10);
  Population tspPopulation(tspOptions, tsp);
  std::unique_ptr<Chromosome> mother = PopulationTestRig::createInitialChromosome(tspPopulation);
  std::unique_ptr<Chromosome> father = PopulationTestRig::createInitialChromosome(tspPopulation);
  std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
    PopulationTestRig::mateChromosomes(tspPopulation, *mother, *father);
  expect_true((children.first->equals(*mother) && children.second->equals(*father)) ||
              (children.first->equals(*father) && children.second->equals(*mother)),
              "TSP mating without crossover should copy the parents");

}

}

int main()
{
  test_dome_fitness();
  test_f6_fitness_is_positive();
  test_spell_fitness_matches_target_word();
  test_alpha_prefers_sorted_alphabet();
  test_alpha_rejects_mismatched_population_settings();
  test_traveling_salesman_construction_and_validation();
  test_traveling_salesman_fixed_seed_is_reproducible();
  test_traveling_salesman_visualization_json();
  test_nqueens_rewards_non_attacking_layouts();
  test_nqueens_visualization_json();
  test_knapsack_prefers_feasible_high_value_selections();
  test_knapsack_visualization_json();
  test_latin_square_rewards_unique_rows_and_columns();
  test_latin_square_validation_rejects_bad_settings();
  test_latin_square_visualization_json();
  test_latin_square_run_path();
  test_graph_coloring_rewards_proper_colorings();
  test_graph_coloring_validation_rejects_bad_settings();
  test_graph_coloring_visualization_json();
  test_timetable_rewards_conflict_free_preferred_assignments();
  test_timetable_validation_rejects_bad_settings();
  test_timetable_visualization_json();
  test_sudoku_rewards_valid_solution_and_givens();
  test_sudoku_validation_rejects_bad_settings();
  test_sudoku_visualization_json();
  test_sudoku_run_path();
  test_constrained_sudoku_preserves_row_structure_and_givens();
  test_constrained_sudoku_run_path();
  test_constrained_sudoku_fixed_seed_is_reproducible();
  test_constrained_sudoku_visualization_json();
  test_constrained_sudoku_validation_rejects_bad_settings();
  test_traveling_salesman_operators_keep_tours_valid();
  test_traveling_salesman_rejects_mismatched_alphabet();
  test_f6_uses_standard_domain();
  test_dome_reports_exact_optimum();
  test_latin_square_reports_solution();
  test_knapsack_validation_and_output();
  test_csv_dataset_loading();
  test_csv_dataset_errors();
  test_expression_compiles_postfix_programs();
  test_expression_rejects_undefined_values();
  test_regression_scaling_and_fit();
  test_regression_operators_respect_length_limits();
  test_regression_run_recovers_formula();
  test_regression_tunes_constants();
  test_regression_archive_and_cross_validated_selection();
  test_regression_sample_json_is_compact();
  test_regression_output_and_visualization();
  test_regression_formula_variants_and_holdout();
  test_regression_configuration_errors();
  test_dataset_file_and_format_edges();
  test_expression_construction_errors();
  test_example_printers_and_edges();

  if (g_failures != 0)
    {
      std::cerr << g_failures << " example test(s) failed" << std::endl;
      return 1;
    }

  std::cout << "All example tests passed" << std::endl;
  return 0;
}
