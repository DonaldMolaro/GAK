#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <ostream>
#include <random>
#include <sstream>

#include "base.hh"
#include "except.hh"
#include "random.hh"
#include "regression.hh"

namespace
{
constexpr double kVarianceEpsilon = 1e-12;
constexpr int kCurveSamples = 200;
constexpr int kSampleCurvePoints = 60;
constexpr int kEffectPoints = 25;

constexpr int kMaxTerms = 8;

// The linear model fitted on top of a formula: y ~= intercept + sum of
// weights[i] * term i, where the terms are the formula's top-level additive
// terms (see Expression::additiveTerms). With one term this is the classic
// linear scaling a + b*f.
struct TermModel
{
   double intercept = 0.0;
   std::vector<double> weights;
};

// The terms that get a fitted coefficient: the formula's additive terms that
// depend on an input (a variable-free term only shifts the intercept).
std::vector<int> modelTerms(const Expression& expression)
{
   std::vector<int> terms;
   for (int node : expression.additiveTerms(kMaxTerms))
   {
      if (expression.usesVariable(node))
      {
         terms.push_back(node);
      }
   }
   return terms;
}

// Least squares for y ~= a + sum w_i t_i, solved on centred data with a tiny
// ridge so that duplicated or (near-)constant terms cannot make it singular.
// A term with no variation gets weight 0, so a formula carrying no shape
// information falls back to predicting the mean.
TermModel fitTerms(const std::vector<std::vector<double> >& terms, const std::vector<double>& targets)
{
   const std::size_t count = targets.size();
   const std::size_t k = terms.size();
   TermModel model;
   model.weights.assign(k, 0.0);
   double meanY = 0.0;
   for (double target : targets) meanY += target;
   meanY /= static_cast<double>(count);

   std::vector<double> means(k, 0.0);
   for (std::size_t i = 0 ; i < k ; i++)
   {
      for (double value : terms[i]) means[i] += value;
      means[i] /= static_cast<double>(count);
   }
   // Normal equations A w = b on centred columns; columns with no variation
   // are left out (weight 0).
   std::vector<std::size_t> active;
   for (std::size_t i = 0 ; i < k ; i++)
   {
      double variance = 0.0;
      for (double value : terms[i]) variance += (value - means[i]) * (value - means[i]);
      if (variance > kVarianceEpsilon * static_cast<double>(count))
      {
         active.push_back(i);
      }
   }
   const std::size_t m = active.size();
   if (m > 0)
   {
      std::vector<double> a(m * m, 0.0);
      std::vector<double> b(m, 0.0);
      for (std::size_t r = 0 ; r < m ; r++)
      {
         const std::vector<double>& tr = terms[active[r]];
         for (std::size_t row = 0 ; row < count ; row++)
         {
            b[r] += (tr[row] - means[active[r]]) * (targets[row] - meanY);
         }
         for (std::size_t c = r ; c < m ; c++)
         {
            const std::vector<double>& tc = terms[active[c]];
            double sum = 0.0;
            for (std::size_t row = 0 ; row < count ; row++)
            {
               sum += (tr[row] - means[active[r]]) * (tc[row] - means[active[c]]);
            }
            a[r * m + c] = sum;
            a[c * m + r] = sum;
         }
      }
      double trace = 0.0;
      for (std::size_t r = 0 ; r < m ; r++) trace += a[r * m + r];
      const double ridge = 1e-10 * trace / static_cast<double>(m);
      for (std::size_t r = 0 ; r < m ; r++) a[r * m + r] += ridge;
      // Gaussian elimination with partial pivoting.
      std::vector<double> w(m, 0.0);
      bool solved = true;
      for (std::size_t col = 0 ; col < m && solved ; col++)
      {
         std::size_t pivot = col;
         for (std::size_t r = col + 1 ; r < m ; r++)
         {
            if (std::fabs(a[r * m + col]) > std::fabs(a[pivot * m + col])) pivot = r;
         }
         if (!(std::fabs(a[pivot * m + col]) > 0.0))
         {
            solved = false;
            break;
         }
         if (pivot != col)
         {
            for (std::size_t c = 0 ; c < m ; c++) std::swap(a[col * m + c], a[pivot * m + c]);
            std::swap(b[col], b[pivot]);
         }
         for (std::size_t r = col + 1 ; r < m ; r++)
         {
            const double factor = a[r * m + col] / a[col * m + col];
            for (std::size_t c = col ; c < m ; c++) a[r * m + c] -= factor * a[col * m + c];
            b[r] -= factor * b[col];
         }
      }
      if (solved)
      {
         for (std::size_t r = m ; r-- > 0 ; )
         {
            double sum = b[r];
            for (std::size_t c = r + 1 ; c < m ; c++) sum -= a[r * m + c] * w[c];
            w[r] = sum / a[r * m + r];
         }
         for (std::size_t r = 0 ; r < m ; r++)
         {
            if (std::isfinite(w[r])) model.weights[active[r]] = w[r];
         }
      }
   }
   model.intercept = meanY;
   for (std::size_t i = 0 ; i < k ; i++) model.intercept -= model.weights[i] * means[i];
   return model;
}

double predictRow(const TermModel& model, const std::vector<std::vector<double> >& terms, std::size_t row)
{
   double value = model.intercept;
   for (std::size_t i = 0 ; i < terms.size() ; i++) value += model.weights[i] * terms[i][row];
   return value;
}

double meanSquaredError(const TermModel& model,
                        const std::vector<std::vector<double> >& terms,
                        const std::vector<double>& targets)
{
   double total = 0.0;
   for (std::size_t row = 0 ; row < targets.size() ; row++)
   {
      const double error = targets[row] - predictRow(model, terms, row);
      total += error * error;
   }
   return total / static_cast<double>(targets.size());
}

// The values of the model terms on `inputs`; terms.empty() still checks the
// whole formula is defined, by evaluating its root.
bool evaluateTerms(const Expression& expression, const std::vector<int>& terms,
                   const std::vector<std::vector<double> >& inputs,
                   std::vector<std::vector<double> >& values)
{
   if (terms.empty())
   {
      std::vector<double> root;
      values.clear();
      return expression.evaluate(inputs, root);
   }
   return expression.evaluateNodes(inputs, terms, values);
}

double rSquared(double mse, double variance)
{
   return variance > kVarianceEpsilon ? 1.0 - mse / variance : 0.0;
}

void writeJsonString(std::ostream& out, const std::string& text)
{
   out << '"';
   for (char c : text)
   {
      switch (c)
      {
      case '"': out << "\\\""; break;
      case '\\': out << "\\\\"; break;
      case '\n': out << "\\n"; break;
      default:
         if (static_cast<unsigned char>(c) < 0x20)
         {
            out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << static_cast<int>(c) << std::dec << std::setfill(' ');
         }
         else
         {
            out << c;
         }
      }
   }
   out << '"';
}
}

