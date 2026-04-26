#include <iomanip>
#include <ostream>
#include <string>

#include "chromosome.hh"
#include "except.hh"
#include "population.hh"
#include "population_report.hh"

namespace
{
const char* operationLabel(Population::OperationMode mode)
{
   return mode == Population::OperationMode::Minimize ? "minimize" : "maximize";
}

const char* reproductionLabel(Population::ReproductionMode mode)
{
   return mode == Population::ReproductionMode::DisallowDuplicates ? "disallow_duplicates"
                                                                   : "allow_duplicates";
}

const char* selectionLabel(Population::ParentSelectionMode mode)
{
   return mode == Population::ParentSelectionMode::Random ? "random" : "roulette";
}

const char* deletionLabel(Population::DeletionMode mode)
{
   switch (mode)
   {
   case Population::DeletionMode::DeleteAll: return "delete_all";
   case Population::DeletionMode::DeleteAllButBest: return "delete_all_but_best";
   case Population::DeletionMode::DeleteHalf: return "delete_half";
   case Population::DeletionMode::DeleteQuarter: return "delete_quarter";
   case Population::DeletionMode::DeleteLast: return "delete_last";
   }
   return "unknown";
}

const char* fitnessLabel(Population::FitnessMode mode)
{
   switch (mode)
   {
   case Population::FitnessMode::Evaluation: return "evaluation";
   case Population::FitnessMode::Windowed: return "windowed";
   case Population::FitnessMode::LinearNormalized: return "linear_normalized";
   }
   return "unknown";
}

const char* variableLengthLabel(Population::VariableLengthMode mode)
{
   return mode == Population::VariableLengthMode::Variable ? "variable" : "fixed";
}

void writeJsonArray(std::ostream& out, const std::vector<double>& values)
{
   out << '[';
   for (std::size_t i = 0 ; i < values.size() ; i++)
   {
      if (i > 0)
      {
         out << ',';
      }
      out << std::fixed << std::setprecision(8) << values[i] << std::defaultfloat;
   }
   out << ']';
}
}

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
      out << "Chromosome Length     :: " << settings.chromosomeLength << '\n';
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
         population.printCandidate(population.populationTable[index].get()->genes(), out);
         out << "(F = " << std::fixed << std::setprecision(8) << summary.mostFit[i]
             << ")\n" << std::defaultfloat;
      }

      if (population.settings().operation == Population::OperationMode::Maximize)
         out << "Worst " << summaryCount << " Members are:\n";
      else
         out << "Best " << summaryCount << " Members are:\n";

      for ( int i = 0 ; i < summaryCount ; i++ )
      {
         population.printCandidate(population.populationTable[i].get()->genes(), out);
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
         population.printCandidate(population.populationTable[index].get()->genes(), out);
         out << std::fixed << std::setprecision(6) << result.finalSummary.mostFit[i]
             << '\n' << std::defaultfloat;
      }
      break;
      case Population::OperationMode::Minimize:
      for ( int i = 0 ; i < summaryCount ; i++ )
      {
         population.printCandidate(population.populationTable[i].get()->genes(), out);
         out << std::fixed << std::setprecision(6) << result.finalSummary.leastFit[i]
             << '\n' << std::defaultfloat;
      }
      break;
      default:
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
}

void PopulationReporter::writeJson(std::ostream& out,
                                   const Population& population,
                                   const Population::RunResult& result)
{
   const Population::Settings& settings = population.settings();
   out << "{\n";
   out << "  \"settings\": {\n";
   out << "    \"operation\": \"" << operationLabel(settings.operation) << "\",\n";
   out << "    \"number_of_individuals\": " << settings.numberOfIndividuals << ",\n";
   out << "    \"number_of_trials\": " << settings.numberOfTrials << ",\n";
   out << "    \"chromosome_length\": " << settings.chromosomeLength << ",\n";
   out << "    \"base_states\": " << settings.baseStates << ",\n";
   out << "    \"bit_mutation_rate\": " << std::fixed << std::setprecision(8)
       << settings.bitMutationRate << ",\n";
   out << "    \"cross_over_rate\": " << settings.crossOverRate << ",\n" << std::defaultfloat;
   out << "    \"reproduction\": \"" << reproductionLabel(settings.reproduction) << "\",\n";
   out << "    \"parent_selection\": \"" << selectionLabel(settings.parentSelection) << "\",\n";
   out << "    \"deletion\": \"" << deletionLabel(settings.deletion) << "\",\n";
   out << "    \"fitness\": \"" << fitnessLabel(settings.fitness) << "\",\n";
   out << "    \"variable_length\": \"" << variableLengthLabel(settings.variableLength) << "\"\n";
   out << "  },\n";
   out << "  \"result\": {\n";
   out << "    \"random_seed\": " << result.randomSeed << ",\n";
   out << "    \"used_configured_seed\": " << (result.usedConfiguredSeed ? "true" : "false") << ",\n";
   out << "    \"stopped_early\": " << (result.stoppedEarly ? "true" : "false") << ",\n";
   out << "    \"solution_found\": " << (result.solutionFound ? "true" : "false") << ",\n";
   out << "    \"generations_completed\": " << result.generationsCompleted << ",\n";
   out << "    \"evaluations\": " << result.evaluations << ",\n";
   out << "    \"final_summary\": {\n";
   out << "      \"most_fit\": ";
   writeJsonArray(out, result.finalSummary.mostFit);
   out << ",\n";
   out << "      \"least_fit\": ";
   writeJsonArray(out, result.finalSummary.leastFit);
   out << "\n";
   out << "    },\n";
   out << "    \"generation_reports\": [\n";
   for (std::size_t i = 0 ; i < result.generationReports.size() ; i++)
   {
      const Population::GenerationReport& report = result.generationReports[i];
      out << "      {\n";
      out << "        \"generation\": " << report.generation << ",\n";
      out << "        \"evaluations\": " << report.evaluations << ",\n";
      out << "        \"most_fit\": ";
      writeJsonArray(out, report.summary.mostFit);
      out << ",\n";
      out << "        \"least_fit\": ";
      writeJsonArray(out, report.summary.leastFit);
      out << "\n";
      out << "      }";
      if (i + 1 < result.generationReports.size())
      {
         out << ',';
      }
      out << '\n';
   }
   out << "    ]\n";
   out << "  }\n";
   out << "}\n";
}

void PopulationReporter::writeGenerationCsv(std::ostream& out,
                                            const Population&,
                                            const Population::RunResult& result)
{
   out << "generation,evaluations,best_fitness,worst_fitness\n";
   for (std::size_t i = 0 ; i < result.generationReports.size() ; i++)
   {
      const Population::GenerationReport& report = result.generationReports[i];
      const double bestFitness = report.summary.mostFit.empty() ? 0.0 : report.summary.mostFit.front();
      const double worstFitness = report.summary.leastFit.empty() ? 0.0 : report.summary.leastFit.front();
      out << report.generation << ','
          << report.evaluations << ','
          << std::fixed << std::setprecision(8) << bestFitness << ','
          << worstFitness << '\n' << std::defaultfloat;
   }
}
