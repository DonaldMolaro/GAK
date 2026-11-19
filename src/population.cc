#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "population_report.hh"
#include "except.hh"
#include "random.hh"

namespace
{
constexpr double kFitnessSelectionEpsilon = 1e-12;
// Upper bound on mating attempts per replacement slot in one generation.
constexpr long kMaxMatingsPerReplacement = 50;
// Consecutive generations with zero insertions before a run is declared stalled.
constexpr int kMaxStalledGenerations = 50;

bool IsVerboseEnabled()
{
   return std::getenv("GAK_VERBOSE") != nullptr;
}
}

Population::Population(const Settings& settings, PopulationProblem& problem)
   : populationInitialized(false),
     problem_(problem),
     settings_(settings),
     activeRandomSeed_(0)
{
   if (settings_.useFixedRandomSeed)
   {
      setRandomSeed(settings_.randomSeed);
   }
   else
   {
      // random_device rather than time() so populations created in the same
      // second do not share a seed.
      setRandomSeed(std::random_device{}());
   }
   problem_.validatePopulation(*this);
}

Population::~Population() = default;

double Population::evaluateFitness(const BaseString& genes)
{
   return problem_.evaluateFitness(genes);
}

void Population::printCandidate(const BaseString& genes, std::ostream& out) const
{
   problem_.printCandidate(genes, out);
}

void Population::writeVisualizationJson(const BaseString& genes, std::ostream& out) const
{
   problem_.writeVisualizationJson(genes, out);
}

void PopulationProblem::validatePopulation(const Population&) const
{
}

void PopulationProblem::writeVisualizationJson(const BaseString& genes, std::ostream& out) const
{
   out << "{ \"gene_values\": [";
   for (int i = 0 ; i < genes.length() ; i++)
   {
      if (i > 0)
      {
         out << ", ";
      }
      out << genes.valueAt(i);
   }
   out << "] }";
}

bool PopulationProblem::hasReachedSolution(const Population&, const BaseString&, double) const
{
   return false;
}

std::unique_ptr<Chromosome> PopulationProblem::initializeCandidate(Population& population)
{
   return std::make_unique<Chromosome>(population.settings().chromosomeLength,
                                       population.settings().variableLength == Population::VariableLengthMode::Variable,
                                       population.settings().baseStates,
                                       &population.randomEngine());
}

std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
PopulationProblem::mateCandidates(Population& population, Chromosome& mother, Chromosome& father)
{
   return mother.mate(father,
                      population.settings().crossOverRate,
                      Chromosome::CrossoverType::SinglePoint,
                      &population.randomEngine());
}

void PopulationProblem::mutateCandidate(Population& population, Chromosome& chromosome)
{
   chromosome.mutate(population.settings().bitMutationRate, &population.randomEngine());
}

void Population::setRandomSeed(unsigned int seed)
{
   activeRandomSeed_ = seed;
   randomGenerator.seed(seed);
}

int Population::replacementCount() const
{
   switch (settings_.deletion)
   {
   case Population::DeletionMode::DeleteAll:
      return settings_.numberOfIndividuals;
   case Population::DeletionMode::DeleteAllButBest:
      return settings_.numberOfIndividuals - 1;
   case Population::DeletionMode::DeleteHalf:
      return  settings_.numberOfIndividuals / 2;
   case Population::DeletionMode::DeleteQuarter:
      return settings_.numberOfIndividuals / 4;
   case Population::DeletionMode::DeleteLast:
      return 2;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported deletion technique");
   }
}

