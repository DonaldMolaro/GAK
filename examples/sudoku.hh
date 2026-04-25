#ifndef __GA_Sudoku_hh__
#define __GA_Sudoku_hh__

#include "population.hh"

class Sudoku : public Population
{
public:
   explicit Sudoku(const Population::Options& options);
   explicit Sudoku(const Population::Configuration& configuration);
   [[deprecated("Use Sudoku(const Population::Options&) or Sudoku(const Population::Configuration&)")]]
   Sudoku(
      Population::OperationTechnique Operation,
      int numberofIndividuals,
      int numberofTrials,
      int GenecticDeversity,
      double BitMutationRate,
      double CrossOverRate,
      Population::ReproductionTechniques PReproductionTechniques,
      Population::ParentSelectionTechnique ParentSelction,
      Population::DeletionTechnique Deletetion,
      Population::FitnessTechnique Fitness,
      Population::VariableLength Variable,
      int baseStates
      );

   double FitnessFunction(const BaseString& b) override;
   void FitnessPrint(const BaseString& b) override;

private:
   static const int kBoardSize = 9;
   static const int kCellCount = 81;
   static const int kSubgridSize = 3;
   static const int kPuzzle[kCellCount];

   void validateConfiguration(const Population::Configuration& configuration) const;
   int uniquenessScoreForRow(const BaseString& b, int row) const;
   int uniquenessScoreForColumn(const BaseString& b, int column) const;
   int uniquenessScoreForBox(const BaseString& b, int boxRow, int boxColumn) const;
   int givenConsistencyScore(const BaseString& b) const;
};

#endif
