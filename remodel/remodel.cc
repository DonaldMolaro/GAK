/*
 * (C) Copyright 2008 Hitachi Global Storage Technologies Corporation.
 * All rights reserved.
 * All information contained herein is proprietary and confidential to
 * Hitachi Global Storage Technologies Corporation. Any use, reproduction,
 * or disclosure without written permission of Hitachi Global Storage
 * Technologies Corporation is prohibited.
 *
 * Author: Donald Molaro (Donald.Molaro@HitachiGST.com
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <math.h>
#include <assert.h>

#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

#include "point.h"
#include "base.hh"
#include "chromosome.hh"
#include "population.hh"

//#define VALUELENGTH 64
//#define FIXEDBASE 1000000000000.00
#define VALUELENGTH 32
#define FIXEDBASE (1.00*1000000.00)


class GenericPolyFinder : public Population
{
  vector< Point<double,double> > curveToMatch;
 public:
  GenericPolyFinder(vector<Point<double,double> > points,
		    Population::OperationTechnique Operation,
		    int numberofIndividuals,
		    int numberofTrials,
		    int GenecticDeversity,
		    double BitMutationRate,
		    double CrossOverRate,
		    Population::ReproductionTechniques ReproductionTechniques,
		    Population::ParrentSelectionTechnique ParentSelection,
		    Population::DeletetionTechnique Deletetion,
		    Population::FitnessTechnique Fitness,
		    Population::VariableLength Variable,
		    int baseStates
		    ):Population(Operation,numberofIndividuals,numberofTrials,
				 GenecticDeversity,BitMutationRate,CrossOverRate,
				 ReproductionTechniques,ParentSelection,
				 Deletetion,Fitness,Variable,baseStates),curveToMatch(points)
  {
    fprintf(stderr,"In <<GPF>> Construtor\n");
  };

  static double expoentiation(double x,unsigned int exponet)
  {
    double result = 1.0000;
    int exp = exponet;
    while (exp--) result *= x;
    return result;
  }

  static double Poly(double x,vector<double> p)
  {
    /*
     * A generic polynomial...
     * 
     * for example if the vector that get's passed in has three elements....
     * result = p[0] *( x ^ 2 ) + p[1] * (x ^1) + p[2] * (x^0)
     *
     */
    double result = 0.00;
    switch (p.size())
      {
      case 5: result += p[4] * (x * x * x * x);	// fprintf(stderr,"(%3.2f(%3.2f)^4)+",p[4],x);
      case 4: result += p[3] * (x * x * x);     // fprintf(stderr,"(%3.2f(%3.2f)^3)+",p[3],x);
      case 3: result += p[2] * (x * x);	        // fprintf(stderr,"(%3.2f(%3.2f)^2)+",p[2],x);
      case 2: result += p[1] * (x);        	// fprintf(stderr,"(%3.2f(%3.2f))+"  ,p[1],x);
      case 1: result += p[0];	                // fprintf(stderr,"(%3.2f) ="        ,p[0]);
	break;
      default:
	fprintf(stderr,"Order must be between 1-5\n");
	assert(0);
      }
    return result;
  }

  virtual double FitnessFunction(BaseString *b)
  {
    double rawfitness = 0.00;
    vector<double> parameters;
    for ( int i = 0 ; i < b->length() ; i += VALUELENGTH )
      {
	int p = decode<int>(b,i,i+VALUELENGTH);
	parameters.push_back( (double)p / FIXEDBASE );
      }
    vector< Point<double,double> >::iterator p;
    for ( p = curveToMatch.begin() ; p != curveToMatch.end() ; p++ )
      {
	double dist = ( p->distance( Point<double,double>(p->x(),Poly(p->x(),parameters))) );
	rawfitness += (dist);
      }
    return (rawfitness / (double)curveToMatch.size());
  }

  void FitnessPrint(FILE *fp,BaseString *b)
  {
    vector<double> parameters;
    int x = 0;
    fprintf(fp," f(x) = ");

    for ( int i = 0 ; i < b->length() ; i += VALUELENGTH )
      {
	double p = ((double)decode<int>(b,i,i+VALUELENGTH))/FIXEDBASE;
	switch (x)
	  {
	  case 0:
	    fprintf(fp,"%3.8f ", p);
	    break;
	  case 1:
	    fprintf(fp,"(%3.8f * x) ",p);
	    break;
	  default:
	    fprintf(fp,"(%3.8f * (x**%d)) ",p,x);
	    break;
	  }
	if ((i+VALUELENGTH) != b->length()) fprintf(fp," + ");
	x++;
      }
  }
  
  int randomBit() { return (random()&0x01); };
  bool select(double rate)
  {
    int range = (int)nearbyint(rate * 65535.00);
    int value = random() & 0xFFFF;
    return (value < range);
  }

  void mutate(Chromosome *child,double rate)
  {
    /*
     * The probablity is the probablity of a single bit mutation.
     * but since we are working with signed values a single bit
     * mutation will change a very small postive/negative value into a 
     * very large negative/positive value which is not what we want 
     * in this case.
     *
     */
    BaseString *b = child->ChromosomeStr();

    // double before,after;

    for ( int i = 0 ; i < b->length() ; i += VALUELENGTH )
      {
	if (select(rate * double(VALUELENGTH)) )
	  {
	    //
	    // This bitsting now has a mutation in it.
	    // select the bit to flip.
	    //
	    int bitToFlip = random() & 0x1F;
	    switch (bitToFlip)
	      {
	      case 0:
		//
		// If we select the first (sign) bit then flip it *and*
		// all of the other bits as well.
		//

		// before = ((double)decode(b,i,i+VALUELENGTH))/FIXEDBASE;

		for (int bit = 0 ; bit < VALUELENGTH ; bit++)
		  {
		    b->set(i+bit,b->test(i+bit) ? 0 : 1);
		  }

		// after = ((double)decode(b,i,i+VALUELENGTH))/FIXEDBASE;

		//fprintf(stderr, "Sign flip..(%3.4f) --> (%3.4f)\n",before,after);

		break;
	      default:
		//
		// flip bit i + bitToFlip
		//
		b->set(i+bitToFlip,b->test(i+bitToFlip) ? 0 : 1);
		/*
		for (int bit = bitToFlip ; bit < VALUELENGTH ; bit++)
		  {
		    b->set(i+bit,b->test(i+bit) ? 0 : 1);
		  }
		*/

		break;
	      }
	  }
      }
  }

  bool callback(BaseString *b,double fitness,int generation,int evaluations)
  {
    FitnessPrint(stdout,b);
    fprintf(stdout,"\nreplot\n");
    FitnessPrint(stderr,b);
    fprintf(stderr," Fitness : %3.4f Generation %d Evaluations %d \n",
	    fitness,generation,evaluations);
    return true;
  }

};

