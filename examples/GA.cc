#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <genetic.hh>

#include "alpha.hh"
#include "dome.hh"
#include "f6.hh"
#include "graphcoloring.hh"
#include "knapsack.hh"
#include "latinsquare.hh"
#include "nqueens.hh"
#include "spell.hh"
#include "sudoku.hh"
#include "sudoku_constrained.hh"
#include "timetable.hh"
#include "ts.hh"
#include "population_report.hh"

namespace
{
const int kKnapsackWeights[] = {2, 3, 5, 7, 1, 4, 1, 6, 9, 8, 3, 5};
const int kKnapsackValues[] = {6, 5, 8, 9, 6, 7, 3, 7, 12, 10, 4, 8};
const int kSudokuPuzzle[] = {
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
  std::string reportJsonPath;
  std::string reportCsvPath;
};

void printSectionHeading(const std::string& title)
{
  std::cout << title << '\n';
}

void printLabeledLine(const std::string& label, const std::string& value)
{
  std::cout << "  " << std::left << std::setw(18) << label << ": " << value << '\n';
}

void printLabeledLine(const std::string& label, int value)
{
  std::cout << "  " << std::left << std::setw(18) << label << ": " << value << '\n';
}

void printLabeledLine(const std::string& label, double value)
{
  std::cout << "  " << std::left << std::setw(18) << label << ": " << value << '\n';
}

void finishInputSection()
{
  std::cout << std::flush;
}

void printSudokuPuzzle()
{
  printSectionHeading("Problem input:");
  std::cout << "Sudoku givens:\n";
  for (int row = 0 ; row < 9 ; row++)
  {
    for (int column = 0 ; column < 9 ; column++)
    {
      const int value = kSudokuPuzzle[(row * 9) + column];
      std::cout << (value == 0 ? '.' : static_cast<char>('0' + value));
      if (column == 2 || column == 5)
      {
        std::cout << " | ";
      }
      else if (column + 1 < 9)
      {
        std::cout << ' ';
      }
    }
    std::cout << '\n';
    if (row == 2 || row == 5)
    {
      std::cout << "------+-------+------\n";
    }
  }
  std::cout << '\n';
  finishInputSection();
}

void printDomeInput()
{
  printSectionHeading("Problem input:");
  printLabeledLine("problem", "Dome objective");
  printLabeledLine("encoding", "16 bits for X, 16 bits for Y");
  printLabeledLine("scaling", "decoded coordinates / 100");
  std::cout << '\n';
  finishInputSection();
}

void printF6Input()
{
  printSectionHeading("Problem input:");
  printLabeledLine("problem", "F6 objective");
  printLabeledLine("encoding", "22 bits for X, 22 bits for Y");
  printLabeledLine("scaling", "decoded / 1000, then shift by -100");
  std::cout << '\n';
  finishInputSection();
}

void printAlphaInput()
{
  printSectionHeading("Problem input:");
  printLabeledLine("target", "sorted 26-letter alphabet");
  printLabeledLine("symbols", "a b c d e f g h i j k l m");
  printLabeledLine("", "n o p q r s t u v w x y z");
  std::cout << '\n';
  finishInputSection();
}

void printSpellInput()
{
  printSectionHeading("Problem input:");
  printLabeledLine("target word", "egghead");
  printLabeledLine("alphabet", "a-z");
  std::cout << '\n';
  finishInputSection();
}

void printNQueensInput()
{
  printSectionHeading("Problem input:");
  printLabeledLine("board", "8x8");
  printLabeledLine("encoding", "one queen per column");
  printLabeledLine("gene meaning", "row index for each column");
  std::cout << '\n';
  finishInputSection();
}

void printKnapsackInput()
{
  printSectionHeading("Problem input:");
  printLabeledLine("capacity", 35);
  std::cout << "  items:\n";
  for (int i = 0 ; i < 12 ; i++)
  {
    std::cout << "    "
              << std::left << std::setw(8) << ("item " + std::to_string(i))
              << " weight=" << std::setw(2) << kKnapsackWeights[i]
              << " value=" << kKnapsackValues[i] << '\n';
  }
  std::cout << '\n';
  finishInputSection();
}

void printLatinSquareInput()
{
  printSectionHeading("Problem input:");
  printLabeledLine("board", "5x5 Latin square");
  printLabeledLine("symbols", "0 1 2 3 4");
  std::cout << '\n';
  finishInputSection();
}

void printGraphColoringInput()
{
  printSectionHeading("Problem input:");
  printLabeledLine("problem", "3-color graph coloring");
  printLabeledLine("nodes", "A B C D E F G H");
  printLabeledLine("colors", "red green blue");
  printLabeledLine("edges", "A-B B-C C-D D-E E-F F-A");
  printLabeledLine("", "G-A G-C G-E H-B H-D H-F");
  std::cout << '\n';
  finishInputSection();
}

void printTimetableInput()
{
  printSectionHeading("Problem input:");
  printLabeledLine("problem", "course timetabling");
  printLabeledLine("slots", "0 1 2 3");
  printLabeledLine("courses", "Calculus Physics Chemistry Biology");
  printLabeledLine("", "History Literature Economics Programming");
  printLabeledLine("hard conflicts", "Calc-Phys Calc-Bio Phys-Chem");
  printLabeledLine("", "Phys-Prog Chem-Hist Chem-Econ");
  printLabeledLine("", "Bio-Lit Bio-Prog Hist-Econ Lit-Prog");
  printLabeledLine("soft goal", "match preferred slot for each course");
  std::cout << '\n';
  finishInputSection();
}

const char* operationLabel(Population::OperationMode mode)
{
  return mode == Population::OperationMode::Minimize ? "minimize" : "maximize";
}

const char* reproductionLabel(Population::ReproductionMode mode)
{
  return mode == Population::ReproductionMode::DisallowDuplicates ? "disallow-duplicates" : "allow-duplicates";
}

const char* selectionLabel(Population::ParentSelectionMode mode)
{
  return mode == Population::ParentSelectionMode::Random ? "random" : "roulette";
}

const char* deletionLabel(Population::DeletionMode mode)
{
  switch (mode)
  {
    case Population::DeletionMode::DeleteAll: return "delete-all";
    case Population::DeletionMode::DeleteAllButBest: return "delete-all-but-best";
    case Population::DeletionMode::DeleteHalf: return "delete-half";
    case Population::DeletionMode::DeleteQuarter: return "delete-quarter";
    case Population::DeletionMode::DeleteLast: return "delete-last";
  }
  return "unknown";
}

const char* fitnessLabel(Population::FitnessMode mode)
{
  switch (mode)
  {
    case Population::FitnessMode::Evaluation: return "evaluation";
    case Population::FitnessMode::Windowed: return "windowed";
    case Population::FitnessMode::LinearNormalized: return "linear-normalized";
  }
  return "unknown";
}

const char* variableLengthLabel(Population::VariableLengthMode mode)
{
  return mode == Population::VariableLengthMode::Variable ? "variable" : "fixed";
}

void printGaParameters(const Population::Settings& settings)
{
  printSectionHeading("GA parameters:");
  printLabeledLine("operation", operationLabel(settings.operation));
  printLabeledLine("population", settings.numberOfIndividuals);
  printLabeledLine("trials", settings.numberOfTrials);
  printLabeledLine("chromosome length", settings.chromosomeLength);
  printLabeledLine("base states", settings.baseStates);
  printLabeledLine("mutation", settings.bitMutationRate);
  printLabeledLine("crossover", settings.crossOverRate);
  printLabeledLine("reproduction", reproductionLabel(settings.reproduction));
  printLabeledLine("selection", selectionLabel(settings.parentSelection));
  printLabeledLine("deletion", deletionLabel(settings.deletion));
  printLabeledLine("fitness", fitnessLabel(settings.fitness));
  printLabeledLine("length mode", variableLengthLabel(settings.variableLength));
  if (settings.useFixedRandomSeed)
  {
    printLabeledLine("seed", std::to_string(settings.randomSeed) + " (fixed)");
  }
  else
  {
    printLabeledLine("seed", "generated at runtime");
  }
  std::cout << '\n';
  finishInputSection();
}

Population::Settings make_dome_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Minimize;
  options.numberOfIndividuals = 100;
  options.numberOfTrials = 4000;
  options.chromosomeLength = 32;
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
  options.chromosomeLength = 44;
  return options;
}