int Population::replacementSlotIndex(int offset) const
{
   switch (settings_.operation)
   {
   case Population::OperationMode::Maximize:
      return offset;
   case Population::OperationMode::Minimize:
      return (settings_.numberOfIndividuals - 1) - offset;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
}

Population::PopulationSummary Population::buildPopulationSummary() const
{
   const int summaryCount = std::min(settings_.numberOfIndividuals, kSummaryCount);
   PopulationSummary summary;
   summary.mostFit.reserve(summaryCount);
   summary.leastFit.reserve(summaryCount);

   // The table is sorted ascending, so the fittest end depends on the
   // optimization direction.
   const bool maximize = settings_.operation == Population::OperationMode::Maximize;
   for ( int i = 0 ; i < summaryCount ; i++ )
   {
      const int highIndex = (settings_.numberOfIndividuals - 1) - i;
      summary.mostFit.push_back(fitnessTable[maximize ? highIndex : i]);
      summary.leastFit.push_back(fitnessTable[maximize ? i : highIndex]);
   }
   return summary;
}

int Population::bestCandidateIndex() const
{
   switch (settings_.operation)
   {
   case Population::OperationMode::Maximize:
      return settings_.numberOfIndividuals - 1;
   case Population::OperationMode::Minimize:
      return 0;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
}

bool Population::hasReachedSolution() const
{
   const int index = bestCandidateIndex();
   return problem_.hasReachedSolution(*this,
                                      populationTable[index].get()->genes(),
                                      fitnessTable[index]);
}

int Population::selectRandomParent()
{
   return randomIndex(settings_.numberOfIndividuals);
}

int Population::selectTournamentParent()
{
   // The table is sorted ascending by fitness, so the fittest entrant is the
   // one with the highest index when maximizing and the lowest when
   // minimizing: comparing indices is enough.
   const bool maximize = settings_.operation == Population::OperationMode::Maximize;
   int winner = randomIndex(settings_.numberOfIndividuals);
   for (int entrant = 1 ; entrant < settings_.tournamentSize ; entrant++)
   {
      const int challenger = randomIndex(settings_.numberOfIndividuals);
      if (maximize ? challenger > winner : challenger < winner)
      {
         winner = challenger;
      }
   }
   return winner;
}

int Population::randomIndex(int upperBoundExclusive)
{
   if (upperBoundExclusive <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"Random index requested with non-positive upper bound");
   }

   return gak::uniformIndex(randomGenerator, upperBoundExclusive);
}

long Population::randomBelow(long upperBoundExclusive)
{
   if (upperBoundExclusive <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"Random value requested with non-positive upper bound");
   }

   return gak::uniformInt(randomGenerator, 0, upperBoundExclusive - 1);
}

bool Population::appendReplacement(std::vector<std::unique_ptr<Chromosome> >& replacementList,
                                   std::unique_ptr<Chromosome> candidate,
                                   int numberToReplace,
                                   bool allowDuplicates)
{
   if (!candidate)
   {
      return false;
   }

   if (!allowDuplicates && containsChromosome(*candidate,replacementList,static_cast<int>(replacementList.size())))
   {
      return false;
   }

   if (static_cast<int>(replacementList.size()) < numberToReplace)
   {
      replacementList.push_back(std::move(candidate));
      return true;
   }

   return false;
}
Population::RunResult Population::executeInternal(bool captureGenerationSummaries)
{
   RunResult result;
   result.randomSeed = activeRandomSeed_;
   result.usedConfiguredSeed = settings_.useFixedRandomSeed;
   if (settings_.numberOfIndividuals <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"Population requires at least one individual");
   }
   if (settings_.parentSelection == Population::ParentSelectionMode::Tournament &&
       settings_.tournamentSize < 1)
   {
      throw GAFatalException(__FILE__,__LINE__,"Tournament size must be at least 1");
   }
   if (!(settings_.crossOverRate >= 0.0 && settings_.crossOverRate <= 1.0))
   {
      throw GAFatalException(__FILE__,__LINE__,"Crossover rate must be between 0 and 1");
   }
   if (!(settings_.bitMutationRate >= 0.0 && settings_.bitMutationRate <= 1.0))
   {
      throw GAFatalException(__FILE__,__LINE__,"Mutation rate must be between 0 and 1");
   }
   const int numToReplace = replacementCount();
   if (numToReplace < 1 || numToReplace > settings_.numberOfIndividuals)
   {
      // A generation that replaces nobody never advances the trial count,
      // so the run would never terminate.
      throw GAFatalException(__FILE__,__LINE__,
                             "Deletion mode replaces no individuals (or more than exist) for this population size");
   }
   int numberBorn = initializePopulation();
   evaluatePopulation();
   if (hasReachedSolution())
   {
      result.stoppedEarly = true;
      result.solutionFound = true;
      result.generationsCompleted = 0;
      result.evaluations = numberBorn;
      result.finalSummary = buildPopulationSummary();
      return result;
   }
   int numGen = 0;
   int stalledGenerations = 0;
   while ( numberBorn < settings_.numberOfTrials )
   {
      std::vector<std::unique_ptr<Chromosome> > replacementList = breedPopulation(numToReplace);
      const int inserted = insertNewPopulation(std::move(replacementList),numToReplace);
      numberBorn += inserted;
      stalledGenerations = (inserted == 0) ? stalledGenerations + 1 : 0;
      evaluatePopulation();
      const bool solutionFound = hasReachedSolution();
      if (captureGenerationSummaries)
      {
         PopulationSummary summary = buildPopulationSummary();
         GenerationReport report;
         report.generation = numGen;
         report.evaluations = numberBorn;
         report.summary = summary;
         const int bestIndex = bestCandidateIndex();
         const BaseString& bestGenes = populationTable[bestIndex].get()->genes();
         report.bestCandidateGenes.reserve(bestGenes.length());
         for (int gene = 0 ; gene < bestGenes.length() ; gene++)
         {
            report.bestCandidateGenes.push_back(bestGenes.valueAt(gene));
         }
         report.bestCandidateFitness = fitnessTable[bestIndex];
         result.generationReports.push_back(report);
         result.generationSummaries.push_back(summary);
      }
      ++numGen;
      if (solutionFound)
      {
         result.stoppedEarly = true;
         result.solutionFound = true;
         break;
      }
      if (stalledGenerations >= kMaxStalledGenerations)
      {
         // Duplicate rejection has stopped producing any new individuals
         // (typically a converged population), so the trial budget can never
         // be consumed.
         result.stoppedEarly = true;
         result.stalled = true;
         break;
      }
   }

   result.generationsCompleted = numGen;
   result.evaluations = numberBorn;
   result.finalSummary = buildPopulationSummary();
   return result;
}

