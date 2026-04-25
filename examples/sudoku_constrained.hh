#ifndef __GA_SudokuConstrained_hh__
#define __GA_SudokuConstrained_hh__

#include <random>
#include <iosfwd>
#include <utility>
#include <vector>

#include "population.hh"

class SudokuConstrained : public Population
{
public:
   explicit SudokuConstrained(const Population::Settings& settings);

   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) override;

protected:
   std::unique_ptr<Chromosome> createInitialChromosome() override;
   std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
      mateChromosomes(Chromosome& mother, Chromosome& father) override;
   void mutateChromosome(Chromosome& chromosome) override;

private:
   static const int kBoardSize = 9;
   static const int kCellCount = 81;
   static const int kSubgridSize = 3;
   static const int kPuzzle[kCellCount];

   using RowColumns = std::vector<int>;

   void validateSettings(const Population::Settings& settings) const;
   int uniquenessScoreForColumn(const BaseString& genes, int column) const;
   int uniquenessScoreForBox(const BaseString& genes, int boxRow, int boxColumn) const;
   int givenConsistencyScore(const BaseString& genes) const;
   std::unique_ptr<BaseString> cloneBoard(const BaseString *source) const;
   void fillRowFromParent(BaseString *destination, const BaseString *source, int row) const;
   void initializeRow(BaseString *board, int row);
   bool rowIsValidPermutation(const BaseString *board, int row) const;

   std::vector<RowColumns> mutableColumnsByRow_;
   std::vector<std::vector<int> > missingDigitsByRow_;
   std::mt19937 randomGenerator_;
};

#endif
