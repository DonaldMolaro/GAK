#include <iomanip>
#include <ostream>

#include "chromosome.hh"
#include "except.hh"
#include "population.hh"
#include "population_report.hh"

void PopulationReporter::write(std::ostream& out,
                               const Population& population,
                               const Population::RunResult& result,
                               const PopulationRunReportOptions& options)
{
   if (options.includeSettings)
   {
      const Population::Settings& settings = population.settings();
      out << "Operation             :: "
          << (settings.operation == Population::OperationMode::Minimize ? "Minimize" : "Maximize") << '\n';
      out << "Number of Individuals :: " << settings.numberOfIndividuals << '\n';
      out << "Number of Trials      :: " << settings.numberOfTrials << '\n';
      out << "Genetic Diversity     :: " << settings.geneticDiversity << '\n';
      out << "Mutation Rate         :: " << std::fixed << std::setprecision(4)
          << settings.bitMutationRate << '\n';
      out << "Cross Over Rate       :: " << std::fixed << std::setprecision(3)
          << settings.crossOverRate << '\n' << std::defaultfloat;
      out << "Duplicate Reproduction:: "
          << (settings.reproduction == Population::ReproductionMode::DisallowDuplicates ? "NOT Ok." : " Ok.") << '\n';
      out << "Variable              :: "
          << (settings.variableLength == Population::VariableLengthMode::Fixed ? "NOT Ok." : " Ok.") << '\n';
      out << "Random Seed           :: " << population.randomSeed()
          << (result.usedConfiguredSeed ? " (configured)" : " (generated)") << '\n';
   }

   auto printPopulationSummary = [&](const Population::PopulationSummary& summary) {
      const int summaryCount = static_cast<int>(summary.mostFit.size());
      if (summaryCount <= 0)
      {
         return;
      }

      if (population.settings().operation == Population::OperationMode::Maximize)
         out << "Best " << summaryCount << " Members are:\n";
      else
         out << "Worst " << summaryCount << " Members are:\n";

      for ( int i = 0 ; i < summaryCount ; i++ )
      {
         const int index = (population.settings().numberOfIndividuals - 1) - i;
         population.printCandidate(population.chromosomes()[index].get()->genes(), out);
         out << "(F = " << std::fixed << std::setprecision(8) << summary.mostFit[i]
             << ")\n" << std::defaultfloat;
      }

      if (population.settings().operation == Population::OperationMode::Maximize)
         out << "Worst " << summaryCount << " Members are:\n";
      else
         out << "Best " << summaryCount << " Members are:\n";

      for ( int i = 0 ; i < summaryCount ; i++ )
      {
         population.printCandidate(population.chromosomes()[i].get()->genes(), out);
         out << "(F = " << std::fixed << std::setprecision(8) << summary.leastFit[i]
             << ")\n" << std::defaultfloat;
      }
   };

   auto printGenerationProgress = [&](const Population::GenerationReport& report, bool printSummary) {
      out << "Generation " << report.generation
          << " Number of Evaluations " << report.evaluations << " \n";
      if (printSummary)
      {
         printPopulationSummary(report.summary);
      }
   };

   if (options.includeGenerationSummaries)
   {
      for (std::size_t i = 0 ; i < result.generationReports.size() ; i++ )
      {
         printGenerationProgress(result.generationReports[i], true);
      }
   }
   else
   {
      const Population::GenerationReport finalProgress = {
         result.generationsCompleted,
         result.evaluations,
         Population::PopulationSummary()
      };
      printGenerationProgress(finalProgress, false);
   }

   const int summaryCount = static_cast<int>(result.finalSummary.mostFit.size());
   switch(population.settings().operation)
   {
      case Population::OperationMode::Maximize:
      for ( int i = 0 ; i < summaryCount ; i++ )
      {
         const int index = (population.settings().numberOfIndividuals - 1) - i;
         population.printCandidate(population.chromosomes()[index].get()->genes(), out);
         out << std::fixed << std::setprecision(6) << result.finalSummary.mostFit[i]
             << '\n' << std::defaultfloat;
      }
      break;
      case Population::OperationMode::Minimize:
      for ( int i = 0 ; i < summaryCount ; i++ )
      {
         population.printCandidate(population.chromosomes()[i].get()->genes(), out);
         out << std::fixed << std::setprecision(6) << result.finalSummary.leastFit[i]
             << '\n' << std::defaultfloat;
      }
      break;
      default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
}
