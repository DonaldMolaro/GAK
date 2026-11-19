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

struct Scaling
{
   double intercept = 0.0;
   double slope = 0.0;
};

// Least-squares a, b minimizing sum (y - (a + b p))^2. A (near-)constant
// prediction carries no shape information, so it falls back to the mean.
Scaling fitScaling(const std::vector<double>& predictions, const std::vector<double>& targets)
{
   const std::size_t count = targets.size();
   double meanP = 0.0;
   double meanY = 0.0;
   for (std::size_t i = 0 ; i < count ; i++)
   {
      meanP += predictions[i];
      meanY += targets[i];
   }
   meanP /= static_cast<double>(count);
   meanY /= static_cast<double>(count);

   double covariance = 0.0;
   double variance = 0.0;
   for (std::size_t i = 0 ; i < count ; i++)
   {
      covariance += (predictions[i] - meanP) * (targets[i] - meanY);
      variance += (predictions[i] - meanP) * (predictions[i] - meanP);
   }

   Scaling scaling;
   if (variance <= kVarianceEpsilon * static_cast<double>(count))
   {
      scaling.intercept = meanY;
      return scaling;
   }
   scaling.slope = covariance / variance;
   scaling.intercept = meanY - scaling.slope * meanP;
   return scaling;
}

double meanSquaredError(const std::vector<double>& predictions,
                        const std::vector<double>& targets,
                        const Scaling& scaling)
{
   double total = 0.0;
   for (std::size_t i = 0 ; i < targets.size() ; i++)
   {
      const double error = targets[i] - (scaling.intercept + scaling.slope * predictions[i]);
      total += error * error;
   }
   return total / static_cast<double>(targets.size());
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
     instructions_(dataset_.inputCount())
{
   if (!(config_.holdoutFraction >= 0.0 && config_.holdoutFraction < 1.0))
   {
      throw GAFatalException(__FILE__, __LINE__, "Holdout fraction must be at least 0 and below 1");
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

double SymbolicRegression::trainingError(const Expression& expression) const
{
   thread_local std::vector<double> predictions;
   if (!expression.evaluate(train_.inputs, predictions))
   {
      return std::numeric_limits<double>::infinity();
   }
   const double mse = meanSquaredError(predictions, train_.targets, fitScaling(predictions, train_.targets));
   return std::isfinite(mse) ? mse : std::numeric_limits<double>::infinity();
}

// Deterministic pattern search over the formula's constants: try moving each
// constant up or down by a relative step, keep any move that lowers the
// training error, and halve the step when no move helps. The GA chooses the
// structure; this finds values such as the 0.3 in exp(-0.3 t) that the
// constant table cannot express directly. No randomness is used, so the
// result depends only on the formula and the data.
void SymbolicRegression::tuneConstants(Expression& expression) const
{
   const std::vector<int> constants = expression.constantNodes();
   if (constants.empty() || config_.constantTuningBudget <= 0)
   {
      return;
   }
   double best = trainingError(expression);
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
            const double error = trainingError(expression);
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
   tuneConstants(expression);
   fit.size = expression.size();
   fit.formula = expression.toString(dataset_.inputNames);

   std::vector<double> predictions;
   if (!expression.evaluate(train_.inputs, predictions))
   {
      return fit;
   }
   const Scaling scaling = fitScaling(predictions, train_.targets);
   const double trainMse = meanSquaredError(predictions, train_.targets, scaling);
   if (!std::isfinite(trainMse))
   {
      return fit;
   }

   fit.valid = true;
   fit.intercept = scaling.intercept;
   fit.slope = scaling.slope;
   fit.trainRmse = std::sqrt(trainMse);
   fit.trainR2 = rSquared(trainMse, train_.variance);
   fit.fitness = trainMse / train_.variance + config_.parsimony * fit.size;

   if (!test_.targets.empty())
   {
      std::vector<double> testPredictions;
      if (expression.evaluate(test_.inputs, testPredictions))
      {
         const double testMse = meanSquaredError(testPredictions, test_.targets, scaling);
         fit.testRmse = std::sqrt(testMse);
         fit.testR2 = rSquared(testMse, test_.variance);
      }
      else
      {
         // Defined on the training rows but not on held-out ones: it does
         // not generalize, so report it as having no predictive value.
         fit.testRmse = std::sqrt(test_.variance);
         fit.testR2 = 0.0;
      }
   }
   return fit;
}

std::string SymbolicRegression::scaledFormula(const Fit& fit) const
{
   std::ostringstream out;
   out << std::setprecision(4);
   if (!fit.valid)
   {
      return "(undefined on the data)";
   }
   if (fit.slope == 0.0)
   {
      out << fit.intercept;
      return out.str();
   }
   const bool hasIntercept = std::fabs(fit.intercept) > 1e-9;
   if (hasIntercept)
   {
      out << fit.intercept << (fit.slope < 0.0 ? " - " : " + ");
   }
   else if (fit.slope < 0.0)
   {
      out << '-';
   }
   // Omit a factor that prints as 1 at the displayed precision.
   std::ostringstream magnitude;
   magnitude << std::setprecision(4) << std::fabs(fit.slope);
   if (magnitude.str() != "1")
   {
      out << magnitude.str() << '*';
   }
   out << '(' << fit.formula << ')';
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
   return describe(genes).fitness;
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
   }
   out << " ::";
}

void SymbolicRegression::writeVisualizationJson(const BaseString& genes, std::ostream& out) const
{
   const Fit fit = describe(genes);
   Expression expression = Expression::compile(genes, instructions_);
   tuneConstants(expression);
   std::vector<double> predictions;
   const bool defined = fit.valid && expression.evaluate(dataset_.inputs, predictions);

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
         out << ", \"predicted\": " << (fit.intercept + fit.slope * predictions[row]);
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
         if (!expression.evaluate(std::vector<std::vector<double> >(1, std::vector<double>(1, x)), value))
         {
            continue;
         }
         if (!first) out << ", ";
         first = false;
         out << "{ \"x\": " << x << ", \"y\": " << (fit.intercept + fit.slope * value[0]) << " }";
      }
   }
   out << "] }" << std::defaultfloat << std::setprecision(6);
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
