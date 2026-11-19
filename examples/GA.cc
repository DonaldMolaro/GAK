#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>
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
#include "dataset.hh"
#include "regression.hh"

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
  std::string dataPath;
  std::string targetColumn;
  bool holdoutProvided = false;
  double holdout = 0.0;
  bool tuningProvided = false;
  int tuning = 0;
  bool restartsProvided = false;
  int restarts = 0;
  bool selectionProvided = false;
  Population::ParentSelectionMode selection = Population::ParentSelectionMode::RouletteWheel;
  bool tournamentProvided = false;
  int tournament = 0;
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
  printLabeledLine("nodes", "A B C D E F G H I J K L M N O P");
  printLabeledLine("colors", "red green blue");
  printLabeledLine("edges", "A-B B-C A-C B-D C-D C-E D-E D-F E-F D-G");
  printLabeledLine("", "F-G D-H F-H E-I F-I E-J I-J D-K E-K C-L");
  printLabeledLine("", "E-L I-M J-M E-N J-N E-O L-O C-P L-P");
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
  switch (mode)
  {
  case Population::ParentSelectionMode::Random: return "random";
  case Population::ParentSelectionMode::Tournament: return "tournament";
  default: return "roulette";
  }
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
  if (settings.parentSelection == Population::ParentSelectionMode::Tournament)
  {
    printLabeledLine("tournament size", settings.tournamentSize);
  }
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

const int kDefaultRegressionRestarts = 4;

Population::Settings make_regression_options(int alphabetSize)
{
  Population::Settings options;
  options.operation = Population::OperationMode::Minimize;
  options.numberOfIndividuals = 400;
  options.numberOfTrials = 15000; // per restart (4 restarts by default)
  options.chromosomeLength = SymbolicRegression::Config().maximumInitialLength;
  options.bitMutationRate = 0.04;
  options.crossOverRate = 0.8;
  options.reproduction = Population::ReproductionMode::DisallowDuplicates;
  options.parentSelection = Population::ParentSelectionMode::RouletteWheel;
  options.deletion = Population::DeletionMode::DeleteHalf;
  options.fitness = Population::FitnessMode::LinearNormalized;
  options.variableLength = Population::VariableLengthMode::Variable;
  options.baseStates = alphabetSize;
  return options;
}

void printRegressionInput(const SymbolicRegression& regression)
{
  const Dataset& data = regression.dataset();
  std::string inputs;
  for (int i = 0 ; i < data.inputCount() ; i++)
  {
    inputs += (i > 0 ? " " : "") + data.inputNames[i];
  }
  printSectionHeading("Problem input:");
  printLabeledLine("problem", "symbolic regression");
  printLabeledLine("data", data.source);
  printLabeledLine("rows", data.rowCount());
  printLabeledLine("inputs", inputs);
  printLabeledLine("target", data.targetName);
  printLabeledLine("train rows", regression.trainRowCount());
  printLabeledLine("test rows", regression.testRowCount());
  printLabeledLine("operators", "+ - * / sin cos exp log sqrt ^2 neg");
  printLabeledLine("constants", "0.1 0.5 1 2 3 5 10");
  std::cout << '\n';
  finishInputSection();
}

Population::Settings make_graph_coloring_options()
{
  Population::Settings options;
  options.operation = Population::OperationMode::Maximize;
  options.numberOfIndividuals = 120;
  options.numberOfTrials = 20000;
  options.chromosomeLength = GraphColoring::kNodeCount;
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
  out << "  C   Sudoku+\n";
  out << "  R   Symbolic regression on a CSV (requires --data)\n\n";
  out << "Options:\n";
  out << "  --seed N          Use a fixed random seed\n";
  out << "  --population N    Override population size\n";
  out << "  --trials N        Override trial/evaluation budget\n";
  out << "  --mutation R      Override mutation rate\n";
  out << "  --crossover R     Override crossover rate\n";
  out << "  --selection S     Parent selection: roulette, random or tournament\n";
  out << "  --tournament N    Tournament size (implies --selection tournament)\n";
  out << "  --grid N          Override TSP grid size\n";
  out << "  --data P          CSV of measured points for mode R (header row, numeric values)\n";
  out << "  --target NAME     Output column for mode R (default: the last column)\n";
  out << "  --holdout R       Share of rows held out for testing in mode R (default 0.25)\n";
  out << "  --restarts N      Independent runs in mode R; the best training fit is reported (default 4)\n";
  out << "  --tuning N        Constant-tuning evaluations per formula in mode R (default 40, 0 = off)\n";
  out << "  --report-json P   Write a structured JSON run report to path P\n";
  out << "  --report-csv P    Write per-generation CSV progress to path P\n";
  out << "  --show-settings   Print the resolved settings report before the final summary\n";
  out << "  --help            Show this help\n";
}