Population::RunResult Population::execute(bool captureGenerationSummaries)
{
   return executeInternal(captureGenerationSummaries);
}

void Population::run(std::ostream& out, const PopulationRunReportOptions& options)
{
   RunResult result = execute(options.includeGenerationSummaries);
   PopulationReporter::write(out, *this, result, options);
}

void Population::run()
{
   const bool verbose = IsVerboseEnabled();
   run(std::cerr, PopulationRunReportOptions{verbose, verbose});
}
//
// Start Population with randomly generated population.
//
//
int Population::initializePopulation()
{
   populationTable.clear();
   populationTable.reserve(settings_.numberOfIndividuals);
   fitnessTable.assign(settings_.numberOfIndividuals, 0.0);
   fitnessEvaluated.assign(settings_.numberOfIndividuals, false);
   windowedFitnessTable.assign(settings_.numberOfIndividuals, 0.0);
   linearNormalizedfitnessTable.assign(settings_.numberOfIndividuals, 0.0);
   
   for ( int i = 0 ; i < settings_.numberOfIndividuals ; i++ )
   {
      populationTable.push_back(createInitialChromosome());
   }
   populationInitialized = true;
   return settings_.numberOfIndividuals;
}
//
// Evaluate all unevalueated members of the population pool.
// fitness must be a positive double.
//
void Population::evaluatePopulation()
{
   for ( int i = 0 ; i < settings_.numberOfIndividuals ; i++ )
   {
      if (!fitnessEvaluated[i])
      {
	 const double fitness = problem_.evaluateFitness(populationTable[i].get()->genes());
	 if (!std::isfinite(fitness))
	 {
	    // NaN breaks the ordering std::stable_sort relies on, and neither
	    // NaN nor infinity can be written to a JSON report.
	    throw GAFatalException(__FILE__,__LINE__,"evaluateFitness returned a non-finite value (NaN or infinity)");
	 }
	 fitnessTable[i] = fitness;
         fitnessEvaluated[i] = true;
      }
   }
   sortPopulation();
   //
   // The population is now sorted in accending order, 
   // members with the highest fitness are now in the last position of
   // table...
   //
   for ( int i = 0 ; i < settings_.numberOfIndividuals ; i++ )
   {
      linearNormalizedfitnessTable[i] = i + 1.00;
      windowedFitnessTable[i] = (settings_.operation == Population::OperationMode::Maximize)
	 ? (fitnessTable[i] - fitnessTable[0] + 1.00)
	 : (fitnessTable[settings_.numberOfIndividuals - 1] - fitnessTable[i] + 1.00);
   }
   return;
}
//
// Roulette wheel parent selection.
//
// The wheel (cumulative weights, already oriented so that larger means
// fitter) is built once per generation by `buildSelectionWheel`, and each
// parent pick is then a single binary search, so breeding is O(N log N)
// rather than O(N^2).
//
Population::SelectionWheel Population::buildSelectionWheel(const std::vector<double>& rouletteTable) const
{
   const int count = settings_.numberOfIndividuals;
   std::vector<double> weights(rouletteTable.begin(), rouletteTable.begin() + count);

   switch (settings_.operation)
   {
   case Population::OperationMode::Maximize:
      if (settings_.fitness == Population::FitnessMode::Evaluation)
      {
         for (double weight : weights)
         {
            if (weight < 0.0)
            {
               throw GAFatalException(__FILE__,__LINE__,
                                      "Roulette selection with FitnessMode::Evaluation requires non-negative "
                                      "fitness when maximizing; use FitnessMode::Windowed or LinearNormalized");
            }
         }
      }
      break;
   case Population::OperationMode::Minimize:
      // Windowed weights are already oriented so the best (smallest)
      // fitness gets the largest weight; inverting them again would favour
      // the worst candidates. Other tables are inverted around their maximum
      // so every weight is at least 1.
      if (settings_.fitness != Population::FitnessMode::Windowed && count > 0)
      {
         const double maximumFitness = *std::max_element(weights.begin(), weights.end());
         for (double& weight : weights)
         {
            weight = (maximumFitness - weight) + 1.00;
         }
      }
      break;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }

   SelectionWheel wheel;
   wheel.cumulative.resize(count);
   double total = 0.0;
   for (int i = 0 ; i < count ; i++)
   {
      total += weights[i];
      wheel.cumulative[i] = total;
   }
   wheel.total = total;
   wheel.uniform = total <= kFitnessSelectionEpsilon;
   return wheel;
}

