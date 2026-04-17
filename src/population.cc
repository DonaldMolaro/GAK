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
//
// Population module;
//         This is the heart of the genectic algorithim system. It intializes and
//         controls the Chromosome data strutures. Evaluates chromosomes with the
//         fitness function, mates members of the population together and replaces unfit
//         members with new children.
//


Population::Population(OperationTechnique POperation,
		       int PnumberofIndividuals,int PnumberofTrials,
		       int PGenecticDeversity,double PBitMutationRate,
		       double PCrossOverRate,
		       ReproductionTechniques PReproductionTechnique,
		       ParrentSelectionTechnique PParentSelection,
		       DeletetionTechnique PDeletion,
		       FitnessTechnique PFitness,
		       VariableLength PVariable,
		       int PbaseStates)
   : populationInitialized(false),
     numberofIndividuals(PnumberofIndividuals),
     numberofTrials(PnumberofTrials),
     GenecticDeversity(PGenecticDeversity),
     BitMutationRate(PBitMutationRate),
     CrossOverRate(PCrossOverRate),
     Operation(POperation),
     Reproduction(PReproductionTechnique),
     ParentSelection(PParentSelection),
     Deletion(PDeletion),
     Fitness(PFitness),
     Variable(PVariable),
     baseStates(PbaseStates)
{
   //
   // Seed the random number generator with something reasonable.
   //
   std::time_t currenttime;
   std::time(&currenttime);
   randomGenerator.seed(static_cast<unsigned int>(currenttime));
   //
   //
   //
   if (IsVerboseEnabled())
   {
   fprintf(stderr,"Operation             :: %s\n", Operation == Minimize ? "Minimize" : "Maximize");
   fprintf(stderr,"Number of Individuals :: %d\n",numberofIndividuals);
   fprintf(stderr,"Number of Trails      :: %d\n",numberofTrials);
   fprintf(stderr,"Genectic Diversity    :: %d\n",GenecticDeversity);
   fprintf(stderr,"Mutation Rate         :: %5.4f\n",BitMutationRate);
   fprintf(stderr,"Cross Over Rate       :: %4.3f\n",CrossOverRate);
   fprintf(stderr,"Duplicate Reproduction:: %s\n", Reproduction == DuplicatesNotAllowed ? "NOT Ok." : " Ok.");
   fprintf(stderr,"Variable              :: %s\n", Variable ==  VariableLengthNotPermitted ? "NOT Ok." : " Ok.");
   }
}

int Population::numToReplaceForDeletion() const
{
   switch (Deletion)
   {
   case DeleteAll:
      return numberofIndividuals;
   case DeleteAllButBest:
      return numberofIndividuals - 1;
   case DeleteHalf:
      return  numberofIndividuals / 2;
   case DeleteQuarter:
      return numberofIndividuals / 4;
   case DeleteLast:
      return 2;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported deletion technique");
   }
}