constexpr double SymbolicRegression::kInvalidFitness;

SymbolicRegression::SymbolicRegression(Dataset dataset, Config config)
   : dataset_(std::move(dataset)),
     config_(config),
     instructions_(dataset_.inputCount(), config.variableCopies)
{
   if (!(config_.holdoutFraction >= 0.0 && config_.holdoutFraction < 1.0))
   {
      throw GAFatalException(__FILE__, __LINE__, "Holdout fraction must be at least 0 and below 1");
   }
   if (config_.crossValidationFolds < 0)
   {
      throw GAFatalException(__FILE__, __LINE__, "Cross-validation folds must not be negative");
   }
   if (!(config_.selectionStandardErrors >= 0.0))
   {
      throw GAFatalException(__FILE__, __LINE__, "The selection standard-error margin must not be negative");
   }
   if (config_.minimumInitialLength < 1 ||
       config_.maximumInitialLength < config_.minimumInitialLength ||
       config_.maximumLength < config_.maximumInitialLength)
   {
      throw GAFatalException(__FILE__, __LINE__, "Inconsistent symbolic regression program lengths");
   }

   std::vector<int> order(dataset_.rowCount());
   for (int i = 0 ; i < dataset_.rowCount() ; i++)
   {
      order[i] = i;
   }
   std::mt19937 splitGenerator(config_.splitSeed);
   gak::shuffle(order.begin(), order.end(), splitGenerator);

   const int testCount = static_cast<int>(std::floor(config_.holdoutFraction * dataset_.rowCount()));
   testRows_.assign(order.begin(), order.begin() + testCount);
   trainRows_.assign(order.begin() + testCount, order.end());
   std::sort(testRows_.begin(), testRows_.end());
   std::sort(trainRows_.begin(), trainRows_.end());
   if (trainRows_.size() < 3)
   {
      throw GAFatalException(__FILE__, __LINE__, "Symbolic regression needs at least 3 training rows");
   }
   train_ = makeSplit(dataset_, trainRows_);
   test_ = makeSplit(dataset_, testRows_);

   // Cross-validation folds over the training rows, assigned from a fixed
   // shuffle so every GA seed is judged on the same folds.
   const int folds = config_.crossValidationFolds;
   if (folds >= 2)
   {
      if (static_cast<int>(trainRows_.size()) < 2 * folds)
      {
         throw GAFatalException(__FILE__, __LINE__,
                                "Too few training rows for the requested cross-validation folds");
      }
      std::vector<int> positions(trainRows_.size());
      for (std::size_t i = 0 ; i < positions.size() ; i++)
      {
         positions[i] = static_cast<int>(i);
      }
      std::mt19937 foldGenerator(config_.splitSeed + 1u);
      gak::shuffle(positions.begin(), positions.end(), foldGenerator);
      for (int fold = 0 ; fold < folds ; fold++)
      {
         std::vector<int> fitRows;
         std::vector<int> heldRows;
         for (std::size_t i = 0 ; i < positions.size() ; i++)
         {
            (static_cast<int>(i) % folds == fold ? heldRows : fitRows).push_back(trainRows_[positions[i]]);
         }
         foldFit_.push_back(makeSplit(dataset_, fitRows));
         foldHeld_.push_back(makeSplit(dataset_, heldRows));
      }
   }
   if (train_.variance <= kVarianceEpsilon)
   {
      throw GAFatalException(__FILE__, __LINE__, "The target column is constant on the training rows");
   }
}

