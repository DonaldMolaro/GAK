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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include <iostream>
using namespace std;

#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "except.hh"

extern "C"
{
   time_t time (time_t *t);
}

#define VERBOSE
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
{
   Operation                 = POperation;            // Minimize or Maximize function.
   numberofIndividuals       = PnumberofIndividuals;  // Number of Chromosomes at any one time.
   numberofTrials            = PnumberofTrials;       // Number to be generated in the whole optimization.
   GenecticDeversity         = PGenecticDeversity;    // The length of the Chromosome in bits.
   BitMutationRate           = PBitMutationRate;      // How often a single bit is mutated.
   CrossOverRate             = PCrossOverRate;        // How often the children are different from parents.
   Reproduction              = PReproductionTechnique;// How the population is strutrured.
   ParentSelection           = PParentSelection;      // How parrents are selected.
   Deletion                  = PDeletion;             // How many "unfit" members are done away with.
   Fitness                   = PFitness;              // How the fitness of the members is evalated.
   Variable                  = PVariable;             // If Variable length chromosomes are ok.
   baseStates                = PbaseStates;           // number of Permitted base states.
   populationTable = 0;
   fitnessTable = 0;
   windowedFitnessTable = 0;
   linearNormalizedfitnessTable = 0;
   //
   // Seed the random number generator with something reasonable.
   //
   time_t currenttime;
   time(&currenttime);
   srandom(currenttime);
   //
   //
   //
#ifdef VERBOSE
   fprintf(stderr,"Operation             :: %s\n", Operation == Minimize ? "Minimize" : "Maximize");
   fprintf(stderr,"Number of Individuals :: %d\n",numberofIndividuals);
   fprintf(stderr,"Number of Trails      :: %d\n",numberofTrials);
   fprintf(stderr,"Genectic Diversity    :: %d\n",GenecticDeversity);
   fprintf(stderr,"Mutation Rate         :: %5.4f\n",BitMutationRate);
   fprintf(stderr,"Cross Over Rate       :: %4.3f\n",CrossOverRate);
   fprintf(stderr,"Duplicate Reproduction:: %s\n", Reproduction == DuplicatesNotAllowed ? "NOT Ok." : " Ok.");
   fprintf(stderr,"Variable              :: %s\n", Variable ==  VariableLengthNotPermitted ? "NOT Ok." : " Ok.");
#endif
   populationInitialized = false;
};

