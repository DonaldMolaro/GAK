#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <genetic.hh>

#include "alpha.hh"
#include "dome.hh"
#include "f6.hh"
#include "knapsack.hh"
#include "latinsquare.hh"
#include "nqueens.hh"
#include "spell.hh"
#include "sudoku.hh"
#include "sudoku_constrained.hh"
#include "ts.hh"
#include "population_report.hh"

namespace
{
struct CliOptions
{
  std::string mode;
  bool showSettings = false;
  bool seedProvided = false;
  unsigned int seed = 0;
  bool populationProvided = false;
  int population = 0;
  bool trialsProvided = false;
  int trials = 0;
  bool mutationProvided = false;
  double mutation = 0.0;
  bool crossoverProvided = false;
  double crossover = 0.0;
  bool gridProvided = false;
  int grid = 0;
};

Population::Settings make_dome_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Minimize;
  options.numberOfIndividuals = 100;
  options.numberOfTrials = 4000;
  options.geneticDiversity = 32;
  options.bitMutationRate = 0.008;
  options.crossOverRate = 0.65;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Fixed;
  options.baseStates = 2;
  return options;
}

Population::Settings make_f6_options()
{
  Population::Settings options = make_dome_options();
  options.numberOfTrials = 5000;
  options.geneticDiversity = 44;
  return options;
}

Population::Settings make_alpha_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 1000;
  options.numberOfTrials = 160000;
  options.geneticDiversity = 26;
  options.bitMutationRate = 0.008;
  options.crossOverRate = 0.65;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Fixed;
  options.baseStates = 26;
  return options;
}

Population::Settings make_spell_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 100;
  options.numberOfTrials = 8000;
  options.geneticDiversity = 7;
  options.bitMutationRate = 0.008;
  options.crossOverRate = 0.65;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Variable;
  options.baseStates = 26;
  return options;
}

Population::Settings make_traveling_salesman_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Minimize;
  options.numberOfIndividuals = 500;
  options.numberOfTrials = 200000;
  options.geneticDiversity = 26;
  options.bitMutationRate = 0.008;
  options.crossOverRate = 0.65;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Fixed;
  options.baseStates = 26;
  return options;
}

Population::Settings make_nqueens_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 250;
  options.numberOfTrials = 15000;
  options.geneticDiversity = 8;
  options.bitMutationRate = 0.015;
  options.crossOverRate = 0.75;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Fixed;
  options.baseStates = 8;
  return options;
}

Population::Settings make_knapsack_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 150;
  options.numberOfTrials = 12000;
  options.geneticDiversity = 12;
  options.bitMutationRate = 0.01;
  options.crossOverRate = 0.70;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Fixed;
  options.baseStates = 2;
  return options;
}

Population::Settings make_latin_square_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 250;
  options.numberOfTrials = 15000;
  options.geneticDiversity = 25;
  options.bitMutationRate = 0.02;
  options.crossOverRate = 0.75;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Fixed;
  options.baseStates = 5;
  return options;
}

Population::Settings make_sudoku_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 400;
  options.numberOfTrials = 20000;
  options.geneticDiversity = 81;
  options.bitMutationRate = 0.02;
  options.crossOverRate = 0.75;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Fixed;
  options.baseStates = 9;
  return options;
}

Population::Settings make_constrained_sudoku_options()
{
  Population::Settings options = make_sudoku_options();
  options.numberOfIndividuals = 250;
  options.numberOfTrials = 12000;
  options.bitMutationRate = 0.05;
  options.crossOverRate = 0.80;
  return options;
}

void usage(std::ostream& out)
{
  out << "Usage:\n";
  out << "  GA MODE [options]\n\n";
  out << "Modes:\n";
  out << "  D   Dome Function\n";
  out << "  6   F6 Function\n";
  out << "  A   Alphabet\n";
  out << "  S   Spell\n";
  out << "  T   Traveling Salesman\n";
  out << "  Q   8 Queens\n";
  out << "  K   Knapsack\n";
  out << "  L   Latin Square\n";
  out << "  U   Sudoku\n";
  out << "  C   Sudoku+\n\n";
  out << "Options:\n";
  out << "  --seed N          Use a fixed random seed\n";
  out << "  --population N    Override population size\n";
  out << "  --trials N        Override trial/evaluation budget\n";
  out << "  --mutation R      Override mutation rate\n";
  out << "  --crossover R     Override crossover rate\n";
  out << "  --grid N          Override TSP grid size\n";
  out << "  --show-settings   Print the resolved settings report before the final summary\n";
  out << "  --help            Show this help\n";
}

bool parseInt(const std::string& text, int& value)
{
  std::istringstream stream(text);
  stream >> value;
  return stream.good() || stream.eof();
}

bool parseUnsigned(const std::string& text, unsigned int& value)
{
  std::istringstream stream(text);
  stream >> value;
  return stream.good() || stream.eof();
}

bool parseDouble(const std::string& text, double& value)
{
  std::istringstream stream(text);
  stream >> value;
  return stream.good() || stream.eof();
}