SymbolicRegression::Split SymbolicRegression::makeSplit(const Dataset& dataset, const std::vector<int>& rows)
{
   Split split;
   for (int row : rows)
   {
      split.inputs.push_back(dataset.inputs[row]);
      split.targets.push_back(dataset.targets[row]);
      split.mean += dataset.targets[row];
   }
   if (rows.empty())
   {
      return split;
   }
   split.mean /= static_cast<double>(rows.size());
   for (double target : split.targets)
   {
      split.variance += (target - split.mean) * (target - split.mean);
   }
   split.variance /= static_cast<double>(rows.size());
   return split;
}

double SymbolicRegression::scaledError(const Expression& expression, const Split& rows) const
{
   thread_local std::vector<std::vector<double> > values;
   const std::vector<int> terms = modelTerms(expression);
   if (!evaluateTerms(expression, terms, rows.inputs, values))
   {
      return std::numeric_limits<double>::infinity();
   }
   const double mse = meanSquaredError(fitTerms(values, rows.targets), values, rows.targets);
   return std::isfinite(mse) ? mse : std::numeric_limits<double>::infinity();
}

// Deterministic pattern search over the formula's constants: try moving each
// constant up or down by a relative step, keep any move that lowers the
// training error, and halve the step when no move helps. The GA chooses the
// structure; this finds values such as the 0.3 in exp(-0.3 t) that the
// constant table cannot express directly. No randomness is used, so the
// result depends only on the formula and the data.
void SymbolicRegression::tuneConstants(Expression& expression, const Split& rows) const
{
   const std::vector<int> constants = expression.constantNodes();
   if (constants.empty() || config_.constantTuningBudget <= 0)
   {
      return;
   }
   double best = scaledError(expression, rows);
   if (!std::isfinite(best))
   {
      return;
   }
   double step = 0.5;
   int evaluations = 0;
   while (evaluations < config_.constantTuningBudget && step > 1e-4)
   {
      bool improved = false;
      for (int node : constants)
      {
         const double original = expression.constantAt(node);
         const double delta = step * std::max(std::fabs(original), 0.1);
         for (double candidate : {original + delta, original - delta})
         {
            expression.setConstant(node, candidate);
            const double error = scaledError(expression, rows);
            ++evaluations;
            if (error < best)
            {
               best = error;
               improved = true;
               break;
            }
            expression.setConstant(node, original);
         }
         if (evaluations >= config_.constantTuningBudget)
         {
            break;
         }
      }
      if (!improved)
      {
         step *= 0.5;
      }
   }
}

