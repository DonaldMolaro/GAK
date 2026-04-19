#include <iostream>
#include <cstdlib>
#include <genetic.hh>
#include "dome.hh"
#include "alpha.hh"
#include "spell.hh"
#include "f6.hh"
#include "ts.hh"
#include "nqueens.hh"
#include "knapsack.hh"
#include "latinsquare.hh"
#include "sudoku.hh"
#include "sudoku_constrained.hh"

namespace
{
Population::Options make_dome_options()
{
  Population::Options options;
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

Population::Options make_f6_options()
{
  Population::Options options = make_dome_options();
  options.numberOfTrials = 5000;
  options.geneticDiversity = 44;
  return options;
}

Population::Options make_alpha_options()
{
  Population::Options options;
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

Population::Options make_spell_options()
{
  Population::Options options;
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

Population::Options make_traveling_salesman_options()
{
  Population::Options options;
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

Population::Options make_nqueens_options()
{
  Population::Options options;
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

Population::Options make_knapsack_options()
{
  Population::Options options;
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

Population::Options make_latin_square_options()
{
  Population::Options options;
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

Population::Options make_sudoku_options()
{
  Population::Options options;
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

Population::Options make_constrained_sudoku_options()
{
  Population::Options options = make_sudoku_options();
  options.numberOfIndividuals = 250;
  options.numberOfTrials = 12000;
  options.bitMutationRate = 0.05;
  options.crossOverRate = 0.80;
  return options;
}
}

void usage(std::ostream& out)
{
  out << "Usage\n";
  out << "\tGA D  - Dome Function (binary fixed length)\n";
  out << "\tGA F6 - F6 Function   (binary fixed length)\n";
  out << "\tGA A  - Alphabet      (high order, fixed length)\n";
  out << "\tGA S  - Spell         (high order, variable length)\n";
  out << "\tGA T  - Traveling Salesman (high order, fixed length)\n";
  out << "\tGA Q  - 8 Queens      (constraint satisfaction)\n";
  out << "\tGA K  - Knapsack      (combinatorial optimization)\n";
  out << "\tGA L  - Latin Square  (symbolic constraint satisfaction)\n";
  out << "\tGA U  - Sudoku        (givens + row/column/box constraints)\n";
  out << "\tGA C  - Sudoku+       (constraint-aware operators)\n";
}

int main(int argc,char *argv[])
{
   //
   // An example of algabraic optimization using
   // a generic algorithim.
   //
  if (argc != 2)
  {
    usage(std::cerr);
    return EXIT_FAILURE;
  }
  switch (*argv[1])
    {
    case 'd':
    case 'D':
      {
	Dome dome(make_dome_options());
	dome.run();
      }
      break;
    case '6':
      {
	F6 F6(make_f6_options());
	F6.run();
      }
      break;

    case 'A':
    case 'a':
      {
	Alpha alpha(make_alpha_options());
	//alpha.RandomAlgorithim();
	alpha.run();
      }
      break;
    case 'S':
    case 's':
      {
      //
      // An example of symbolic optimization
      // using variable length chromosomes.
      //
	Spell spell(make_spell_options());
	spell.run();
      }
      break;
    case 'T':
    case 't':
      /*
       * Traveling salesman problem solved with a high order GA.
       * Donald Molaro - Copyright 1997.
       *
       * Construts the population object, and set's it running.
       *
       */
      {
	TravelingSalesman travelingSalesman(make_traveling_salesman_options(), 500);
	//
	// Let the population go.
	//
	travelingSalesman.run();
      }
      break;
    case 'Q':
    case 'q':
      {
        NQueens queens(make_nqueens_options());
        queens.run();
      }
      break;
    case 'K':
    case 'k':
      {
        Knapsack knapsack(make_knapsack_options());
        knapsack.run();
      }
      break;
    case 'L':
    case 'l':
      {
        LatinSquare latin_square(make_latin_square_options());
        latin_square.run();
      }
      break;
    case 'U':
    case 'u':
      {
        Sudoku sudoku(make_sudoku_options());
        sudoku.run();
      }
      break;
    case 'C':
    case 'c':
      {
        SudokuConstrained sudoku(make_constrained_sudoku_options());
        sudoku.run();
      }
      break;
    default:
      usage(std::cerr);
      return EXIT_FAILURE;
    }
  return 0;
}