int Population::replacementIndex(int offset) const
{
   switch (Operation)
   {
   case Maximize:
      return offset;
   case Minimize:
      return (numberofIndividuals - 1) - offset;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
}

void Population::printPopulationSummary()
{
   const int summaryCount = std::min(numberofIndividuals, kSummaryCount);
   if (summaryCount <= 0)
   {
      return;
   }

   if (Operation == Maximize)
      fprintf(stderr,"Best %d Members are:\n",summaryCount);
   else
      fprintf(stderr,"Worst %d Members are:\n",summaryCount);

   for ( int i = 0 ; i < summaryCount ; i++ )
   {
      FitnessPrint(populationTable[(numberofIndividuals - 1) - i].get()->ChromosomeStr());
      fprintf(stderr,"(F = %3.8f)\n",fitnessTable[(numberofIndividuals - 1) - i]);
   }

   if (Operation == Maximize)
      fprintf(stderr,"Worst %d Members are:\n",summaryCount);
   else
      fprintf(stderr,"Best %d Members are:\n",summaryCount);

   for ( int i = 0 ; i < summaryCount ; i++ )
   {
      FitnessPrint(populationTable[i].get()->ChromosomeStr());
      fprintf(stderr,"(F = %3.8f)\n",fitnessTable[i]);
   }
}

Chromosome *Population::selectRandomParent(int *selected)
{
   *selected = randomIndex(numberofIndividuals);
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
   if (!allowDuplicates && findMatch(candidate,replacementList,numberGenerated) == true)
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
void Population::run()
{
   int numberBorn = initializePopulation();
   evaluatePopulation();
   int numGen = 0;
   const int numToReplace = numToReplaceForDeletion();
   while ( numberBorn < numberofTrials )
   {
      std::vector<std::unique_ptr<Chromosome> > replacementList = breedPopulation(numToReplace);
      numberBorn += insertNewPopulation(std::move(replacementList),numToReplace);
      evaluatePopulation();
      fprintf(stderr,"Generation %d Number of Evaluations %d \n",numGen++,numberBorn);
      if (IsVerboseEnabled())
      {
         printPopulationSummary();
      }
   }

   fprintf(stderr,"Generation %d Number of Evaluations %d \n",numGen++,numberBorn);
   switch(Operation)
   {
      case Maximize: {
      const int summaryCount = std::min(numberofIndividuals, kSummaryCount);
      for ( int i = 0 ; i < summaryCount ; i++ )
      {
	 FitnessPrint(populationTable[(numberofIndividuals - 1) - i].get()->ChromosomeStr());
	 fprintf(stderr,"%f\n",fitnessTable[(numberofIndividuals - 1) - i]);
      }
      break;
      }
      case Minimize: {
      const int summaryCount = std::min(numberofIndividuals, kSummaryCount);
      for ( int i = 0 ; i < summaryCount ; i++ )
      {
	 FitnessPrint(populationTable[i].get()->ChromosomeStr());
	 fprintf(stderr,"%f\n",fitnessTable[i]);
      }
      break;
      }
      default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
}
//
// Start Population with randomly generated population.
//
//
int Population::initializePopulation()
{
   populationTable.clear();
   populationTable.reserve(numberofIndividuals);
   fitnessTable.assign(numberofIndividuals, -1.0);
   windowedFitnessTable.assign(numberofIndividuals, 0.0);
   linearNormalizedfitnessTable.assign(numberofIndividuals, 0.0);
   
   for ( int i = 0 ; i < numberofIndividuals ; i++ )
   {
      populationTable.push_back(std::make_unique<Chromosome>(GenecticDeversity,Variable,baseStates));
   }
   populationInitialized = true;
   return numberofIndividuals;
}
//
// Evaluate all unevalueated members of the population pool.
// fitness must be a positive double.
//
void Population::evaluatePopulation()
{
   for ( int i = 0 ; i < numberofIndividuals ; i++ )
   {
      if (fitnessTable[i] < 0.0)
      {
	 fitnessTable[i] = FitnessFunction(populationTable[i].get()->ChromosomeStr());
      }
   }
   sortPopulation();
   //
   // The population is now sorted in accending order, 
   // members with the highest fitness are now in the last position of
   // table...
   //
   for ( int i = 0 ; i < numberofIndividuals ; i++ )
   {
      linearNormalizedfitnessTable[i] = i + 1.00;
      windowedFitnessTable[i] = (Operation == Maximize)
	 ? (fitnessTable[i] - fitnessTable[0] + 1.00)
	 : (fitnessTable[numberofIndividuals - 1] - fitnessTable[i] + 1.00);
   }
   return;
}
//
// Roulette Wheel parrent select.
// Select a parrent based upon the total Fitness of the
// population.
//
Chromosome *Population::selectParrent(int *selected,double *rouletteTable)
{
  double totalFitness = 0;
  double maximumFitness;
  maximumFitness = 0;
  *selected = -1;
   
  for ( int i = 0 ; i < numberofIndividuals ; i++ )
    {
      totalFitness += rouletteTable[i];
      if (rouletteTable[i] > maximumFitness) maximumFitness = rouletteTable[i];
    }
   
  std::vector<double> invertedrouletteTable(numberofIndividuals);
  double invertedTotalFitness = 0.00;

   for ( int i = 0 ; i < numberofIndividuals ; i++ )
     {
       double invertedFitness = (maximumFitness - rouletteTable[i]) + 1.00;
       invertedrouletteTable[i] = invertedFitness;
       invertedTotalFitness += invertedFitness;
     }
   
   long int selectValue;
   switch (Operation)
     {
     case Maximize:
       if (totalFitness <= 0.0)
       {
          *selected = randomIndex(numberofIndividuals);
          return populationTable[*selected].get();
       }
       selectValue = randomBelow(static_cast<long int>(std::rint(totalFitness)));
       while (selectValue >= 0)
	 {
	   selectValue -= static_cast<long int>(std::rint(rouletteTable[++(*selected)]));
	 }
       break;
     case Minimize:
       if (invertedTotalFitness <= 0.0)
       {
          *selected = randomIndex(numberofIndividuals);
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
   if (*selected < 0 || *selected >= numberofIndividuals)
   {
      throw GAFatalException(__FILE__,__LINE__,"Roulette selection produced an invalid index");
   }
   return populationTable[*selected].get();
}

double *Population::selectFitnessTable()
{
   switch (Fitness)
   {
   case FitnessIsEvaluation:
      return fitnessTable.data();
   case WindowedFitness:
      return windowedFitnessTable.data();
   case LinearNormalizedFitness:
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
	    fprintf(stderr,"Adding to replacement list.. duplicates %s permitted\n",Reproduction == DuplicatesNotAllowed ? "NOT" : "");
#endif 
  int numberGenerated = 0;
  switch (ParentSelection)
   {
   case RouletteWheel:
   case Random:
      while (numberGenerated < numberToReplace)
      {
	 int selected = -1;
	 Chromosome *father = (ParentSelection == RouletteWheel)
	    ? selectParrent(&selected,selectFitnessTable())
	    : selectRandomParent(&selected);
	 Chromosome *mother = (ParentSelection == RouletteWheel)
	    ? selectParrent(&selected,selectFitnessTable())
	    : selectRandomParent(&selected);
	 Chromosome *son;
	 Chromosome *daughter;
	 mother->Mate(father,&son,&daughter,CrossOverRate,Chromosome::SinglePoint);
	 //
	 son->SingleBitMutate(BitMutationRate);
	 daughter->SingleBitMutate(BitMutationRate);
	 //
	 // if this child is required for the next generation
	 // keep it. otherwise abort it.
	 // Practically son's will never be aborted just daughters.
	 //
	 switch (Reproduction)
	 {
	 case DuplicatesAllowed:
	    appendReplacement(replacementList,son,numberGenerated,numberToReplace,true);
	    //
	    // Keep the daugter if necessary to complete the
	    // replacement for the next generation.
	    //
	    appendReplacement(replacementList,daughter,numberGenerated,numberToReplace,true);
	    break;
	 case DuplicatesNotAllowed:
	    appendReplacement(replacementList,son,numberGenerated,numberToReplace,false);
	    //
	    // Keep the daugter if necessary to complete the
	    // replacement for the next generation.
	    //
	    appendReplacement(replacementList,daughter,numberGenerated,numberToReplace,false);
	    break;
	 }
      }
      break;
   default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported parent selection technique");
   }
   return replacementList;
}
bool Population::findMatch(const Chromosome *candidate,
			   const std::vector<std::unique_ptr<Chromosome> >& pop,
			   int tableLength) const
{
   for ( int i = 0 ; i < tableLength ; i++ )
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
   fprintf(stderr,"Replacing population.. duplicates %s permitted\n",Reproduction == DuplicatesNotAllowed ? "NOT" : "");
#endif 
   if (numToReplace > numberofIndividuals)
   {
      throw GAFatalException(__FILE__,__LINE__,"Cannot replace more individuals than exist in the population");
   }
   int replaced = 0;
   for ( int i = 0 ; i < numToReplace; i++ )
   {
      const int index = replacementIndex(i);
      if (Reproduction == DuplicatesNotAllowed &&
          findMatch(replacementList[i].get(),populationTable,numberofIndividuals) == true)
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
   std::vector<int> order(numberofIndividuals);
   for ( int i = 0 ; i < numberofIndividuals ; i++ )
   {
      order[i] = i;
   }

   std::sort(order.begin(), order.end(),
             [this](int left, int right) { return fitnessTable[left] < fitnessTable[right]; });

   std::vector<double> sortedFitness(numberofIndividuals);
   std::vector<std::unique_ptr<Chromosome> > sortedPopulation(numberofIndividuals);
   for ( int i = 0 ; i < numberofIndividuals ; i++ )
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
int Population::decode(BaseString *b,int start,int end)
{
   int res = 0;
   for ( int i = start ; i < end ; i++ )
   {
      res <<= 1;
      if ( b->test(i) )
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