SymbolicRegression::Fit SymbolicRegression::describe(const BaseString& genes) const
{
   Fit fit;
   fit.fitness = kInvalidFitness;
   Expression expression = Expression::compile(genes, instructions_);
   tuneConstants(expression, train_);
   fit.size = expression.size();
   fit.formula = expression.toString(dataset_.inputNames);

   const std::vector<int> terms = modelTerms(expression);
   std::vector<std::vector<double> > values;
   if (!evaluateTerms(expression, terms, train_.inputs, values))
   {
      return fit;
   }
   const TermModel model = fitTerms(values, train_.targets);
   const double trainMse = meanSquaredError(model, values, train_.targets);
   if (!std::isfinite(trainMse))
   {
      return fit;
   }

   fit.valid = true;
   fit.intercept = model.intercept;
   fit.weights = model.weights;
   fit.slope = model.weights.size() == 1 ? model.weights[0] : 0.0;
   for (int node : terms)
   {
      fit.terms.push_back(expression.nodeText(dataset_.inputNames, node));
   }
   for (int variable : expression.variablesUsed())
   {
      fit.inputsUsed.push_back(dataset_.inputNames[variable]);
   }
   fit.trainRmse = std::sqrt(trainMse);
   fit.trainR2 = rSquared(trainMse, train_.variance);
   const double extraTerms = terms.size() > 1 ? static_cast<double>(terms.size() - 1) : 0.0;
   fit.fitness = trainMse / train_.variance +
                 config_.parsimony * (fit.size + config_.termPenaltyNodes * extraTerms);

   heldOutScore(expression, fit, test_, fit.testR2, fit.testRmse);
   return fit;
}

bool SymbolicRegression::predictRows(const Expression& expression, const Fit& fit,
                                     const std::vector<std::vector<double> >& inputs,
                                     std::vector<double>& predictions) const
{
   const std::vector<int> terms = modelTerms(expression);
   std::vector<std::vector<double> > values;
   if (!fit.valid || terms.size() != fit.weights.size() || !evaluateTerms(expression, terms, inputs, values))
   {
      return false;
   }
   TermModel model;
   model.intercept = fit.intercept;
   model.weights = fit.weights;
   predictions.resize(inputs.size());
   for (std::size_t row = 0 ; row < inputs.size() ; row++)
   {
      predictions[row] = predictRow(model, values, row);
   }
   return true;
}

// Score a held-out split with the model fitted on the training rows.
void SymbolicRegression::heldOutScore(const Expression& expression, const Fit& fit,
                                      const Split& split, double& r2, double& rmse) const
{
   r2 = 0.0;
   rmse = 0.0;
   if (split.targets.empty())
   {
      return;
   }
   std::vector<double> predictions;
   if (!predictRows(expression, fit, split.inputs, predictions))
   {
      // Defined on the training rows but not on held-out ones: it does not
      // generalize, so report it as having no predictive value.
      rmse = std::sqrt(split.variance);
      return;
   }
   double total = 0.0;
   for (std::size_t row = 0 ; row < predictions.size() ; row++)
   {
      const double error = split.targets[row] - predictions[row];
      total += error * error;
   }
   const double mse = total / static_cast<double>(predictions.size());
   rmse = std::sqrt(mse);
   r2 = rSquared(mse, split.variance);
}

BaseString SymbolicRegression::genesFromValues(const std::vector<int>& values) const
{
   BaseString genes(static_cast<int>(values.size()), alphabetSize());
   for (int i = 0 ; i < static_cast<int>(values.size()) ; i++)
   {
      genes.setValue(i, values[i]);
   }
   return genes;
}

