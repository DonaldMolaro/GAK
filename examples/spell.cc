#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "spell.hh"

Spell::Spell(const Population::Options& options)
   : Spell(options.toConfiguration())
{
}

Spell::Spell(const Population::Configuration& configuration)
   : Population(configuration)
{
}

double Spell::FitnessFunction(const BaseString& b)
{
   //
   // Fitness is the total squared distance that each 
   // element in the BaseString is wrong by.
   //
   int res = 0;
   //
   // ok for each element in the correct order.
   // 
   for ( int i = 0 ; i < b.length() ; i++ )
   {
      switch(i)
      {
      case 0:
	 if (b.test(i) == 'e' - 'a') res++;
	 break;
      case 1:
	 if (b.test(i) == 'g' - 'a') res++;
	 break;
      case 2:
	 if (b.test(i) == 'g' - 'a') res++;
	 break;
      case 3:
	 if (b.test(i) == 'h' - 'a') res++;
	 break;
      case 4:
	 if (b.test(i) == 'e' - 'a') res++;
	 break;
      case 5:
	 if (b.test(i) == 'a' - 'a') res++;
	 break;
      case 6:
	 if (b.test(i) == 'd' - 'a') res++;
	 break;
      default:
	 if (res > 1) res--;
	 break;
      }
   }
   //
   return res;
};


void Spell::FitnessPrint(const BaseString& b, std::ostream& out)
{
   for ( int i = 0 ; i < b.length() ; i++ )
   {
/*
      assert(b->test(i) >= 0);
      assert(b->test(i) < 26);
*/
      out << static_cast<char>(b.test(i) + 'a');
   }
   out << " ::";
}