// Strict parsers: the whole string must be a number in range. Empty
// strings, trailing junk ("12abc"), overflow and negative unsigned values
// ("-1" would otherwise wrap to 4294967295) are all rejected.
bool parseInt(const std::string& text, int& value)
{
  if (text.empty() || std::isspace(static_cast<unsigned char>(text[0])))
  {
    return false;
  }
  errno = 0;
  char* end = nullptr;
  const long parsed = std::strtol(text.c_str(), &end, 10);
  if (errno != 0 || *end != '\0' ||
      parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max())
  {
    return false;
  }
  value = static_cast<int>(parsed);
  return true;
}

bool parsePositiveInt(const std::string& text, int& value)
{
  return parseInt(text, value) && value > 0;
}

bool parseUnsigned(const std::string& text, unsigned int& value)
{
  if (text.empty() || !std::isdigit(static_cast<unsigned char>(text[0])))
  {
    return false;
  }
  errno = 0;
  char* end = nullptr;
  const unsigned long long parsed = std::strtoull(text.c_str(), &end, 10);
  if (errno != 0 || *end != '\0' || parsed > std::numeric_limits<unsigned int>::max())
  {
    return false;
  }
  value = static_cast<unsigned int>(parsed);
  return true;
}

bool parseProbability(const std::string& text, double& value)
{
  if (text.empty() || std::isspace(static_cast<unsigned char>(text[0])))
  {
    return false;
  }
  errno = 0;
  char* end = nullptr;
  const double parsed = std::strtod(text.c_str(), &end);
  if (errno != 0 || *end != '\0' || !(parsed >= 0.0 && parsed <= 1.0))
  {
    return false;
  }
  value = parsed;
  return true;
}

bool reportInvalidValue(const std::string& option, const std::string& value, const char* expected)
{
  std::cerr << "Invalid value for " << option << ": '" << value << "' (expected " << expected << ")\n";
  return false;
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
      if (!options.seedProvided) return reportInvalidValue(arg, value, "an integer from 0 to 4294967295");
    }
    else if (arg == "--population")
    {
      options.populationProvided = parsePositiveInt(value, options.population);
      if (!options.populationProvided) return reportInvalidValue(arg, value, "a positive integer");
    }
    else if (arg == "--trials")
    {
      options.trialsProvided = parsePositiveInt(value, options.trials);
      if (!options.trialsProvided) return reportInvalidValue(arg, value, "a positive integer");
    }
    else if (arg == "--mutation")
    {
      options.mutationProvided = parseProbability(value, options.mutation);
      if (!options.mutationProvided) return reportInvalidValue(arg, value, "a number from 0 to 1");
    }
    else if (arg == "--crossover")
    {
      options.crossoverProvided = parseProbability(value, options.crossover);
      if (!options.crossoverProvided) return reportInvalidValue(arg, value, "a number from 0 to 1");
    }
    else if (arg == "--grid")
    {
      options.gridProvided = parsePositiveInt(value, options.grid);
      if (!options.gridProvided) return reportInvalidValue(arg, value, "a positive integer");
    }
    else if (arg == "--tuning")
    {
      options.tuningProvided = parseInt(value, options.tuning) && options.tuning >= 0;
      if (!options.tuningProvided) return reportInvalidValue(arg, value, "a non-negative integer");
    }
    else if (arg == "--restarts")
    {
      options.restartsProvided = parsePositiveInt(value, options.restarts);
      if (!options.restartsProvided) return reportInvalidValue(arg, value, "a positive integer");
    }
    else if (arg == "--selection")
    {
      options.selectionProvided = true;
      if (value == "roulette") options.selection = Population::ParentSelectionMode::RouletteWheel;
      else if (value == "random") options.selection = Population::ParentSelectionMode::Random;
      else if (value == "tournament") options.selection = Population::ParentSelectionMode::Tournament;
      else return reportInvalidValue(arg, value, "roulette, random or tournament");
    }
    else if (arg == "--tournament")
    {
      options.tournamentProvided = parsePositiveInt(value, options.tournament);
      if (!options.tournamentProvided) return reportInvalidValue(arg, value, "a positive integer");
    }
    else if (arg == "--data")
    {
      options.dataPath = value;
    }
    else if (arg == "--target")
    {
      options.targetColumn = value;
    }
    else if (arg == "--holdout")
    {
      options.holdoutProvided = parseProbability(value, options.holdout) && options.holdout < 1.0;
      if (!options.holdoutProvided) return reportInvalidValue(arg, value, "a number from 0 up to (not including) 1");
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
  if (cli.selectionProvided)
  {
    options.parentSelection = cli.selection;
  }
  if (cli.tournamentProvided)
  {
    options.parentSelection = Population::ParentSelectionMode::Tournament;
    options.tournamentSize = cli.tournament;
  }
  return options;
}