bool SymbolicRegression::selectByCrossValidation(std::vector<int>& genes, Fit& fit) const
{
   if (foldFit_.empty() || archive_.empty())
   {
      return false;
   }
   // One-standard-error rule: find the lowest cross-validated error, then
   // take the smallest formula (the archive is ordered by size) whose error
   // is within `selectionStandardErrors` standard errors of it.
   struct Scored { const Archived* archived; double mse; };
   std::vector<Scored> scored;
   double bestMse = std::numeric_limits<double>::infinity();
   double bestStandardError = 0.0;
   for (const auto& entry : archive_)
   {
      double mse = 0.0;
      double standardError = 0.0;
      if (!crossValidationStats(genesFromValues(entry.second.genes), mse, standardError))
      {
         continue;
      }
      scored.push_back(Scored{&entry.second, mse});
      if (mse < bestMse)
      {
         bestMse = mse;
         bestStandardError = standardError;
      }
   }
   const double threshold = bestMse + config_.selectionStandardErrors * bestStandardError;
   for (const Scored& candidate : scored)
   {
      if (candidate.mse <= threshold)
      {
         genes = candidate.archived->genes;
         fit = describe(genesFromValues(genes));
         fit.crossValidatedRmse = std::sqrt(candidate.mse);
         fit.crossValidatedR2 = rSquared(candidate.mse, train_.variance);
         return true;
      }
   }
   return false;
}

// Refit the formula (constants and scaling) on each fold's complement and
// collect the squared errors on the held-out fold, so every training row is
// predicted by a fit that never saw it.
bool SymbolicRegression::crossValidationStats(const BaseString& genes, double& mse, double& standardError) const
{
   const Expression structure = Expression::compile(genes, instructions_);
   std::vector<double> squared;
   squared.reserve(trainRows_.size());
   const std::vector<int> terms = modelTerms(structure);
   std::vector<std::vector<double> > fitValues;
   std::vector<std::vector<double> > heldValues;
   for (std::size_t fold = 0 ; fold < foldFit_.size() ; fold++)
   {
      Expression expression = structure;
      tuneConstants(expression, foldFit_[fold]);
      if (!evaluateTerms(expression, terms, foldFit_[fold].inputs, fitValues) ||
          !evaluateTerms(expression, terms, foldHeld_[fold].inputs, heldValues))
      {
         return false;
      }
      const TermModel model = fitTerms(fitValues, foldFit_[fold].targets);
      for (std::size_t i = 0 ; i < foldHeld_[fold].targets.size() ; i++)
      {
         const double error = foldHeld_[fold].targets[i] - predictRow(model, heldValues, i);
         squared.push_back(error * error);
      }
   }
   const double count = static_cast<double>(squared.size());
   mse = 0.0;
   for (double value : squared) mse += value;
   mse /= count;
   double variance = 0.0;
   for (double value : squared) variance += (value - mse) * (value - mse);
   variance /= count > 1.0 ? count - 1.0 : 1.0;
   standardError = std::sqrt(variance / count);
   return std::isfinite(mse);
}

std::string SymbolicRegression::scaledFormula(const Fit& fit) const
{
   std::ostringstream out;
   out << std::setprecision(4);
   if (!fit.valid)
   {
      return "(undefined on the data)";
   }
   std::vector<std::size_t> shown;
   for (std::size_t i = 0 ; i < fit.weights.size() ; i++)
   {
      if (fit.weights[i] != 0.0) shown.push_back(i);
   }
   if (shown.empty())
   {
      out << fit.intercept;
      return out.str();
   }
   // Each term is printed with its own fitted coefficient. The coefficient
   // belongs to the term as split off (any top-level minus sign removed), so
   // the term's own text is printed, not the whole formula's.
   bool first = true;
   if (std::fabs(fit.intercept) > 1e-9)
   {
      out << fit.intercept;
      first = false;
   }
   for (std::size_t i : shown)
   {
      const double weight = fit.weights[i];
      if (first)
      {
         if (weight < 0.0) out << '-';
      }
      else
      {
         out << (weight < 0.0 ? " - " : " + ");
      }
      first = false;
      // Omit a factor that prints as 1 at the displayed precision.
      std::ostringstream magnitude;
      magnitude << std::setprecision(4) << std::fabs(weight);
      if (magnitude.str() != "1")
      {
         out << magnitude.str() << '*';
      }
      out << '(' << fit.terms[i] << ')';
   }
   return out.str();
}

void SymbolicRegression::validatePopulation(const Population& population) const
{
   const Population::Settings& settings = population.settings();
   if (settings.baseStates != alphabetSize())
   {
      throw GAFatalException(__FILE__, __LINE__,
                             "SymbolicRegression requires baseStates equal to its instruction alphabet size");
   }
   if (settings.variableLength != Population::VariableLengthMode::Variable)
   {
      throw GAFatalException(__FILE__, __LINE__, "SymbolicRegression requires variable-length chromosomes");
   }
}

