#pragma once


#include <iosfwd>
#include <random>
#include <utility>
#include <vector>

#include "population.hh"

class SudokuConstrained : public PopulationProblem
{
public:
   explicit SudokuConstrained(const Population::Settings& settings);

   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   void writeVisualizationJson(const BaseString& genes, std::ostream& out) const override;
   bool hasReachedSolution(const Population& population,
                           const BaseString& genes,
                           double fitness) const override;
   std::unique_ptr<Chromosome> initializeCandidate(Population& population) override;
   std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
      mateCandidates(Population& population, Chromosome& mother, Chromosome& father) override;
   void mutateCandidate(Population& population, Chromosome& chromosome) override;

   static const int kBoardSize = 9;
   static const int kCellCount = 81;
   static const int kSubgridSize = 3;
   static const int kPuzzle[kCellCount];

   using RowColumns = std::vector<int>;

private:
   int uniquenessScoreForColumn(const BaseString& genes, int column) const;
   int uniquenessScoreForBox(const BaseString& genes, int boxRow, int boxColumn) const;
   int givenConsistencyScore(const BaseString& genes) const;
   BaseString cloneBoard(const BaseString& source) const;
   void fillRowFromParent(BaseString& destination, const BaseString& source, int row) const;
   void initializeRow(BaseString& board, int row, std::mt19937& randomGenerator);
   bool rowIsValidPermutation(const BaseString& board, int row) const;

   std::vector<RowColumns> mutableColumnsByRow_;
   std::vector<std::vector<int> > missingDigitsByRow_;
   Population::Settings settings_;
};
