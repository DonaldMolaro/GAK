#include <ostream>

#include "base.hh"
#include "except.hh"
#include "graphcoloring.hh"
#include "population.hh"

const GraphColoring::Edge GraphColoring::kEdges[GraphColoring::kEdgeCount] = {
   {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 0},
   {6, 0}, {6, 2}, {6, 4},
   {7, 1}, {7, 3}, {7, 5}
};

const char* GraphColoring::kNodeNames[GraphColoring::kNodeCount] = {
   "A", "B", "C", "D", "E", "F", "G", "H"
};

const char* GraphColoring::kColorNames[GraphColoring::kColorCount] = {
   "red", "green", "blue"
};

double GraphColoring::evaluateFitness(const BaseString& genes)
{
   int properEdges = 0;
   for (int edge = 0 ; edge < kEdgeCount ; edge++)
   {
      if (genes.valueAt(kEdges[edge].left) != genes.valueAt(kEdges[edge].right))
      {
         properEdges++;
      }
   }
   return properEdges;
}

void GraphColoring::printCandidate(const BaseString& genes, std::ostream& out) const
{
   out << "Coloring:";
   for (int node = 0 ; node < kNodeCount ; node++)
   {
      out << ' ' << kNodeNames[node] << '=' << kColorNames[genes.valueAt(node)];
   }

   int conflicts = 0;
   for (int edge = 0 ; edge < kEdgeCount ; edge++)
   {
      if (genes.valueAt(kEdges[edge].left) == genes.valueAt(kEdges[edge].right))
      {
         conflicts++;
      }
   }
   out << " | conflicts=" << conflicts << " ::";
}

void GraphColoring::writeVisualizationJson(const BaseString& genes, std::ostream& out) const
{
   out << "{ \"type\": \"graph_coloring\", \"nodes\": [";
   for (int node = 0 ; node < kNodeCount ; node++)
   {
      if (node > 0)
      {
         out << ", ";
      }
      out << "{ \"name\": \"" << kNodeNames[node]
          << "\", \"color_index\": " << genes.valueAt(node)
          << ", \"color_name\": \"" << kColorNames[genes.valueAt(node)] << "\" }";
   }
   out << "], \"edges\": [";
   for (int edge = 0 ; edge < kEdgeCount ; edge++)
   {
      if (edge > 0)
      {
         out << ", ";
      }
      const bool conflict = genes.valueAt(kEdges[edge].left) == genes.valueAt(kEdges[edge].right);
      out << "{ \"left\": " << kEdges[edge].left
          << ", \"right\": " << kEdges[edge].right
          << ", \"conflict\": " << (conflict ? "true" : "false") << " }";
   }
   out << "] }";
}

bool GraphColoring::hasReachedSolution(const Population&, const BaseString&, double fitness) const
{
   return fitness >= kEdgeCount;
}

void GraphColoring::validatePopulation(const Population& population) const
{
   if (population.settings().chromosomeLength != kNodeCount)
   {
      throw GAFatalException(__FILE__, __LINE__,
                             "GraphColoring requires one gene per graph node.");
   }
   if (population.settings().baseStates != kColorCount)
   {
      throw GAFatalException(__FILE__, __LINE__,
                             "GraphColoring requires exactly three graph colors.");
   }
}