double SymbolicRegression::evaluateFitness(const BaseString& genes)
{
   const Fit fit = describe(genes);
   if (fit.valid)
   {
      // Keep the best (lowest training error) formula of each size for the
      // final cross-validated choice.
      auto found = archive_.find(fit.size);
      if (found == archive_.end() || fit.trainRmse < found->second.trainRmse)
      {
         Archived& slot = archive_[fit.size];
         slot.trainRmse = fit.trainRmse;
         slot.genes.resize(genes.length());
         for (int i = 0 ; i < genes.length() ; i++)
         {
            slot.genes[i] = genes.valueAt(i);
         }
      }
   }
   return fit.fitness;
}

void SymbolicRegression::printCandidate(const BaseString& genes, std::ostream& out) const
{
   const Fit fit = describe(genes);
   out << dataset_.targetName << " ~= " << scaledFormula(fit);
   if (fit.valid)
   {
      out << std::fixed << std::setprecision(4)
          << " | nodes=" << fit.size
          << " train R2=" << fit.trainR2 << " RMSE=" << fit.trainRmse;
      if (testRowCount() > 0)
      {
         out << " | test R2=" << fit.testR2 << " RMSE=" << fit.testRmse;
      }
      out << std::defaultfloat;
      // With several inputs, which ones the formula reads is a result in
      // itself (an input it never uses is one it found irrelevant).
      if (dataset_.inputCount() > 1)
      {
         out << " | uses " << fit.inputsUsed.size() << " of " << dataset_.inputCount() << " inputs:";
         for (const std::string& name : fit.inputsUsed)
         {
            out << ' ' << name;
         }
      }
   }
   out << " ::";
}

void SymbolicRegression::writeVisualizationJson(const BaseString& genes, std::ostream& out) const
{
   const Fit fit = describe(genes);
   Expression expression = Expression::compile(genes, instructions_);
   tuneConstants(expression, train_);
   std::vector<double> predictions;
   const bool defined = predictRows(expression, fit, dataset_.inputs, predictions);

   out << "{ \"type\": \"regression\", \"target\": ";
   writeJsonString(out, dataset_.targetName);
   out << ", \"inputs\": [";
   for (int i = 0 ; i < dataset_.inputCount() ; i++)
   {
      if (i > 0) out << ", ";
      writeJsonString(out, dataset_.inputNames[i]);
   }
   out << "], \"formula\": ";
   writeJsonString(out, scaledFormula(fit));
   out << std::setprecision(10)
       << ", \"nodes\": " << fit.size
       << ", \"inputs_used\": [";
   for (std::size_t i = 0 ; i < fit.inputsUsed.size() ; i++)
   {
      if (i > 0) out << ", ";
      writeJsonString(out, fit.inputsUsed[i]);
   }
   out << "]"
       << ", \"train_r2\": " << (fit.valid ? fit.trainR2 : 0.0)
       << ", \"test_r2\": " << (fit.valid ? fit.testR2 : 0.0)
       << ", \"points\": [";

   std::vector<char> isTest(dataset_.rowCount(), 0);
   for (int row : testRows_) isTest[row] = 1;
   for (int row = 0 ; row < dataset_.rowCount() ; row++)
   {
      if (row > 0) out << ", ";
      out << "{ \"inputs\": [";
      for (int i = 0 ; i < dataset_.inputCount() ; i++)
      {
         if (i > 0) out << ", ";
         out << dataset_.inputs[row][i];
      }
      out << "], \"y\": " << dataset_.targets[row];
      if (defined)
      {
         out << ", \"predicted\": " << predictions[row];
      }
      out << ", \"set\": \"" << (isTest[row] ? "test" : "train") << "\" }";
   }
   out << "], \"curve\": [";

   // For a single input, sample the fitted formula across the input range so
   // the viewer can draw it as a curve; undefined samples are omitted.
   if (fit.valid && dataset_.inputCount() == 1)
   {
      double low = dataset_.inputs[0][0];
      double high = low;
      for (const std::vector<double>& row : dataset_.inputs)
      {
         low = std::min(low, row[0]);
         high = std::max(high, row[0]);
      }
      bool first = true;
      for (int sample = 0 ; sample < kCurveSamples ; sample++)
      {
         const double x = low + (high - low) * sample / (kCurveSamples - 1);
         std::vector<double> value;
         if (!predictRows(expression, fit, std::vector<std::vector<double> >(1, std::vector<double>(1, x)), value))
         {
            continue;
         }
         if (!first) out << ", ";
         first = false;
         out << "{ \"x\": " << x << ", \"y\": " << value[0] << " }";
      }
   }
   out << "]";

   // With several inputs: an effect curve per input and each term's share of
   // the explained variance (see the header).
   if (fit.valid && dataset_.inputCount() > 1)
   {
      writeEffectsJson(expression, fit, out);
      writeTermSharesJson(expression, fit, out);
   }
   out << " }" << std::defaultfloat << std::setprecision(6);
}

