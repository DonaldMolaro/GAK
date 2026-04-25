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
#include <assert.h>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
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
   return std::getenv("GAK_VERBOSE") != NULL;
}
}

Population::Options Population::Configuration::toOptions() const
{
   Options options;
   options.operation = operation;
   options.numberOfIndividuals = numberOfIndividuals;
   options.numberOfTrials = numberOfTrials;
   options.geneticDiversity = geneticDiversity;
   options.bitMutationRate = bitMutationRate;
   options.crossOverRate = crossOverRate;
   options.reproduction = reproduction;
   options.parentSelection = parentSelection;
   options.deletion = deletion;
   options.fitness = fitness;
   options.variableLength = variableLength;
   options.baseStates = baseStates;
   options.useFixedRandomSeed = useFixedRandomSeed;
   options.randomSeed = randomSeed;
   return options;
}

Population::Configuration Population::Options::toConfiguration() const
{
   Configuration configuration;
   configuration.operation = operation;
   configuration.numberOfIndividuals = numberOfIndividuals;
   configuration.numberOfTrials = numberOfTrials;
   configuration.geneticDiversity = geneticDiversity;
   configuration.bitMutationRate = bitMutationRate;
   configuration.crossOverRate = crossOverRate;
   configuration.reproduction = reproduction;
   configuration.parentSelection = parentSelection;
   configuration.deletion = deletion;
   configuration.fitness = fitness;
   configuration.variableLength = variableLength;
   configuration.baseStates = baseStates;
   configuration.useFixedRandomSeed = useFixedRandomSeed;
   configuration.randomSeed = randomSeed;
   return configuration;
}
//
// Population module;
//         This is the heart of the genectic algorithim system. It intializes and
//         controls the Chromosome data strutures. Evaluates chromosomes with the
//         fitness function, mates members of the population together and replaces unfit
//         members with new children.
//


Population::Population(const Configuration& configuration)
   : populationInitialized(false),
     config_(configuration),
     activeRandomSeed_(0)
{
   if (config_.useFixedRandomSeed)
   {
      setRandomSeed(config_.randomSeed);
   }
   else
   {
      std::time_t currenttime;
      std::time(&currenttime);
      setRandomSeed(static_cast<unsigned int>(currenttime));
   }
}

Population::Population(const Options& options)
   : Population(options.toConfiguration())
{
}

Population::~Population() = default;

void Population::setRandomSeed(unsigned int seed)
{
   activeRandomSeed_ = seed;
   randomGenerator.seed(seed);
   Chromosome::seedRandom(seed);
}

int Population::replacementCount() const
{
   switch (config_.deletion)
   {
   case Population::DeletionMode::DeleteAll:
      return config_.numberOfIndividuals;
   case Population::DeletionMode::DeleteAllButBest:
      return config_.numberOfIndividuals - 1;
   case Population::DeletionMode::DeleteHalf:
      return  config_.numberOfIndividuals / 2;
   case Population::DeletionMode::DeleteQuarter:
      return config_.numberOfIndividuals / 4;
   case Population::DeletionMode::DeleteLast:
      return 2;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported deletion technique");
   }
}