int Population::spinSelectionWheel(const SelectionWheel& wheel)
{
   const int count = settings_.numberOfIndividuals;
   if (wheel.uniform)
   {
      return randomIndex(count);
   }

   const double selectValue = gak::uniformReal(randomGenerator, 0.0, wheel.total);
   // upper_bound so a pick of exactly 0.0 cannot land on a zero-weight entry.
   const auto found = std::upper_bound(wheel.cumulative.begin(), wheel.cumulative.end(), selectValue);
   if (found != wheel.cumulative.end())
   {
      return static_cast<int>(found - wheel.cumulative.begin());
   }

   // Floating-point slack put the pick past the end: fall back to the last
   // entry that actually carries weight.
   for (int i = count - 1 ; i >= 0 ; --i)
   {
      const double previous = (i == 0) ? 0.0 : wheel.cumulative[i - 1];
      if (wheel.cumulative[i] - previous > kFitnessSelectionEpsilon)
      {
         return i;
      }
   }
   return randomIndex(count);
}

int Population::selectParent(const std::vector<double>& rouletteTable)
{
   return spinSelectionWheel(buildSelectionWheel(rouletteTable));
}

const std::vector<double>& Population::selectFitnessWeights()
{
   switch (settings_.fitness)
   {
   case Population::FitnessMode::Evaluation:
      return fitnessTable;
   case Population::FitnessMode::Windowed:
      return windowedFitnessTable;
   case Population::FitnessMode::LinearNormalized:
      return linearNormalizedfitnessTable;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported fitness technique");
   }
}
//
// Generate a population of Children.
// Select a set of parrents and mate them to generate two
// Children.
//
std::vector<std::unique_ptr<Chromosome> > Population::breedPopulation(int numberToReplace)
{
   std::vector<std::unique_ptr<Chromosome> > replacementList;
   replacementList.reserve(numberToReplace);
  switch (settings_.parentSelection)
   {
   case Population::ParentSelectionMode::RouletteWheel:
   case Population::ParentSelectionMode::Random:
   case Population::ParentSelectionMode::Tournament:
   {
      const bool roulette = settings_.parentSelection == Population::ParentSelectionMode::RouletteWheel;
      const bool tournament = settings_.parentSelection == Population::ParentSelectionMode::Tournament;
      // Weights cannot change while breeding, so build the wheel once.
      const SelectionWheel wheel = roulette ? buildSelectionWheel(selectFitnessWeights()) : SelectionWheel();
      auto pickParent = [&]() {
         if (roulette) return spinSelectionWheel(wheel);
         if (tournament) return selectTournamentParent();
         return selectRandomParent();
      };
      // Bound the number of matings so duplicate rejection cannot loop
      // forever when fewer distinct children exist than slots to fill.
      for (long matings = 0 ;
           static_cast<int>(replacementList.size()) < numberToReplace &&
           matings < static_cast<long>(numberToReplace) * kMaxMatingsPerReplacement ;
           ++matings)
      {
	 const int fatherIndex = pickParent();
	 const int motherIndex = pickParent();
         std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
            mateChromosomes(*populationTable[motherIndex], *populationTable[fatherIndex]);
         mutateChromosome(*children.first);
         mutateChromosome(*children.second);
	 //
	 // if this child is required for the next generation
	 // keep it. otherwise abort it.
	 // Practically son's will never be aborted just daughters.
	 //
	 switch (settings_.reproduction)
	 {
	 case Population::ReproductionMode::AllowDuplicates:
	    appendReplacement(replacementList,std::move(children.first),numberToReplace,true);
	    //
	    // Keep the daugter if necessary to complete the
	    // replacement for the next generation.
	    //
	    appendReplacement(replacementList,std::move(children.second),numberToReplace,true);
	    break;
	 case Population::ReproductionMode::DisallowDuplicates:
	    appendReplacement(replacementList,std::move(children.first),numberToReplace,false);
	    //
	    // Keep the daugter if necessary to complete the
	    // replacement for the next generation.
	    //
	    appendReplacement(replacementList,std::move(children.second),numberToReplace,false);
	    break;
	 }
      }
      break;
   }
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported parent selection technique");
   }
   return replacementList;
}

