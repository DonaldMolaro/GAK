#pragma once

// Symbolic regression: discover a formula y = f(inputs) that fits measured
// data, using postfix-encoded formulas (see expression.hh).
//
// Each candidate formula f is linearly scaled, y ~= a + b*f, with a and b
// solved by least squares, so the GA only has to find the SHAPE of the
// relationship. Fitness (minimized) is the normalized training error
// MSE / variance(y) -- 0 is a perfect fit, 1 is no better than predicting the
// mean -- plus a small penalty per formula node to discourage formulas that
// fit the noise. A held-out test split reports how well the formula
// generalizes.

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "chromosome.hh"
#include "dataset.hh"
#include "expression.hh"
#include "population.hh"

class SymbolicRegression : public PopulationProblem
{
public:
   struct Config
   {
      double holdoutFraction = 0.25; // share of rows kept out of fitting
      double parsimony = 0.002;      // fitness penalty per formula node
      int minimumInitialLength = 4;  // genes in a random initial program
      int maximumInitialLength = 24;
      int maximumLength = 64;        // offspring are truncated to this
      unsigned int splitSeed = 0x5eed; // fixed so every GA seed sees the same split
      // Pattern-search evaluations spent tuning each formula's constants
      // before it is scored (0 disables tuning).
      int constantTuningBudget = 40;
   };

   // Everything worth reporting about one candidate.
   struct Fit
   {
      bool valid = false;
      std::string formula;          // unscaled f, in infix
      double intercept = 0.0;       // a
      double slope = 0.0;           // b
      int size = 0;                 // formula nodes
      double trainR2 = 0.0;
      double testR2 = 0.0;
      double trainRmse = 0.0;
      double testRmse = 0.0;
      double fitness = 0.0;
   };

   SymbolicRegression(Dataset dataset, Config config);

   int alphabetSize() const { return instructions_.size(); }
   const Dataset& dataset() const { return dataset_; }
   const Config& config() const { return config_; }
   int trainRowCount() const { return static_cast<int>(trainRows_.size()); }
   int testRowCount() const { return static_cast<int>(testRows_.size()); }

   Fit describe(const BaseString& genes) const;
   // The scaled formula as text: "a + b*(f)", simplified when b is 1 or a is 0.
   std::string scaledFormula(const Fit& fit) const;

   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   void writeVisualizationJson(const BaseString& genes, std::ostream& out) const override;
   std::unique_ptr<Chromosome> initializeCandidate(Population& population) override;
   std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
      mateCandidates(Population& population, Chromosome& mother, Chromosome& father) override;

   // Fitness assigned to formulas that are undefined on the training data.
   static constexpr double kInvalidFitness = 1e6;

private:
   struct Split
   {
      std::vector<std::vector<double> > inputs;
      std::vector<double> targets;
      double mean = 0.0;
      double variance = 0.0; // population variance of the targets
   };

   static Split makeSplit(const Dataset& dataset, const std::vector<int>& rows);
   // Scaled training MSE of `expression`, or +infinity if it is undefined.
   double trainingError(const Expression& expression) const;
   void tuneConstants(Expression& expression) const;
   std::unique_ptr<Chromosome> truncated(std::unique_ptr<Chromosome> chromosome,
                                         const Population& population) const;

   Dataset dataset_;
   Config config_;
   InstructionSet instructions_;
   std::vector<int> trainRows_;
   std::vector<int> testRows_;
   Split train_;
   Split test_;
};
