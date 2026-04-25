#include <iostream>
#include "base.hh"
#include "chromosome.hh"
#include "population.hh"
#include "spell.hh"

Spell::Spell(const Population::Settings& settings)
   : Population(settings)
{
}

double Spell::evaluateFitness(const BaseString& genes)
{
   //
   // Fitness is the total squared distance that each 
   // element in the BaseString is wrong by.
   //
   int res = 0;
   //
   // ok for each element in the correct order.
   // 
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
      switch(i)
      {
      case 0:
	 if (genes.test(i) == 'e' - 'a') res++;
	 break;
      case 1:
	 if (genes.test(i) == 'g' - 'a') res++;
	 break;
      case 2:
	 if (genes.test(i) == 'g' - 'a') res++;
	 break;
      case 3:
	 if (genes.test(i) == 'h' - 'a') res++;
	 break;
      case 4:
	 if (genes.test(i) == 'e' - 'a') res++;
	 break;
      case 5:
	 if (genes.test(i) == 'a' - 'a') res++;
	 break;
      case 6:
	 if (genes.test(i) == 'd' - 'a') res++;
	 break;
      default:
	 if (res > 1) res--;
	 break;
      }
   }
   //
   return res;
};


void Spell::printCandidate(const BaseString& genes, std::ostream& out)
{
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
/*
      assert(b->test(i) >= 0);
      assert(b->test(i) < 26);
*/
      out << static_cast<char>(genes.test(i) + 'a');
   }
   out << " ::";
}