bool shouldCaptureGenerations(const CliOptions& cli)
{
  return std::getenv("GAK_VERBOSE") != nullptr ||
         !cli.reportJsonPath.empty() ||
         !cli.reportCsvPath.empty();
}

int reportRun(Population& population, const Population::RunResult& result, const CliOptions& cli)
{
  const bool captureGenerationSummaries = shouldCaptureGenerations(cli);
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

int runPopulation(Population& population, const CliOptions& cli)
{
  printGaParameters(population.settings());
  Population::RunResult result = population.execute(shouldCaptureGenerations(cli));
  return reportRun(population, result, cli);
}

// Seed for restart `index`: restart 0 keeps the base seed, so one restart
// reproduces a plain run exactly; later restarts get well-separated seeds.
unsigned int restartSeed(unsigned int baseSeed, int index)
{
  return baseSeed ^ (static_cast<unsigned int>(index) * 0x9E3779B9u);
}

// Run independent populations and report the one whose best formula has the
// lowest TRAINING fitness. Held-out rows never influence the choice, so the
// reported test score is still an honest estimate.
int runRegression(SymbolicRegression& regression, const Population::Settings& baseSettings,
                  const CliOptions& cli, int restarts)
{
  printGaParameters(baseSettings);
  printLabeledLine("restarts", restarts);
  std::cout << '\n';

  std::unique_ptr<Population> best;
  Population::RunResult bestResult;
  double bestFitness = 0.0;
  const unsigned int baseSeed = baseSettings.useFixedRandomSeed ? baseSettings.randomSeed
                                                                : std::random_device{}();
  for (int index = 0 ; index < restarts ; index++)
  {
    Population::Settings settings = baseSettings;
    settings.useFixedRandomSeed = true;
    settings.randomSeed = restartSeed(baseSeed, index);
    std::unique_ptr<Population> population(new Population(settings, regression));
    Population::RunResult result = population->execute(shouldCaptureGenerations(cli));
    const double fitness = result.finalSummary.mostFit.front();
    if (restarts > 1)
    {
      std::cout << "Restart " << (index + 1) << " (seed " << settings.randomSeed << "): fitness "
                << fitness << '\n';
    }
    if (!best || fitness < bestFitness)
    {
      best = std::move(population);
      bestResult = result;
      bestFitness = fitness;
    }
  }
  if (restarts > 1)
  {
    std::cout << "Best restart seed: " << best->randomSeed() << "\n\n";
  }
  return reportRun(*best, bestResult, cli);
}
}

int runGa(int argc, char *argv[])
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
    case 'R':
    case 'r':
      {
        if (cli.dataPath.empty())
        {
          std::cerr << "Mode R needs --data PATH (a CSV with a header row)\n";
          return EXIT_FAILURE;
        }
        SymbolicRegression::Config config;
        if (cli.holdoutProvided)
        {
          config.holdoutFraction = cli.holdout;
        }
        if (cli.tuningProvided)
        {
          config.constantTuningBudget = cli.tuning;
        }
        SymbolicRegression regression(loadCsvDataset(cli.dataPath, cli.targetColumn), config);
        printRegressionInput(regression);
        return runRegression(regression,
                             applyOverrides(make_regression_options(regression.alphabetSize()), cli),
                             cli, cli.restartsProvided ? cli.restarts : kDefaultRegressionRestarts);
      }
    default:
      usage(std::cerr);
      return EXIT_FAILURE;
  }
}

int main(int argc, char *argv[])
{
  // Configuration problems (for example a population too small for the
  // deletion mode) surface as GAFatalException; report them cleanly instead
  // of terminating with an uncaught exception.
  try
  {
    return runGa(argc, argv);
  }
  catch (const std::exception& error)
  {
    std::cerr << "GA: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