vector< Point<double,double> > rationalize(vector< Point<double,double> > &raw, int base)
{
  vector<Point<double, double> > cooked;

  Point<double,double> minimum(raw.begin()->x(),raw.begin()->y());
  Point<double,double> maximum(raw.begin()->x(),raw.begin()->y());

  for (vector<Point<double,double> >::iterator i = raw.begin() ; i != raw.end() ; i++ )
    {
      minimum = Point<double,double>(min<double>(minimum.x(),i->x()),min<double>(minimum.y(),i->y()));
      maximum = Point<double,double>(max<double>(maximum.x(),i->x()),max<double>(maximum.y(),i->y()));
    }

  minimum = Point<double,double>(0.00,0.00); // A hack to make the animation look nice.
    
  Point<double,double> range = maximum - minimum;

  for (vector<Point<double,double> >::iterator i = raw.begin() ; i != raw.end() ; i++ )
    {
      Point<double,double> normalized = (*i) - minimum;
      Point<double,double> scaled     = Point<double,double>( (normalized.x() / range.x()) * (double)base,
							      (normalized.y() / range.y()) * (double)base);
      
      cooked.push_back( scaled );
    }
  return cooked;
}



int main(int argc,char *argv[])
{
  if (argc == 3)
    {
      int order = atoi(argv[2]);

      if (( order >= 1 ) && ( order < 6 ))
	{
	  vector< Point<double,double> > rawPoints;

	  ifstream ifile;
	  ifile.open(string(argv[1]).c_str());

	  while (ifile.eof() == false)
	    {
	      double x; 
	      double y;
	      ifile >> x >> y;
	      Point<double,double> p(x,y);
	      rawPoints.push_back(p);
	    }

	  vector< Point<double,double> > points = rationalize(rawPoints,1000);

	  vector< Point<double,double> >::iterator p = points.begin();

	  cout << "set xrange [0:1000]" << endl;
	  cout << "set yrange [0:1200]" << endl;

	  {
	    ofstream pointsTXT("points.txt");
	    while ( p != points.end() )
	      {
		pointsTXT << (*p++) << endl;
	      }
	  }
	  cout << "f(x) = 1\n";
	  cout << "plot \"points.txt\",f(x) linewidth 5 " << endl;

	  GenericPolyFinder GPF(points,
				Population::Minimize,                     // Operation
				2000,                                     // Population Size.
				100000000,                                 // Number in civilzation
				VALUELENGTH + (VALUELENGTH * order),      // Genentic Length
				0.080,                                    // Mutation rate.
				0.65,                                     // Cross overate.
				Population::DuplicatesNotAllowed,         // Duplication permitted.
				Population::RouletteWheel,                // Parent Selection
				Population::DeleteHalf,                   // Deletion technique.
				Population::LinearNormalizedFitness,      // Fitness evaluation.
				Population::VariableLengthNotPermitted,   // If Variable Length Chromosomes are Ok.
				2                                         // Number of permitted states of each base.
				);
	  GPF.run();
	}
      else
	{
	  cerr << "order must be between 1 - 5" << endl;
	}
    }
  else
    {
      cerr << "Usage remodel <datafile.txt> <order>" << endl;
    }
  return 0;
};

