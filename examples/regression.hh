#pragma once

// Symbolic regression: discover a formula y = f(inputs) that fits measured
// data, using postfix-encoded formulas (see expression.hh).
//
// Each candidate formula is split into its top-level additive terms and
// fitted as y ~= a + b1*term1 + b2*term2 + ... by least squares, so the GA
// only has to find the right TERMS; their weights come for free (with one
// term this is classic linear scaling, a + b*f). Fitness (minimized) is the normalized training error
// MSE / variance(y) -- 0 is a perfect fit, 1 is no better than predicting the
// mean -- plus a small penalty per formula node to discourage formulas that
// fit the noise.
//
// The GA searches on the training rows. Because training error always rewards
// extra terms that fit noise, the final formula is chosen differently: the
// best formula of each size seen during the search is archived, each is
// scored by k-fold cross-validation on the training rows (refitting its
// scaling and constants on k-1 folds and predicting the held-out fold), and
// the smallest formula within one standard error of the best cross-validated
// error wins. A held-out test split, used for nothing else, reports how well
// the chosen formula generalizes.

#include <map>
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
      double holdoutFraction = 0.25; // share of all rows kept for the test score
      // Folds for choosing the final formula by cross-validation on the
      // training rows (0 or 1 disables selection: the GA's best formula is
      // reported).
      int crossValidationFolds = 5;
      // One-standard-error rule: choose the smallest formula whose
      // cross-validated error is within this many standard errors of the
      // best. Error estimates from small data are noisy, so smaller
      // differences are not evidence that a larger formula is better.
      double selectionStandardErrors = 1.0;
      double parsimony = 0.002;      // fitness penalty per formula node
      // Each additive term beyond the first gets its own fitted coefficient,
      // which can fit noise like extra formula nodes can; it is charged as
      // this many nodes.
      double termPenaltyNodes = 6.0;
      // Copies of each input variable in the gene alphabet. Fewer copies for
      // many inputs (so operators are not crowded out) was measured and did
      // not help on 3-10 input data.
      int variableCopies = 3;
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
      // One fitted coefficient per additive term of f (terms[i] is its text):
      // y ~= intercept + sum weights[i] * term i.
      std::vector<double> weights;
      std::vector<std::string> terms;
      double slope = 0.0;           // weights[0] when f is a single term, else 0
      std::vector<std::string> inputsUsed; // input columns the formula reads
      int size = 0;                 // formula nodes
      double trainR2 = 0.0;
      double testR2 = 0.0;
      double trainRmse = 0.0;
      double testRmse = 0.0;
      // Set only by selectByCrossValidation.
      double crossValidatedR2 = 0.0;
      double crossValidatedRmse = 0.0;
      double fitness = 0.0;
   };

   SymbolicRegression(Dataset dataset, Config config);

   int alphabetSize() const { return instructions_.size(); }
   const Dataset& dataset() const { return dataset_; }
   const Config& config() const { return config_; }
   int trainRowCount() const { return static_cast<int>(trainRows_.size()); }
   int crossValidationFolds() const { return static_cast<int>(foldFit_.size()); }
   int testRowCount() const { return static_cast<int>(testRows_.size()); }

   // The archive: for each formula size, the genes of the formula with the
   // lowest training error evaluated so far (across every run that used this
   // problem object).
   int archivedSizeCount() const { return static_cast<int>(archive_.size()); }
   // Choose the final formula from the archive by cross-validated error.
   // Returns false when cross-validation is disabled or nothing has been
   // evaluated.
   bool selectByCrossValidation(std::vector<int>& genes, Fit& fit) const;
   BaseString genesFromValues(const std::vector<int>& values) const;

   Fit describe(const BaseString& genes) const;
   // The scaled formula as text: "a + b*(f)", simplified when b is 1 or a is 0.
   std::string scaledFormula(const Fit& fit) const;

   void validatePopulation(const Population& population) const override;
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
   void writeVisualizationJson(const BaseString& genes, std::ostream& out) const override;
   // Compact payload for sampled population members: the formula and its
   // scores plus a coarse curve (one input) or one prediction per row (several
   // inputs), without repeating the data points.
   void writeSampleJson(const BaseString& genes, std::ostream& out) const override;
   // For each input, how many members of the generation use it:
   // { "members": N, "input_usage": [count per input] }.
   std::string summarizePopulation(const std::vector<const BaseString*>& members) const override;
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
   // Scaled MSE of `expression` on `rows`, or +infinity if it is undefined.
   double scaledError(const Expression& expression, const Split& rows) const;
   // Tune the formula's constants to minimize scaledError on `rows`.
   void tuneConstants(Expression& expression, const Split& rows) const;
   std::unique_ptr<Chromosome> truncated(std::unique_ptr<Chromosome> chromosome,
                                         const Population& population) const;

   Dataset dataset_;
   Config config_;
   InstructionSet instructions_;
   struct Archived
   {
      double trainRmse = 0.0;
      std::vector<int> genes;
   };
   void heldOutScore(const Expression& expression, const Fit& fit,
                     const Split& split, double& r2, double& rmse) const;
   // Predictions of a fitted formula on `inputs`; false if it is undefined.
   bool predictRows(const Expression& expression, const Fit& fit,
                    const std::vector<std::vector<double> >& inputs,
                    std::vector<double>& predictions) const;
   // Visualization extras for several inputs: "effects" (partial dependence
   // per input) and "terms" (each additive term's share of the explained
   // variance and the inputs it involves).
   void writeEffectsJson(const Expression& expression, const Fit& fit, std::ostream& out) const;
   void writeTermSharesJson(const Expression& expression, const Fit& fit, std::ostream& out) const;
   // Out-of-fold mean squared error of a formula structure, and its
   // standard error over the training rows.
   bool crossValidationStats(const BaseString& genes, double& mse, double& standardError) const;

   std::vector<int> trainRows_;
   std::vector<int> testRows_;
   Split train_;
   std::vector<Split> foldFit_;  // per fold: the training rows outside it
   std::vector<Split> foldHeld_; // per fold: the training rows inside it
   Split test_;
   std::map<int, Archived> archive_;
};
