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

   double FitnessFunction(BaseString *b) override;
   void FitnessPrint(BaseString *b) override;

private:
   static const int kBoardSize = 9;
   static const int kCellCount = 81;
   static const int kSubgridSize = 3;
   static const int kPuzzle[kCellCount];

   void validateConfiguration(const Population::Configuration& configuration) const;
   int uniquenessScoreForRow(BaseString *b, int row) const;
   int uniquenessScoreForColumn(BaseString *b, int column) const;
   int uniquenessScoreForBox(BaseString *b, int boxRow, int boxColumn) const;
   int givenConsistencyScore(BaseString *b) const;
};

#endif