// Partial dependence: for each input, the prediction averaged over every
// data row with that input set to each value on a grid across its range.
// Averaging over the data (rather than holding the other inputs at fixed
// values) keeps interactions such as x1*x2 honest. A grid value at which the
// formula is undefined for some row is left out.
void SymbolicRegression::writeEffectsJson(const Expression& expression, const Fit& fit, std::ostream& out) const
{
   out << ", \"effects\": [";
   std::vector<std::vector<double> > rows = dataset_.inputs;
   std::vector<double> predictions;
   for (int column = 0 ; column < dataset_.inputCount() ; column++)
   {
      double low = dataset_.inputs[0][column];
      double high = low;
      for (const std::vector<double>& row : dataset_.inputs)
      {
         low = std::min(low, row[column]);
         high = std::max(high, row[column]);
      }
      out << (column == 0 ? "" : ", ") << "{ \"input\": ";
      writeJsonString(out, dataset_.inputNames[column]);
      out << ", \"curve\": [";
      bool first = true;
      for (int step = 0 ; step < kEffectPoints ; step++)
      {
         const double x = low + (high - low) * step / (kEffectPoints - 1);
         for (std::vector<double>& row : rows) row[column] = x;
         if (!predictRows(expression, fit, rows, predictions))
         {
            continue;
         }
         double mean = 0.0;
         for (double value : predictions) mean += value;
         mean /= static_cast<double>(predictions.size());
         out << (first ? "" : ", ") << "[" << x << ", " << mean << "]";
         first = false;
      }
      for (std::size_t row = 0 ; row < rows.size() ; row++) rows[row][column] = dataset_.inputs[row][column];
      out << "] }";
   }
   out << "]";
}

// Each additive term's share of the explained variance on the training rows:
// the variance of weight * term, scaled so the shares add up to the training
// R^2 (correlated terms share credit in proportion to their own variance).
// Each term also lists the inputs it involves, which identifies it across
// generations better than its text does.
void SymbolicRegression::writeTermSharesJson(const Expression& expression, const Fit& fit, std::ostream& out) const
{
   const std::vector<int> terms = modelTerms(expression);
   std::vector<std::vector<double> > values;
   out << ", \"terms\": [";
   if (terms.size() != fit.weights.size() || !evaluateTerms(expression, terms, train_.inputs, values))
   {
      out << "]";
      return;
   }
   std::vector<double> variances(terms.size(), 0.0);
   double total = 0.0;
   for (std::size_t i = 0 ; i < terms.size() ; i++)
   {
      double mean = 0.0;
      for (double value : values[i]) mean += value;
      mean /= static_cast<double>(values[i].size());
      for (double value : values[i])
      {
         const double centred = fit.weights[i] * (value - mean);
         variances[i] += centred * centred;
      }
      total += variances[i];
   }
   for (std::size_t i = 0 ; i < terms.size() ; i++)
   {
      const double share = total > 0.0 ? fit.trainR2 * variances[i] / total : 0.0;
      out << (i == 0 ? "" : ", ") << "{ \"text\": ";
      writeJsonString(out, fit.terms[i]);
      out << ", \"weight\": " << fit.weights[i] << ", \"share\": " << share << ", \"inputs\": [";
      const std::vector<int> used = expression.variablesUsedBy(terms[i]);
      for (std::size_t v = 0 ; v < used.size() ; v++)
      {
         out << (v == 0 ? "" : ", ");
         writeJsonString(out, dataset_.inputNames[used[v]]);
      }
      out << "] }";
   }
   out << "]";
}

