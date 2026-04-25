//
// Population Module Genectic algorithim implementation.
//
// Author:
//        Donald Molaro
//
// Revision History:
//
// Genesis         Sept 1994.
// General Debugging and Documentation:
//                 Sept 27 - 30 1994.
//
// Added in Options for not allowing duplicates in
// population.     Oct 2-3 1994.
//
// Added in support for vaiable length chromosome strings
// and symbolic ( rather than binary ) bases.
//  
//
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "except.hh"

namespace
{
bool IsVerboseEnabled()
{
   return std::getenv("GAK_VERBOSE") != nullptr;
}
}

Population::Population(const Settings& settings)
   : populationInitialized(false),
     settings_(settings),
     activeRandomSeed_(0)
{
   if (settings_.useFixedRandomSeed)
   {
      setRandomSeed(settings_.randomSeed);
   }
   else
   {
      std::time_t currenttime;
      std::time(&currenttime);
      setRandomSeed(static_cast<unsigned int>(currenttime));
   }
}

Population::~Population() = default;

void Population::setInitializationStrategy(std::unique_ptr<InitializationStrategy> strategy)
{
   initializationStrategy_ = std::move(strategy);
}

void Population::setMatingStrategy(std::unique_ptr<MatingStrategy> strategy)
{
   matingStrategy_ = std::move(strategy);
}

void Population::setMutationStrategy(std::unique_ptr<MutationStrategy> strategy)
{
   mutationStrategy_ = std::move(strategy);
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

   for ( int i = 0 ; i < summaryCount ; i++ )
   {
      summary.mostFit.push_back(fitnessTable[(settings_.numberOfIndividuals - 1) - i]);
      summary.leastFit.push_back(fitnessTable[i]);
   }
   return summary;
}

int Population::selectRandomParent()
{
   return randomIndex(settings_.numberOfIndividuals);
}

int Population::randomIndex(int upperBoundExclusive)
{
   if (upperBoundExclusive <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"Random index requested with non-positive upper bound");
   }

   std::uniform_int_distribution<int> distribution(0, upperBoundExclusive - 1);
   return distribution(randomGenerator);
}