std::unique_ptr<Chromosome> Population::createInitialChromosome()
{
   return problem_.initializeCandidate(*this);
}

std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
Population::mateChromosomes(Chromosome& mother, Chromosome& father)
{
   return problem_.mateCandidates(*this, mother, father);
}

void Population::mutateChromosome(Chromosome& chromosome)
{
   problem_.mutateCandidate(*this, chromosome);
}

bool Population::containsChromosome(const Chromosome& candidate,
                                    const std::vector<std::unique_ptr<Chromosome> >& pop,
                                    int populationLength) const
{
   for ( int i = 0 ; i < populationLength ; i++ )
   {
      if (candidate.equals(*pop[i]))
      {
	 return true;
      }
   }
   return false;
}
//
// Delete members of existing population to make room for the
// new children to be inserted.
// Population table must be sorted in decending order; the highest 
// fitness are at the end.
//
int Population::insertNewPopulation(std::vector<std::unique_ptr<Chromosome> > replacementList,int numToReplace)
{
   if (numToReplace > settings_.numberOfIndividuals)
   {
      throw GAFatalException(__FILE__,__LINE__,"Cannot replace more individuals than exist in the population");
   }
   int replaced = 0;
   const int available = std::min(numToReplace, static_cast<int>(replacementList.size()));
   for ( int i = 0 ; i < available; i++ )
   {
      const int index = replacementSlotIndex(i);
      if (settings_.reproduction == Population::ReproductionMode::DisallowDuplicates &&
          containsChromosome(*replacementList[i],populationTable,settings_.numberOfIndividuals))
      {
         continue;
      }

      fitnessTable[index] = 0.0;
      fitnessEvaluated[index] = false;
      populationTable[index] = std::move(replacementList[i]);
      replaced++;
   }
   return replaced;
}
//
// Sort population in decending order; most fit members will be in
// the last position.
//
void Population::sortPopulation()
{
   std::vector<int> order(settings_.numberOfIndividuals);
   for ( int i = 0 ; i < settings_.numberOfIndividuals ; i++ )
   {
      order[i] = i;
   }

   // stable_sort so tied fitness values keep a defined order on every
   // standard library, which keeps fixed-seed runs reproducible.
   std::stable_sort(order.begin(), order.end(),
             [this](int left, int right) { return fitnessTable[left] < fitnessTable[right]; });

   std::vector<double> sortedFitness(settings_.numberOfIndividuals);
   std::vector<bool> sortedFitnessEvaluated(settings_.numberOfIndividuals);
   std::vector<std::unique_ptr<Chromosome> > sortedPopulation(settings_.numberOfIndividuals);
   for ( int i = 0 ; i < settings_.numberOfIndividuals ; i++ )
   {
      sortedFitness[i] = fitnessTable[order[i]];
      sortedFitnessEvaluated[i] = fitnessEvaluated[order[i]];
      sortedPopulation[i] = std::move(populationTable[order[i]]);
   }

   fitnessTable.swap(sortedFitness);
   fitnessEvaluated.swap(sortedFitnessEvaluated);
   populationTable.swap(sortedPopulation);
}