int Population::replacementSlotIndex(int offset) const
{
   switch (config_.operation)
   {
   case Population::OperationMode::Maximize:
      return offset;
   case Population::OperationMode::Minimize:
      return (config_.numberOfIndividuals - 1) - offset;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
}

Population::PopulationSummary Population::buildPopulationSummary() const
{
   const int summaryCount = std::min(config_.numberOfIndividuals, kSummaryCount);
   PopulationSummary summary;
   summary.mostFit.reserve(summaryCount);
   summary.leastFit.reserve(summaryCount);

   for ( int i = 0 ; i < summaryCount ; i++ )
   {
      summary.mostFit.push_back(fitnessTable[(config_.numberOfIndividuals - 1) - i]);
      summary.leastFit.push_back(fitnessTable[i]);
   }
   return summary;
}

void Population::printConfigurationSummary() const
{
   fprintf(stderr,"Operation             :: %s\n", config_.operation == Population::OperationMode::Minimize ? "Minimize" : "Maximize");
   fprintf(stderr,"Number of Individuals :: %d\n",config_.numberOfIndividuals);
   fprintf(stderr,"Number of Trails      :: %d\n",config_.numberOfTrials);
   fprintf(stderr,"Genectic Diversity    :: %d\n",config_.geneticDiversity);
   fprintf(stderr,"Mutation Rate         :: %5.4f\n",config_.bitMutationRate);
   fprintf(stderr,"Cross Over Rate       :: %4.3f\n",config_.crossOverRate);
   fprintf(stderr,"Duplicate Reproduction:: %s\n", config_.reproduction == Population::ReproductionMode::DisallowDuplicates ? "NOT Ok." : " Ok.");
   fprintf(stderr,"Variable              :: %s\n", config_.variableLength == Population::VariableLengthMode::Fixed ? "NOT Ok." : " Ok.");
   fprintf(stderr,"Random Seed           :: %u%s\n",
           activeRandomSeed_,
           config_.useFixedRandomSeed ? " (configured)" : " (generated)");
}

void Population::printPopulationSummary(const PopulationSummary& summary)
{
   const int summaryCount = static_cast<int>(summary.mostFit.size());
   if (summaryCount <= 0)
   {
      return;
   }

   if (config_.operation == Population::OperationMode::Maximize)
      fprintf(stderr,"Best %d Members are:\n",summaryCount);
   else
      fprintf(stderr,"Worst %d Members are:\n",summaryCount);

   for ( int i = 0 ; i < summaryCount ; i++ )
   {
      FitnessPrint(populationTable[(config_.numberOfIndividuals - 1) - i].get()->chromosomeString());
      fprintf(stderr,"(F = %3.8f)\n",summary.mostFit[i]);
   }

   if (config_.operation == Population::OperationMode::Maximize)
      fprintf(stderr,"Worst %d Members are:\n",summaryCount);
   else
      fprintf(stderr,"Best %d Members are:\n",summaryCount);

   for ( int i = 0 ; i < summaryCount ; i++ )
   {
      FitnessPrint(populationTable[i].get()->chromosomeString());
      fprintf(stderr,"(F = %3.8f)\n",summary.leastFit[i]);
   }
}

void Population::printGenerationProgress(const GenerationReport& report, bool printSummary)
{
   fprintf(stderr,"Generation %d Number of Evaluations %d \n",
           report.generation,
           report.evaluations);
   if (printSummary)
   {
      printPopulationSummary(report.summary);
   }
}

void Population::printFinalSummary(const RunResult& result)
{
   const int summaryCount = static_cast<int>(result.finalSummary.mostFit.size());
   switch(config_.operation)
   {
      case Population::OperationMode::Maximize:
      for ( int i = 0 ; i < summaryCount ; i++ )
      {
	 FitnessPrint(populationTable[(config_.numberOfIndividuals - 1) - i].get()->chromosomeString());
	 fprintf(stderr,"%f\n",result.finalSummary.mostFit[i]);
      }
      break;
      case Population::OperationMode::Minimize:
      for ( int i = 0 ; i < summaryCount ; i++ )
      {
	 FitnessPrint(populationTable[i].get()->chromosomeString());
	 fprintf(stderr,"%f\n",result.finalSummary.leastFit[i]);
      }
      break;
      default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
}

Chromosome *Population::selectRandomParent(int *selected)
{
   *selected = randomIndex(config_.numberOfIndividuals);
   return populationTable[*selected].get();
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
                                   Chromosome *candidate,
                                   int& numberGenerated,
                                   int numberToReplace,
                                   bool allowDuplicates)
{
   if (!allowDuplicates && containsChromosome(candidate,replacementList,numberGenerated))
   {
      delete candidate;
      return false;
   }

   if (numberGenerated < numberToReplace)
   {
      replacementList.push_back(std::unique_ptr<Chromosome>(candidate));
      numberGenerated++;
      return true;
   }

   delete candidate;
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
   result.usedConfiguredSeed = config_.useFixedRandomSeed;
   int numberBorn = initializePopulation();
   evaluatePopulation();
   int numGen = 0;
   const int numToReplace = replacementCount();
   while ( numberBorn < config_.numberOfTrials )
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

void Population::reportRun(const RunResult& result, bool printGenerationSummaries)
{
   if (printGenerationSummaries)
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

   printFinalSummary(result);
}

void Population::run()
{
   const bool verbose = IsVerboseEnabled();
   if (verbose)
   {
      printConfigurationSummary();
   }
   RunResult result = execute(verbose);
   reportRun(result, verbose);
}
//
// Start Population with randomly generated population.
//
//
int Population::initializePopulation()
{
   populationTable.clear();
   populationTable.reserve(config_.numberOfIndividuals);
   fitnessTable.assign(config_.numberOfIndividuals, -1.0);
   windowedFitnessTable.assign(config_.numberOfIndividuals, 0.0);
   linearNormalizedfitnessTable.assign(config_.numberOfIndividuals, 0.0);
   
   for ( int i = 0 ; i < config_.numberOfIndividuals ; i++ )
   {
      populationTable.push_back(createInitialChromosome());
   }
   populationInitialized = true;
   return config_.numberOfIndividuals;
}
//
// Evaluate all unevalueated members of the population pool.
// fitness must be a positive double.
//
void Population::evaluatePopulation()
{
   for ( int i = 0 ; i < config_.numberOfIndividuals ; i++ )
   {
      if (fitnessTable[i] < 0.0)
      {
	 fitnessTable[i] = FitnessFunction(populationTable[i].get()->chromosomeString());
      }
   }
   sortPopulation();
   //
   // The population is now sorted in accending order, 
   // members with the highest fitness are now in the last position of
   // table...
   //
   for ( int i = 0 ; i < config_.numberOfIndividuals ; i++ )
   {
      linearNormalizedfitnessTable[i] = i + 1.00;
      windowedFitnessTable[i] = (config_.operation == Population::OperationMode::Maximize)
	 ? (fitnessTable[i] - fitnessTable[0] + 1.00)
	 : (fitnessTable[config_.numberOfIndividuals - 1] - fitnessTable[i] + 1.00);
   }
   return;
}
//
// Roulette Wheel parrent select.
// Select a parrent based upon the total Fitness of the
// population.
//
Chromosome *Population::selectParent(int *selected,double *rouletteTable)
{
  double totalFitness = 0;
  double maximumFitness;
  maximumFitness = 0;
  *selected = -1;
   
  for ( int i = 0 ; i < config_.numberOfIndividuals ; i++ )
    {
      totalFitness += rouletteTable[i];
      if (rouletteTable[i] > maximumFitness) maximumFitness = rouletteTable[i];
    }
   
  std::vector<double> invertedrouletteTable(config_.numberOfIndividuals);
  double invertedTotalFitness = 0.00;

   for ( int i = 0 ; i < config_.numberOfIndividuals ; i++ )
     {
       double invertedFitness = (maximumFitness - rouletteTable[i]) + 1.00;
       invertedrouletteTable[i] = invertedFitness;
       invertedTotalFitness += invertedFitness;
     }
   
   long int selectValue;
   switch (config_.operation)
     {
     case Population::OperationMode::Maximize:
       if (totalFitness <= 0.0)
       {
          *selected = randomIndex(config_.numberOfIndividuals);
          return populationTable[*selected].get();
       }
       selectValue = randomBelow(static_cast<long int>(std::rint(totalFitness)));
       while (selectValue >= 0)
	 {
	   selectValue -= static_cast<long int>(std::rint(rouletteTable[++(*selected)]));
	 }
       break;
     case Population::OperationMode::Minimize:
       if (invertedTotalFitness <= 0.0)
       {
          *selected = randomIndex(config_.numberOfIndividuals);
          return populationTable[*selected].get();
       }
       selectValue = randomBelow(static_cast<long int>(std::rint(invertedTotalFitness)));
       while (selectValue >= 0)
	 {
	   selectValue -= static_cast<long int>(std::rint(invertedrouletteTable[++(*selected)]));
	 }
       break;
     default:
       throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
     }
   if (*selected < 0 || *selected >= config_.numberOfIndividuals)
   {
      throw GAFatalException(__FILE__,__LINE__,"Roulette selection produced an invalid index");
   }
   return populationTable[*selected].get();
}

double *Population::selectFitnessWeights()
{
   switch (config_.fitness)
   {
   case Population::FitnessMode::Evaluation:
      return fitnessTable.data();
   case Population::FitnessMode::Windowed:
      return windowedFitnessTable.data();
   case Population::FitnessMode::LinearNormalized:
      return linearNormalizedfitnessTable.data();
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
#ifdef VVERBOSE
	    fprintf(stderr,"Adding to replacement list.. duplicates %s permitted\n",config_.reproduction == Population::ReproductionMode::DisallowDuplicates ? "NOT" : "");
#endif 
  int numberGenerated = 0;
  switch (config_.parentSelection)
   {
   case Population::ParentSelectionMode::RouletteWheel:
   case Population::ParentSelectionMode::Random:
      while (numberGenerated < numberToReplace)
      {
	 int selected = -1;
	 Chromosome *father = (config_.parentSelection == Population::ParentSelectionMode::RouletteWheel)
	    ? selectParent(&selected,selectFitnessWeights())
	    : selectRandomParent(&selected);
	 Chromosome *mother = (config_.parentSelection == Population::ParentSelectionMode::RouletteWheel)
	    ? selectParent(&selected,selectFitnessWeights())
	    : selectRandomParent(&selected);
         std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> > children =
            mateChromosomes(mother, father);
	 //
         mutateChromosome(children.first.get());
         mutateChromosome(children.second.get());
	 //
	 // if this child is required for the next generation
	 // keep it. otherwise abort it.
	 // Practically son's will never be aborted just daughters.
	 //
	 switch (config_.reproduction)
	 {
	 case Population::ReproductionMode::AllowDuplicates:
	    appendReplacement(replacementList,children.first.release(),numberGenerated,numberToReplace,true);
	    //
	    // Keep the daugter if necessary to complete the
	    // replacement for the next generation.
	    //
	    appendReplacement(replacementList,children.second.release(),numberGenerated,numberToReplace,true);
	    break;
	 case Population::ReproductionMode::DisallowDuplicates:
	    appendReplacement(replacementList,children.first.release(),numberGenerated,numberToReplace,false);
	    //
	    // Keep the daugter if necessary to complete the
	    // replacement for the next generation.
	    //
	    appendReplacement(replacementList,children.second.release(),numberGenerated,numberToReplace,false);
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
   return std::make_unique<Chromosome>(config_.geneticDiversity,
                                       static_cast<unsigned int>(config_.variableLength == Population::VariableLengthMode::Variable),
                                       config_.baseStates);
}

std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
Population::mateChromosomes(Chromosome *mother, Chromosome *father)
{
   return mother->mate(*father,config_.crossOverRate,Chromosome::SinglePoint);
}

void Population::mutateChromosome(Chromosome *chromosome)
{
   chromosome->SingleBitMutate(config_.bitMutationRate);
}

bool Population::containsChromosome(const Chromosome *candidate,
                                    const std::vector<std::unique_ptr<Chromosome> >& pop,
                                    int populationLength) const
{
   for ( int i = 0 ; i < populationLength ; i++ )
   {
      if (candidate->compare(pop[i].get()) == true)
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
   //
   // Assume population table is sorted by fitness.
   //
#ifdef VVERBOSE
   fprintf(stderr,"Replacing population.. duplicates %s permitted\n",config_.reproduction == Population::ReproductionMode::DisallowDuplicates ? "NOT" : "");
#endif 
   if (numToReplace > config_.numberOfIndividuals)
   {
      throw GAFatalException(__FILE__,__LINE__,"Cannot replace more individuals than exist in the population");
   }
   int replaced = 0;
   for ( int i = 0 ; i < numToReplace; i++ )
   {
      const int index = replacementSlotIndex(i);
      if (config_.reproduction == Population::ReproductionMode::DisallowDuplicates &&
          containsChromosome(replacementList[i].get(),populationTable,config_.numberOfIndividuals))
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
   std::vector<int> order(config_.numberOfIndividuals);
   for ( int i = 0 ; i < config_.numberOfIndividuals ; i++ )
   {
      order[i] = i;
   }

   std::sort(order.begin(), order.end(),
             [this](int left, int right) { return fitnessTable[left] < fitnessTable[right]; });

   std::vector<double> sortedFitness(config_.numberOfIndividuals);
   std::vector<std::unique_ptr<Chromosome> > sortedPopulation(config_.numberOfIndividuals);
   for ( int i = 0 ; i < config_.numberOfIndividuals ; i++ )
   {
      sortedFitness[i] = fitnessTable[order[i]];
      sortedPopulation[i] = std::move(populationTable[order[i]]);
   }

   fitnessTable.swap(sortedFitness);
   populationTable.swap(sortedPopulation);
}
//
//
//
int Population::decode(const BaseString& b,int start,int end) const
{
   int res = 0;
   for ( int i = start ; i < end ; i++ )
   {
      res <<= 1;
      if ( b.test(i) )
      {
	 res += 1;   
      }
      else
      {
	 res += 0;
      }
      
   }
   return res;
}
