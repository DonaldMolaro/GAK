#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <genetic.hh>
#include "dome.hh"
#include "alpha.hh"
#include "spell.hh"
#include "f6.hh"
#include "ts.hh"

void usage()
{
  fprintf(stderr,"Usage\n");
  fprintf(stderr,"\tGA D  - Dome Function (binary fixed length)\n");
  fprintf(stderr,"\tGA F6 - F6 Function   (binary fixed length)\n");
  fprintf(stderr,"\tGA A  - Alphabet      (high order, fixed length)\n");
  fprintf(stderr,"\tGA S  - Spell         (high order, variable length)\n");
  fprintf(stderr,"\tGA T  - Traveling Salesman (high order, fixed length)\n");
  exit(0);
}

int main(int argc,char *argv[]);
int main(int argc,char *argv[])
{
   //
   // An example of algabraic optimization using
   // a generic algorithim.
   //
  if (argc != 2) usage();
  switch (*argv[1])
    {
    case 'd':
    case 'D':
      {
	Dome dome(Population::Minimize,                     // Operation
		  100,                                      // Population Size.
		  4000,                                     // Number in civilzation 
		  32,                                       // Genentic Length
		  0.008,                                    // Mutation rate.
		  0.65,                                     // Cross overate.
		  Population::DuplicatesNotAllowed,         // Duplication permitted.
		  Population::RouletteWheel,                // Parent Selection
		  Population::DeleteHalf,                   // Deletion technique.
		  Population::LinearNormalizedFitness,      // Fitness evaluation.
		  Population::VariableLengthNotPermitted,   // If Variable Length Chromosomes are Ok.
		  2                                         // Number of permitted states of each base.
		  );     
	dome.run();
      }
      break;
    case '6':
      {
	F6 F6(Population::Minimize,                     // Operation
	      100,                                      // Population Size.
	      5000,                                     // Number in civilzation 
	      44,                                       // Genentic Length
	      0.008,                                    // Mutation rate.
	      0.65,                                     // Cross overate.
	      Population::DuplicatesNotAllowed,         // Duplication permitted.
	      Population::RouletteWheel,                // Parent Selection
	      Population::DeleteHalf,                   // Deletion technique.
	      Population::LinearNormalizedFitness,      // Fitness evaluation.
	      Population::VariableLengthNotPermitted,   // If Variable Length Chromosomes are Ok.
	      2                                         // Number of permitted states of each base.
	      );     
	F6.run();
      }
      break;

    case 'A':
    case 'a':
      {
	//
	// An example of symbolic optimization
	// using a genetic algorithim.
	//
	Alpha alpha(Population::Maximize,                   // Operation
		    1000,                                      // Population Size.
		    160000,                                    // Number in civilzation 
		    26,                                       // Genentic Length
		    0.008,                                    // Mutation rate.
		    0.65,                                     // Cross overate.
		    Population::DuplicatesNotAllowed,         // Duplication permitted.
		    Population::RouletteWheel,                // Parent Selection
		    Population::DeleteHalf,                   // Deletion technique.
		    Population::LinearNormalizedFitness,      // Fitness evaluation.
		    Population::VariableLengthNotPermitted,   // If Variable Length Chromosomes are Ok.
		    26                                        // Number of permitted states of each base.
		    );     
	//alpha.RandomAlgorithim();
	alpha.run();
      }
      break;
    case 'S':
    case 's':
      {
      //
      // An example of symbolic optimization
      // using variable length chromosomes.
      //
	Spell spell(Population::Maximize,                   // Operation
		    100,                                      // Population Size.
		    8000,                                     // Number in civilzation 
		    7,                                        // Genentic Length
		    0.008,                                    // Mutation rate.
		    0.65,                                     // Cross overate.
		    Population::DuplicatesNotAllowed,         // Duplication permitted.
		    Population::RouletteWheel,                // Parent Selection
		    Population::DeleteHalf,                   // Deletion technique.
		    Population::LinearNormalizedFitness,      // Fitness evaluation.
		    Population::VariableLengthPermitted,      // If Variable Length Chromosomes are Ok.
		    26                                        // Number of permitted states of each base.
		    );     
	spell.run();
      }
      break;
    case 'T':
    case 't':
      /*
       * Traveling salesman problem solved with a high order GA.
       * Donald Molaro - Copyright 1997.
       *
       * Construts the population object, and set's it running.
       *
       */
      {
	//
	// An example of symbolic optimization
	// using a genetic algorithim.
	//
	TravelingSalesman 
	  travelingSalesman(Population::Minimize, // What to do with the fitness function.
			    500,                                            // Population Size.
			    200000,                                         // Number in civilzation 
			    26,                                             // Genentic Length
			    0.008,                                          // Mutation rate.
			    0.65,                                           // Cross overrate.
			    Population::DuplicatesNotAllowed,               // Duplication not permitted.
			    Population::RouletteWheel,                      // Parent Selection technique.
			    Population::DeleteHalf,                         // Deletion technique.
			    Population::LinearNormalizedFitness,            // Fitness evaluation.
			    Population::VariableLengthNotPermitted,         // If Variable Length Chromosomes are Ok.
			    26,                                             // Number of permitted states(order) of each base.
			    500                                      
			    );
	//
	// Let the population go.
	//
	travelingSalesman.run();
      }
      break;
    default:
      usage();
    }
  return 0;
}