Population::~Population()
{
   if (populationInitialized == true)
   {
      for ( int i = 0 ; i < numberofIndividuals ; i++ )
      {
	 delete populationTable[i];
      }
   }
   delete[] populationTable;
   delete[] fitnessTable;
   delete[] windowedFitnessTable;
   delete[] linearNormalizedfitnessTable;
};



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
   int numToReplace = 0;
   switch (Deletion)
   {
   case DeleteAll:
      numToReplace = numberofIndividuals;
      break;
   case DeleteAllButBest:
      numToReplace = numberofIndividuals - 1;
      break;
   case DeleteHalf:
      numToReplace =  numberofIndividuals / 2;
      break;
   case DeleteQuarter:
      numToReplace = numberofIndividuals / 4;
      break;
   case DeleteLast:
      numToReplace = 2;
      break;
   default:
      fprintf(stderr,"UNKNOWN DELETION TECHNIQUE...(dead)\n");
      exit ( -1 );
      break;
   }
   while ( numberBorn < numberofTrials )
   {
      Chromosome **replacementList;
      replacementList = breedPopulation(numToReplace);
      numberBorn +=     insertNewPopulation(replacementList,numToReplace);
      evaluatePopulation();
      fprintf(stderr,"Generation %d Number of Evaluations %d \n",numGen++,numberBorn);
#ifdef VERBOSE
      //
      //
      if (Operation == Maximize )
	 fprintf(stderr,"Best 5 Members are:\n");
      else
	 fprintf(stderr,"Worst 5 Members are:\n");
	 
      for ( int i = 0 ; i < 5 ; i++ )
      {
	 FitnessPrint(populationTable[(numberofIndividuals - 1) - i]->ChromosomeStr());
	 fprintf(stderr,"(F = %3.8f)\n",fitnessTable[(numberofIndividuals - 1) - i]);
      }
      
      if (Operation == Maximize )
	 fprintf(stderr,"Worst 5 Members are:\n");
      else
	 fprintf(stderr,"Best 5 Members are:\n");
      
      for ( int i = 0 ; i < 5 ; i++ )
      {
	 FitnessPrint(populationTable[i]->ChromosomeStr());
	 fprintf(stderr,"(F = %3.8f)\n",fitnessTable[i]);
      }
      //
      //
#endif
#define XGRAPH
#undef XGRAPH
#ifdef XGRAPH
      if (Operation == Maximize)
      {
	 for ( int i = 0 ; i < 1 ; i++ )
	 {
	    fprintf(stdout,"%d %d\n",numberBorn,fitnessTable[(numberofIndividuals - 1) - i]);
	 }
      }
      else
      {
	 for ( int i = 0 ; i < 1 ; i++ )
	 {
	    fprintf(stdout,"%d %d\n",numberBorn,fitnessTable[i]);
	 }
      }
      //
      //
#endif     
   }

      fprintf(stderr,"Generation %d Number of Evaluations %d \n",numGen++,numberBorn);
   switch(Operation)
   {
      case Maximize:
      for ( int i = 0 ; i < 5 ; i++ )
      {
	 FitnessPrint(populationTable[(numberofIndividuals - 1) - i]->ChromosomeStr());
	 fprintf(stderr,"%f\n",fitnessTable[(numberofIndividuals - 1) - i]);
      }
      break;
      case Minimize:
      for ( int i = 0 ; i < 5 ; i++ )
      {
	 FitnessPrint(populationTable[i]->ChromosomeStr());
	 fprintf(stderr,"%f\n",fitnessTable[i]);
      }
      break;
   }
}
//
// Start Population with randomly generated population.
//
//
int Population::initializePopulation()
{
   populationTable              = new Chromosome*[numberofIndividuals];
   fitnessTable                 = new double[numberofIndividuals];
   windowedFitnessTable         = new double[numberofIndividuals];
   linearNormalizedfitnessTable = new double[numberofIndividuals];
   
   for ( int i = 0 ; i < numberofIndividuals ; i++ )
   {
      populationTable[i] = new Chromosome(GenecticDeversity,Variable,baseStates);  
      fitnessTable[i] = -1.00;
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
	 fitnessTable[i] = FitnessFunction(populationTable[i]->ChromosomeStr());
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
   
  double *invertedrouletteTable = new double[numberofIndividuals];
  double invertedTotalFitness = 0.00;

   for ( int i = 0 ; i < numberofIndividuals ; i++ )
     {
       invertedrouletteTable[i] = (maximumFitness - rouletteTable[i]) + 1.00;
       assert(invertedrouletteTable[i] > 0.00);
       invertedTotalFitness   += invertedrouletteTable[i];
     }
   
   long int selectValue;
   if (totalFitness <= 0.0 || invertedTotalFitness <= 0.0)
     {
       delete[] invertedrouletteTable;
       throw GAFatalException(__FILE__,__LINE__,"Roulette selection requires positive total fitness");
     }
   switch (Operation)
     {
     case Maximize:
       selectValue = random() % (long int)rint(totalFitness);
       while (selectValue >= 0)
	 {
	   selectValue -= (long int)rint(rouletteTable[++(*selected)]);
	 }
       break;
     case Minimize:
       selectValue = random() % (long int)rint(invertedTotalFitness);
       while (selectValue >= 0)
	 {
	   selectValue -= (long int)rint(invertedrouletteTable[++(*selected)]);
	 }
       break;
     }
   assert(*selected != -1);
   assert(*selected >= 0);
   assert(*selected < numberofIndividuals);
   delete[] invertedrouletteTable;
   return populationTable[*selected];
}

double *Population::selectFitnessTable()
{
   switch (Fitness)
   {
   case FitnessIsEvaluation:
      return fitnessTable;
      break;
   case WindowedFitness:
      return windowedFitnessTable;
      break;
   case LinearNormalizedFitness:
      return linearNormalizedfitnessTable;
      break;
   default:
      fprintf(stderr,"UNKNOWN FITNESS TECHNIQUE .. dead\n");
      assert(0);
   }
   assert(0);
   return (double *)NULL;
}
//
// Generate a population of Children.
// Select a set of parrents and mate them to generate two
// Children.
//
Chromosome **Population::breedPopulation(int numberToReplace)
{
   Chromosome **replacementList = new Chromosome*[numberToReplace];
#ifdef VVERBOSE
	    fprintf(stderr,"Adding to replacement list.. duplicates %s permitted\n",Reproduction == DuplicatesNotAllowed ? "NOT" : "");
#endif 
  int numberGenerated = 0;
  switch (ParentSelection)
   {
   case RouletteWheel:
      while (numberGenerated < numberToReplace)
      {
	 int selected = -1;
	 Chromosome *father = selectParrent(&selected,selectFitnessTable());
	 Chromosome *mother = selectParrent(&selected,selectFitnessTable());
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
	    if (numberGenerated < numberToReplace)
	       replacementList[numberGenerated++] = son;
	    else
	       delete son;
	    //
	    // Keep the daugter if necessary to complete the
	    // replacement for the next generation.
	    //
	    if (numberGenerated < numberToReplace)
	       replacementList[numberGenerated++] = daughter;
	    else
	       delete daughter;
	    break;
	 case DuplicatesNotAllowed:
	    if (findMatch(son,replacementList,numberGenerated) == true)
	    {
#ifdef VVERBOSE
	       fprintf(stderr,"Deleted ::");
	       FitnessPrint(son->ChromosomeStr());
	       fprintf(stderr,"...\n");
#endif
	       delete son;
	    }
	    else
	    {
	       if (numberGenerated < numberToReplace)
		  replacementList[numberGenerated++] = son;
	       else
		  delete son;
	    }
	    //
	    // Keep the daugter if necessary to complete the
	    // replacement for the next generation.
	    //
	    if (findMatch(daughter,replacementList,numberGenerated) == true)
	    {
#ifdef VVERBOSE
	       fprintf(stderr,"Deleted ::");
	       FitnessPrint(daughter->ChromosomeStr());
	       fprintf(stderr,"...\n");
#endif
	       delete daughter;
	    }
	    else
	    {
	       if (numberGenerated < numberToReplace)
		  replacementList[numberGenerated++] = daughter;
	       else
		  delete daughter;
	    }
	    break;
	 }
      }
      break;
   default:
      fprintf(stderr,"Unsupported Reproduction Teechnique..dead.\n");
      assert(0);
      break;
   }
   return replacementList;
}
//
//
//
bool Population::findMatch(Chromosome *candidate,Chromosome **pop, int tableLength)
{
   for ( int i = 0 ; i < tableLength ; i++ )
   {
      if (candidate->compare(pop[i]) == true) 
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
int Population::insertNewPopulation(Chromosome **replacementList,int numToReplace)
{
   //
   // Assume population table is sorted by fitness.
   //
#ifdef VVERBOSE
   fprintf(stderr,"Replacing population.. duplicates %s permitted\n",Reproduction == DuplicatesNotAllowed ? "NOT" : "");
#endif 
   assert(numToReplace <= numberofIndividuals);
   int replaced = 0;
   switch (Reproduction)
   {
   case DuplicatesAllowed:
      switch (Operation)
      {
      case Maximize:
	 for ( int i = 0 ; i < numToReplace; i++ )
	 {
	    fitnessTable[i] = -1;
	    delete populationTable[i];
	    populationTable[i] = replacementList[i];
	    replaced++;
	 }
	 break;
      case Minimize:
	 for ( int i = 0 ; i < numToReplace; i++ )
	 {
	    fitnessTable          [(numberofIndividuals - 1) - i] = -1;
	    delete populationTable[(numberofIndividuals - 1) - i];
	    populationTable       [(numberofIndividuals - 1) - i] = replacementList[i];
	    replaced++;
	 }
	 break;
      default:
	 break;
      }
      break;
   case DuplicatesNotAllowed:
      switch (Operation)
      {
      case Maximize:
	 for ( int i = 0 ; i < numToReplace; i++ )
	 {
	    if (findMatch(replacementList[i],populationTable,numberofIndividuals) == true)
	    {
	       delete replacementList[i];
	    }
	    else
	    {
	       fitnessTable[i] = -1;
	       delete populationTable[i];
	       populationTable[i] = replacementList[i];
	       replaced++;
	    }
	 }
	 break;
      case Minimize:
	 for ( int i = 0 ; i < numToReplace; i++ )
	 {
	    if (findMatch(replacementList[i],populationTable,numberofIndividuals) == true)
	    {
	       delete replacementList[i];
	    }
	    else
	    {
	       fitnessTable          [(numberofIndividuals - 1) - i] = -1;
	       delete populationTable[(numberofIndividuals - 1) - i];
	       populationTable       [(numberofIndividuals - 1) - i] = replacementList[i];
	       replaced++;
	    }
	 }
	 break;
      default:
	 break;
      }
      break;
   }
   delete[] replacementList;
   return replaced;
}
//
// Sort population in decending order; most fit members will be in
// the last position.
//
void Population::sortPopulation()
{
   //
   // Uses shell's method of sorting.. good for partialiy ordered
   // data.
   // Outline for sort taken from K&R 1st ed. page 58.
   //
   for (int gap = numberofIndividuals / 2 ;  gap > 0 ; gap /= 2 )
   {
      for ( int i = gap ; i < numberofIndividuals; i++ )
      {
	 for ( int j = i - gap ; j >= 0 && fitnessTable[j]>fitnessTable[j+gap]; j -=gap)
	 {
	    // Exchange the fitness value and the Chromsome for a particular
	    // position.
	    //
	    double tmp          = fitnessTable[j];
	    fitnessTable[j]     = fitnessTable[j+gap];
	    fitnessTable[j+gap] = tmp;
	    //
	    // Exchange the Chromosome string..
	    //
	    Chromosome *ctmp       = populationTable[j];
	    populationTable[j]     = populationTable[j+gap];
	    populationTable[j+gap] = ctmp;
	 }
      }
   }
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