Population::Settings make_alpha_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 1000;
  options.numberOfTrials = 160000;
  options.chromosomeLength = 26;
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
  options.chromosomeLength = 7;
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
  options.chromosomeLength = 26;
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
  options.chromosomeLength = 8;
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
  options.chromosomeLength = 12;
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
  options.chromosomeLength = 25;
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
  options.chromosomeLength = 81;
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

Population::Settings make_graph_coloring_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 120;
  options.numberOfTrials = 7000;
  options.chromosomeLength = 8;
  options.bitMutationRate = 0.02;
  options.crossOverRate = 0.75;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Fixed;
  options.baseStates = 3;
  return options;
}

Population::Settings make_timetable_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 150;
  options.numberOfTrials = 9000;
  options.chromosomeLength = 8;
  options.bitMutationRate = 0.02;
  options.crossOverRate = 0.75;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Fixed;
  options.baseStates = 4;
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
  out << "  G   Graph Coloring\n";
  out << "  M   Timetable\n";
  out << "  U   Sudoku\n";
  out << "  C   Sudoku+\n\n";
  out << "Options:\n";
  out << "  --seed N          Use a fixed random seed\n";
  out << "  --population N    Override population size\n";
  out << "  --trials N        Override trial/evaluation budget\n";
  out << "  --mutation R      Override mutation rate\n";
  out << "  --crossover R     Override crossover rate\n";
  out << "  --grid N          Override TSP grid size\n";
  out << "  --report-json P   Write a structured JSON run report to path P\n";
  out << "  --report-csv P    Write per-generation CSV progress to path P\n";
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
    else if (arg == "--report-json")
    {
      options.reportJsonPath = value;
    }
    else if (arg == "--report-csv")
    {
      options.reportCsvPath = value;
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

int runPopulation(Population& population, const CliOptions& cli)
{
  const bool verbose = std::getenv("GAK_VERBOSE") != nullptr;
  const bool captureGenerationSummaries = verbose ||
                                          !cli.reportJsonPath.empty() ||
                                          !cli.reportCsvPath.empty();
  printGaParameters(population.settings());
  Population::RunResult result = population.execute(captureGenerationSummaries);
  PopulationReporter::write(std::cout,
                            population,
                            result,
                            PopulationRunReportOptions(cli.showSettings, captureGenerationSummaries));

  if (!cli.reportJsonPath.empty())
  {
    std::ofstream jsonOut(cli.reportJsonPath.c_str());
    if (!jsonOut)
    {
      std::cerr << "Failed to open JSON report path: " << cli.reportJsonPath << '\n';
      return EXIT_FAILURE;
    }
    PopulationReporter::writeJson(jsonOut, population, result);
  }

  if (!cli.reportCsvPath.empty())
  {
    std::ofstream csvOut(cli.reportCsvPath.c_str());
    if (!csvOut)
    {
      std::cerr << "Failed to open CSV report path: " << cli.reportCsvPath << '\n';
      return EXIT_FAILURE;
    }
    PopulationReporter::writeGenerationCsv(csvOut, population, result);
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
        printDomeInput();
        Dome dome;
        Population population(applyOverrides(make_dome_options(), cli), dome);
        return runPopulation(population, cli);
      }
    case '6':
      {
        printF6Input();
        F6 f6;
        Population population(applyOverrides(make_f6_options(), cli), f6);
        return runPopulation(population, cli);
      }
    case 'A':
    case 'a':
      {
        printAlphaInput();
        Population::Settings settings = applyOverrides(make_alpha_options(), cli);
        Alpha alpha(settings);
        Population population(settings, alpha);
        return runPopulation(population, cli);
      }
    case 'S':
    case 's':
      {
        printSpellInput();
        Spell spell;
        Population population(applyOverrides(make_spell_options(), cli), spell);
        return runPopulation(population, cli);
      }
    case 'T':
    case 't':
      {
        const int gridSize = cli.gridProvided ? cli.grid : 500;
        Population::Settings settings = applyOverrides(make_traveling_salesman_options(), cli);
        TravelingSalesman travelingSalesman(settings, gridSize);
        Population population(settings, travelingSalesman);
        travelingSalesman.writeCityList(std::cout);
        return runPopulation(population, cli);
      }
    case 'Q':
    case 'q':
      {
        printNQueensInput();
        NQueens queens;
        Population population(applyOverrides(make_nqueens_options(), cli), queens);
        return runPopulation(population, cli);
      }
    case 'K':
    case 'k':
      {
        printKnapsackInput();
        Knapsack knapsack;
        Population population(applyOverrides(make_knapsack_options(), cli), knapsack);
        return runPopulation(population, cli);
      }
    case 'L':
    case 'l':
      {
        printLatinSquareInput();
        LatinSquare latinSquare;
        Population population(applyOverrides(make_latin_square_options(), cli), latinSquare);
        return runPopulation(population, cli);
      }
    case 'G':
    case 'g':
      {
        printGraphColoringInput();
        GraphColoring graphColoring;
        Population population(applyOverrides(make_graph_coloring_options(), cli), graphColoring);
        return runPopulation(population, cli);
      }
    case 'M':
    case 'm':
      {
        printTimetableInput();
        Timetable timetable;
        Population population(applyOverrides(make_timetable_options(), cli), timetable);
        return runPopulation(population, cli);
      }
    case 'U':
    case 'u':
      {
        printSudokuPuzzle();
        Sudoku sudoku;
        Population population(applyOverrides(make_sudoku_options(), cli), sudoku);
        return runPopulation(population, cli);
      }
    case 'C':
    case 'c':
      {
        printSudokuPuzzle();
        Population::Settings settings = applyOverrides(make_constrained_sudoku_options(), cli);
        SudokuConstrained sudoku(settings);
        Population population(settings, sudoku);
        return runPopulation(population, cli);
      }
    default:
      usage(std::cerr);
      return EXIT_FAILURE;
  }
}