void SymbolicRegression::writeSampleJson(const BaseString& genes, std::ostream& out) const
{
   const Fit fit = describe(genes);
   Expression expression = Expression::compile(genes, instructions_);
   tuneConstants(expression, train_);
   out << "{ \"type\": \"regression_sample\", \"formula\": ";
   writeJsonString(out, scaledFormula(fit));
   out << std::setprecision(6)
       << ", \"valid\": " << (fit.valid ? "true" : "false")
       << ", \"nodes\": " << fit.size
       << ", \"train_r2\": " << (fit.valid ? fit.trainR2 : 0.0);
   if (fit.valid && dataset_.inputCount() == 1)
   {
      double low = dataset_.inputs[0][0];
      double high = low;
      for (const std::vector<double>& row : dataset_.inputs)
      {
         low = std::min(low, row[0]);
         high = std::max(high, row[0]);
      }
      out << ", \"curve\": [";
      bool first = true;
      for (int sample = 0 ; sample < kSampleCurvePoints ; sample++)
      {
         const double x = low + (high - low) * sample / (kSampleCurvePoints - 1);
         std::vector<double> value;
         if (!predictRows(expression, fit, std::vector<std::vector<double> >(1, std::vector<double>(1, x)), value))
         {
            continue;
         }
         out << (first ? "" : ", ") << "[" << x << ", " << value[0] << "]";
         first = false;
      }
      out << "]";
   }
   else if (fit.valid)
   {
      std::vector<double> predictions;
      if (predictRows(expression, fit, dataset_.inputs, predictions))
      {
         out << ", \"predicted\": [";
         for (std::size_t row = 0 ; row < predictions.size() ; row++)
         {
            out << (row == 0 ? "" : ", ") << predictions[row];
         }
         out << "]";
      }
   }
   out << " }" << std::defaultfloat;
}

std::string SymbolicRegression::summarizePopulation(const std::vector<const BaseString*>& members) const
{
   // Decoding a formula is cheap (no evaluation), so every member is counted.
   std::vector<int> usage(dataset_.inputCount(), 0);
   for (const BaseString* genes : members)
   {
      for (int variable : Expression::compile(*genes, instructions_).variablesUsed())
      {
         ++usage[variable];
      }
   }
   std::ostringstream out;
   out << "{ \"members\": " << members.size() << ", \"input_usage\": [";
   for (std::size_t i = 0 ; i < usage.size() ; i++)
   {
      out << (i == 0 ? "" : ", ") << usage[i];
   }
   out << "] }";
   return out.str();
}

std::unique_ptr<Chromosome> SymbolicRegression::initializeCandidate(Population& population)
{
   std::mt19937& generator = population.randomEngine();
   const int length = static_cast<int>(gak::uniformInt(generator,
                                                       config_.minimumInitialLength,
                                                       config_.maximumInitialLength));
   BaseString genes(length, alphabetSize());
   for (int i = 0 ; i < length ; i++)
   {
      genes.setValue(i, gak::uniformIndex(generator, alphabetSize()));
   }
   return std::make_unique<Chromosome>(std::move(genes), true, alphabetSize());
}

std::unique_ptr<Chromosome> SymbolicRegression::truncated(std::unique_ptr<Chromosome> chromosome,
                                                          const Population&) const
{
   if (chromosome->length() <= config_.maximumLength)
   {
      return chromosome;
   }
   // Dropping the tail keeps a valid program (underflow is skipped), and
   // bounds how large formulas can grow.
   BaseString genes(config_.maximumLength, alphabetSize());
   for (int i = 0 ; i < config_.maximumLength ; i++)
   {
      genes.setValue(i, chromosome->genes().valueAt(i));
   }
   return std::make_unique<Chromosome>(std::move(genes), true, alphabetSize());
}

std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
SymbolicRegression::mateCandidates(Population& population, Chromosome& mother, Chromosome& father)
{
   std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
      mother.mate(father, population.settings().crossOverRate,
                  Chromosome::CrossoverType::SinglePoint, &population.randomEngine());
   return std::make_pair(truncated(std::move(children.first), population),
                         truncated(std::move(children.second), population));
}
