#include <ostream>

#include "base.hh"
#include "except.hh"
#include "graphcoloring.hh"
#include "population.hh"

// A 16-node "2-tree": starting from triangle A-B-C, every later node is
// joined to both ends of an existing edge. Such graphs are uniquely
// 3-colorable, so only 6 of the 3^16 (~43 million) assignments are proper
// and a random initial population essentially never contains a solution.
const GraphColoring::Edge GraphColoring::kEdges[GraphColoring::kEdgeCount] = {
   {0, 1}, {1, 2}, {0, 2}, {1, 3}, {2, 3}, {2, 4}, {3, 4}, {3, 5}, {4, 5}, {3, 6},
   {5, 6}, {3, 7}, {5, 7}, {4, 8}, {5, 8}, {4, 9}, {8, 9}, {3, 10}, {4, 10}, {2, 11},
   {4, 11}, {8, 12}, {9, 12}, {4, 13}, {9, 13}, {4, 14}, {11, 14}, {2, 15}, {11, 15}
};

const char* GraphColoring::kNodeNames[GraphColoring::kNodeCount] = {
   "A", "B", "C", "D", "E", "F", "G", "H",
   "I", "J", "K", "L", "M", "N", "O", "P"
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