bool parseCliOptions(int argc, char* argv[], CliOptions& options)
{
  if (argc < 2)
  {
    return false;
  }

  options.mode = argv[1];
  if (options.mode == "--help")
  {
    return false;
  }

  for (int i = 2 ; i < argc ; i++)
  {
    const std::string arg = argv[i];
    if (arg == "--help")
    {
      return false;
    }
    if (arg == "--show-settings")
    {
      options.showSettings = true;
      continue;
    }
    if (i + 1 >= argc)
    {
      return false;
    }

    const std::string value = argv[++i];
    if (arg == "--seed")
    {
      options.seedProvided = parseUnsigned(value, options.seed);
      if (!options.seedProvided) return false;
    }
    else if (arg == "--population")
    {
      options.populationProvided = parseInt(value, options.population);
      if (!options.populationProvided) return false;
    }
    else if (arg == "--trials")
    {
      options.trialsProvided = parseInt(value, options.trials);
      if (!options.trialsProvided) return false;
    }
    else if (arg == "--mutation")
    {
      options.mutationProvided = parseDouble(value, options.mutation);
      if (!options.mutationProvided) return false;
    }
    else if (arg == "--crossover")
    {
      options.crossoverProvided = parseDouble(value, options.crossover);
      if (!options.crossoverProvided) return false;
    }
    else if (arg == "--grid")
    {
      options.gridProvided = parseInt(value, options.grid);
      if (!options.gridProvided) return false;
    }
    else
    {
      return false;
    }
  }

  return options.mode.length() == 1;
}

Population::Settings applyOverrides(Population::Settings options, const CliOptions& cli)
{
  if (cli.seedProvided)
  {
    options.useFixedRandomSeed = true;
    options.randomSeed = cli.seed;
  }
  if (cli.populationProvided)
  {
    options.numberOfIndividuals = cli.population;
  }
  if (cli.trialsProvided)
  {
    options.numberOfTrials = cli.trials;
  }
  if (cli.mutationProvided)
  {
    options.bitMutationRate = cli.mutation;
  }
  if (cli.crossoverProvided)
  {
    options.crossOverRate = cli.crossover;
  }
  return options;
}

int runPopulation(Population& population, bool showSettings)
{
  if (showSettings)
  {
    Population::RunResult result = population.execute(false);
    PopulationReporter::write(std::cout,
                                   population,
                                   result,
                                   PopulationRunReportOptions(true, false));
  }
  else
  {
    population.run();
  }
  return EXIT_SUCCESS;
}
}

int main(int argc,char *argv[])
{
  CliOptions cli;
  if (!parseCliOptions(argc, argv, cli))
  {
    usage(std::cerr);
    return EXIT_FAILURE;
  }

  switch (cli.mode[0])
  {
    case 'd':
    case 'D':
      {
        Dome dome;
        Population population(applyOverrides(make_dome_options(), cli), dome);
        return runPopulation(population, cli.showSettings);
      }
    case '6':
      {
        F6 f6;
        Population population(applyOverrides(make_f6_options(), cli), f6);
        return runPopulation(population, cli.showSettings);
      }
    case 'A':
    case 'a':
      {
        Alpha alpha(applyOverrides(make_alpha_options(), cli));
        return runPopulation(alpha, cli.showSettings);
      }
    case 'S':
    case 's':
      {
        Spell spell;
        Population population(applyOverrides(make_spell_options(), cli), spell);
        return runPopulation(population, cli.showSettings);
      }
    case 'T':
    case 't':
      {
        const int gridSize = cli.gridProvided ? cli.grid : 500;
        TravelingSalesman travelingSalesman(applyOverrides(make_traveling_salesman_options(), cli), gridSize);
        travelingSalesman.writeCityList(std::cout);
        return runPopulation(travelingSalesman, cli.showSettings);
      }
    case 'Q':
    case 'q':
      {
        NQueens queens;
        Population population(applyOverrides(make_nqueens_options(), cli), queens);
        return runPopulation(population, cli.showSettings);
      }
    case 'K':
    case 'k':
      {
        Knapsack knapsack;
        Population population(applyOverrides(make_knapsack_options(), cli), knapsack);
        return runPopulation(population, cli.showSettings);
      }
    case 'L':
    case 'l':
      {
        LatinSquare latinSquare;
        Population population(applyOverrides(make_latin_square_options(), cli), latinSquare);
        return runPopulation(population, cli.showSettings);
      }
    case 'U':
    case 'u':
      {
        Sudoku sudoku;
        Population population(applyOverrides(make_sudoku_options(), cli), sudoku);
        return runPopulation(population, cli.showSettings);
      }
    case 'C':
    case 'c':
      {
        SudokuConstrained sudoku(applyOverrides(make_constrained_sudoku_options(), cli));
        return runPopulation(sudoku, cli.showSettings);
      }
    default:
      usage(std::cerr);
      return EXIT_FAILURE;
  }
}
