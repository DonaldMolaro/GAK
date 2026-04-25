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
   int res = 0;
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
      switch(i)
      {
      case 0:
	 if (genes.valueAt(i) == 'e' - 'a') res++;
	 break;
      case 1:
	 if (genes.valueAt(i) == 'g' - 'a') res++;
	 break;
      case 2:
	 if (genes.valueAt(i) == 'g' - 'a') res++;
	 break;
      case 3:
	 if (genes.valueAt(i) == 'h' - 'a') res++;
	 break;
      case 4:
	 if (genes.valueAt(i) == 'e' - 'a') res++;
	 break;
      case 5:
	 if (genes.valueAt(i) == 'a' - 'a') res++;
	 break;
      case 6:
	 if (genes.valueAt(i) == 'd' - 'a') res++;
	 break;
      default:
	 if (res > 1) res--;
	 break;
      }
   }
   return res;
}

void Spell::printCandidate(const BaseString& genes, std::ostream& out)
{
   for ( int i = 0 ; i < genes.length() ; i++ )
   {
      out << static_cast<char>(genes.valueAt(i) + 'a');
   }
   out << " ::";
}