long Population::randomBelow(long upperBoundExclusive)
{
   if (upperBoundExclusive <= 0)
   {
      throw GAFatalException(__FILE__,__LINE__,"Random value requested with non-positive upper bound");
   }

   std::uniform_int_distribution<long> distribution(0, upperBoundExclusive - 1);
   return distribution(randomGenerator);
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



//
// Outline of Genectic Algorithim taken from
// 
//    "Handbook of Genectic Algorithims"
//     Ed. Lawrence Davis.
//     Van Nostrand Reinhold, 1991.
//
// Expanded on significantly by author.
//
Population::RunResult Population::executeInternal(bool captureGenerationSummaries)
{
   RunResult result;
   result.randomSeed = activeRandomSeed_;
   result.usedConfiguredSeed = settings_.useFixedRandomSeed;
   int numberBorn = initializePopulation();
   evaluatePopulation();
   int numGen = 0;
   const int numToReplace = replacementCount();
   while ( numberBorn < settings_.numberOfTrials )
   {
      std::vector<std::unique_ptr<Chromosome> > replacementList = breedPopulation(numToReplace);
      numberBorn += insertNewPopulation(std::move(replacementList),numToReplace);
      evaluatePopulation();
      if (captureGenerationSummaries)
      {
         PopulationSummary summary = buildPopulationSummary();
         GenerationReport report;
         report.generation = numGen;
         report.evaluations = numberBorn;
         report.summary = summary;
         result.generationReports.push_back(report);
         result.generationSummaries.push_back(summary);
      }
      ++numGen;
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

void Population::RunReporter::write(std::ostream& out,
                                    const Population& population,
                                    const RunResult& result,
                                    const RunReportOptions& options)
{
   if (options.includeSettings)
   {
      const Population::Settings& settings = population.settings_;
      out << "Operation             :: "
          << (settings.operation == Population::OperationMode::Minimize ? "Minimize" : "Maximize") << '\n';
      out << "Number of Individuals :: " << settings.numberOfIndividuals << '\n';
      out << "Number of Trials      :: " << settings.numberOfTrials << '\n';
      out << "Genetic Diversity     :: " << settings.geneticDiversity << '\n';
      out << "Mutation Rate         :: " << std::fixed << std::setprecision(4)
          << settings.bitMutationRate << '\n';
      out << "Cross Over Rate       :: " << std::fixed << std::setprecision(3)
          << settings.crossOverRate << '\n' << std::defaultfloat;
      out << "Duplicate Reproduction:: "
          << (settings.reproduction == Population::ReproductionMode::DisallowDuplicates ? "NOT Ok." : " Ok.") << '\n';
      out << "Variable              :: "
          << (settings.variableLength == Population::VariableLengthMode::Fixed ? "NOT Ok." : " Ok.") << '\n';
      out << "Random Seed           :: " << population.activeRandomSeed_
          << (settings.useFixedRandomSeed ? " (configured)" : " (generated)") << '\n';
   }

   auto printPopulationSummary = [&](const PopulationSummary& summary) {
      const int summaryCount = static_cast<int>(summary.mostFit.size());
      if (summaryCount <= 0)
      {
         return;
      }

      if (population.settings_.operation == Population::OperationMode::Maximize)
         out << "Best " << summaryCount << " Members are:\n";
      else
         out << "Worst " << summaryCount << " Members are:\n";

      for ( int i = 0 ; i < summaryCount ; i++ )
      {
         population.printCandidate(population.populationTable[(population.settings_.numberOfIndividuals - 1) - i].get()->genes(), out);
         out << "(F = " << std::fixed << std::setprecision(8) << summary.mostFit[i]
             << ")\n" << std::defaultfloat;
      }

      if (population.settings_.operation == Population::OperationMode::Maximize)
         out << "Worst " << summaryCount << " Members are:\n";
      else
         out << "Best " << summaryCount << " Members are:\n";

      for ( int i = 0 ; i < summaryCount ; i++ )
      {
         population.printCandidate(population.populationTable[i].get()->genes(), out);
         out << "(F = " << std::fixed << std::setprecision(8) << summary.leastFit[i]
             << ")\n" << std::defaultfloat;
      }
   };

   auto printGenerationProgress = [&](const GenerationReport& report, bool printSummary) {
      out << "Generation " << report.generation
          << " Number of Evaluations " << report.evaluations << " \n";
      if (printSummary)
      {
         printPopulationSummary(report.summary);
      }
   };

   if (options.includeGenerationSummaries)
   {
      for (std::size_t i = 0 ; i < result.generationReports.size() ; i++ )
      {
         printGenerationProgress(result.generationReports[i], true);
      }
   }
   else
   {
      const GenerationReport finalProgress = {
         result.generationsCompleted,
         result.evaluations,
         PopulationSummary()
      };
      printGenerationProgress(finalProgress, false);
   }

   const int summaryCount = static_cast<int>(result.finalSummary.mostFit.size());
   switch(population.settings_.operation)
   {
      case Population::OperationMode::Maximize:
      for ( int i = 0 ; i < summaryCount ; i++ )
      {
	 population.printCandidate(population.populationTable[(population.settings_.numberOfIndividuals - 1) - i].get()->genes(), out);
	 out << std::fixed << std::setprecision(6) << result.finalSummary.mostFit[i]
             << '\n' << std::defaultfloat;
      }
      break;
      case Population::OperationMode::Minimize:
      for ( int i = 0 ; i < summaryCount ; i++ )
      {
	 population.printCandidate(population.populationTable[i].get()->genes(), out);
	 out << std::fixed << std::setprecision(6) << result.finalSummary.leastFit[i]
             << '\n' << std::defaultfloat;
      }
      break;
      default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
}

void Population::run(std::ostream& out, const RunReportOptions& options)
{
   RunResult result = execute(options.includeGenerationSummaries);
   RunReporter::write(out, *this, result, options);
}

void Population::run()
{
   const bool verbose = IsVerboseEnabled();
   run(std::cerr, RunReportOptions{verbose, verbose});
}
//
// Start Population with randomly generated population.
//
//
int Population::initializePopulation()
{
   populationTable.clear();
   populationTable.reserve(settings_.numberOfIndividuals);
   fitnessTable.assign(settings_.numberOfIndividuals, -1.0);
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
      if (fitnessTable[i] < 0.0)
      {
	 fitnessTable[i] = evaluateFitness(populationTable[i].get()->genes());
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
// Roulette Wheel parrent select.
// Select a parrent based upon the total Fitness of the
// population.
//
int Population::selectParent(const std::vector<double>& rouletteTable)
{
  double totalFitness = 0;
  double maximumFitness;
  maximumFitness = 0;
  int selected = -1;
   
  for ( int i = 0 ; i < settings_.numberOfIndividuals ; i++ )
    {
      totalFitness += rouletteTable[i];
      if (rouletteTable[i] > maximumFitness) maximumFitness = rouletteTable[i];
    }
   
  std::vector<double> invertedrouletteTable(settings_.numberOfIndividuals);
  double invertedTotalFitness = 0.00;

   for ( int i = 0 ; i < settings_.numberOfIndividuals ; i++ )
     {
       double invertedFitness = (maximumFitness - rouletteTable[i]) + 1.00;
       invertedrouletteTable[i] = invertedFitness;
       invertedTotalFitness += invertedFitness;
     }
   
   long int selectValue;
   switch (settings_.operation)
     {
     case Population::OperationMode::Maximize:
       if (totalFitness <= 0.0)
       {
          return randomIndex(settings_.numberOfIndividuals);
       }
       selectValue = randomBelow(static_cast<long int>(std::rint(totalFitness)));
       while (selectValue >= 0)
	 {
	   selectValue -= static_cast<long int>(std::rint(rouletteTable[++selected]));
	 }
       break;
     case Population::OperationMode::Minimize:
       if (invertedTotalFitness <= 0.0)
       {
          return randomIndex(settings_.numberOfIndividuals);
       }
       selectValue = randomBelow(static_cast<long int>(std::rint(invertedTotalFitness)));
       while (selectValue >= 0)
	 {
	   selectValue -= static_cast<long int>(std::rint(invertedrouletteTable[++selected]));
	 }
       break;
     default:
       throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
     }
   if (selected < 0 || selected >= settings_.numberOfIndividuals)
   {
      throw GAFatalException(__FILE__,__LINE__,"Roulette selection produced an invalid index");
   }
   return selected;
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
      while (static_cast<int>(replacementList.size()) < numberToReplace)
      {
	 const std::vector<double>& fitnessWeights = selectFitnessWeights();
	 const int fatherIndex = (settings_.parentSelection == Population::ParentSelectionMode::RouletteWheel)
	    ? selectParent(fitnessWeights)
	    : selectRandomParent();
	 const int motherIndex = (settings_.parentSelection == Population::ParentSelectionMode::RouletteWheel)
	    ? selectParent(fitnessWeights)
	    : selectRandomParent();
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
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported parent selection technique");
   }
   return replacementList;
}

std::unique_ptr<Chromosome> Population::createInitialChromosome()
{
   if (initializationStrategy_)
   {
      return initializationStrategy_->create(*this);
   }
   return createDefaultChromosome();
}

std::unique_ptr<Chromosome> Population::createDefaultChromosome()
{
   return std::make_unique<Chromosome>(settings_.geneticDiversity,
                                       settings_.variableLength == Population::VariableLengthMode::Variable,
                                       settings_.baseStates,
                                       &randomGenerator);
}

std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
Population::mateChromosomes(Chromosome& mother, Chromosome& father)
{
   if (matingStrategy_)
   {
      return matingStrategy_->mate(*this, mother, father);
   }
   return mateDefaultChromosomes(mother, father);
}

std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
Population::mateDefaultChromosomes(Chromosome& mother, Chromosome& father)
{
   return mother.mate(father,settings_.crossOverRate,Chromosome::CrossoverType::SinglePoint,&randomGenerator);
}

void Population::mutateChromosome(Chromosome& chromosome)
{
   if (mutationStrategy_)
   {
      mutationStrategy_->mutate(*this, chromosome);
      return;
   }
   mutateDefaultChromosome(chromosome);
}

void Population::mutateDefaultChromosome(Chromosome& chromosome)
{
   chromosome.mutate(settings_.bitMutationRate, &randomGenerator);
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
   for ( int i = 0 ; i < numToReplace; i++ )
   {
      const int index = replacementSlotIndex(i);
      if (settings_.reproduction == Population::ReproductionMode::DisallowDuplicates &&
          containsChromosome(*replacementList[i],populationTable,settings_.numberOfIndividuals))
      {
         continue;
      }

      fitnessTable[index] = -1;
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

   std::sort(order.begin(), order.end(),
             [this](int left, int right) { return fitnessTable[left] < fitnessTable[right]; });

   std::vector<double> sortedFitness(settings_.numberOfIndividuals);
   std::vector<std::unique_ptr<Chromosome> > sortedPopulation(settings_.numberOfIndividuals);
   for ( int i = 0 ; i < settings_.numberOfIndividuals ; i++ )
   {
      sortedFitness[i] = fitnessTable[order[i]];
      sortedPopulation[i] = std::move(populationTable[order[i]]);
   }

   fitnessTable.swap(sortedFitness);
   populationTable.swap(sortedPopulation);
}
