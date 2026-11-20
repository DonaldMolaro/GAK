#include <iomanip>
#include <ostream>
#include <string>

#include "chromosome.hh"
#include "except.hh"
#include "base.hh"
#include "population.hh"
#include "population_report.hh"

namespace
{
// Restores the caller's stream formatting when a report finishes.
class StreamStateGuard
{
public:
   explicit StreamStateGuard(std::ostream& stream)
      : stream_(stream), flags_(stream.flags()), precision_(stream.precision())
   {
   }
   ~StreamStateGuard()
   {
      stream_.flags(flags_);
      stream_.precision(precision_);
   }
   StreamStateGuard(const StreamStateGuard&) = delete;
   StreamStateGuard& operator=(const StreamStateGuard&) = delete;
private:
   std::ostream& stream_;
   std::ios::fmtflags flags_;
   std::streamsize precision_;
};

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
   switch (mode)
   {
   case Population::ParentSelectionMode::Random: return "random";
   case Population::ParentSelectionMode::Tournament: return "tournament";
   default: return "roulette";
   }
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

void writeJsonIntArray(std::ostream& out, const std::vector<int>& values)
{
   out << '[';
   for (std::size_t i = 0 ; i < values.size() ; i++)
   {
      if (i > 0)
      {
         out << ',';
      }
      out << values[i];
   }
   out << ']';
}

BaseString buildSnapshotGenes(const Population& population, const std::vector<int>& geneValues)
{
   BaseString genes(static_cast<int>(geneValues.size()), population.settings().baseStates);
   for (int i = 0 ; i < static_cast<int>(geneValues.size()) ; i++)
   {
      genes.setValue(i, geneValues[static_cast<std::size_t>(i)]);
   }
   return genes;
}
}

void PopulationReporter::write(std::ostream& out,
                               const Population& population,
                               const Population::RunResult& result,
                               const PopulationRunReportOptions& options)
{
   StreamStateGuard guard(out);
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

   // Index of the i-th best (or i-th worst) member in the ascending table.
   auto rankedIndex = [&](int i, bool best) {
      const bool highIsBest = population.settings().operation == Population::OperationMode::Maximize;
      const int highIndex = (population.settings().numberOfIndividuals - 1) - i;
      return (best == highIsBest) ? highIndex : i;
   };

   auto printFitnessList = [&](const char* label, const std::vector<double>& values) {
      out << label << ' ' << values.size() << " Fitness Values:";
      for (double value : values)
      {
         out << ' ' << std::fixed << std::setprecision(8) << value << std::defaultfloat;
      }
      out << '\n';
   };

   // A generation report only snapshots the best member's genes, so print
   // that member plus the fitness values; the live population table holds
   // the final generation and must not be paired with historical values.
   auto printGenerationSummary = [&](const Population::GenerationReport& report) {
      if (report.summary.mostFit.empty())
      {
         return;
      }
      if (!report.bestCandidateGenes.empty())
      {
         out << "Best Member:\n";
         population.printCandidate(buildSnapshotGenes(population, report.bestCandidateGenes), out);
         out << "(F = " << std::fixed << std::setprecision(8) << report.bestCandidateFitness
             << ")\n" << std::defaultfloat;
      }
      printFitnessList("Best", report.summary.mostFit);
      printFitnessList("Worst", report.summary.leastFit);
      if (!report.fitnessQuartiles.empty())
      {
         out << "Population fitness (best, 25%, median, 75%, worst):";
         for (double value : report.fitnessQuartiles)
         {
            out << ' ' << std::fixed << std::setprecision(8) << value << std::defaultfloat;
         }
         out << '\n';
      }
   };

   auto printGenerationProgress = [&](const Population::GenerationReport& report, bool printSummary) {
      out << "Generation " << report.generation
          << " Number of Evaluations " << report.evaluations << " \n";
      if (printSummary)
      {
         printGenerationSummary(report);
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
      Population::GenerationReport finalProgress;
      finalProgress.generation = result.generationsCompleted;
      finalProgress.evaluations = result.evaluations;
      printGenerationProgress(finalProgress, false);
   }

   const int summaryCount = static_cast<int>(result.finalSummary.mostFit.size());
   if (population.settings().operation != Population::OperationMode::Maximize &&
       population.settings().operation != Population::OperationMode::Minimize)
   {
      throw GAFatalException(__FILE__,__LINE__,"Unsupported operation technique");
   }
   for ( int i = 0 ; i < summaryCount ; i++ )
   {
      population.printCandidate(population.populationTable[rankedIndex(i, true)].get()->genes(), out);
      out << std::fixed << std::setprecision(6) << result.finalSummary.mostFit[i]
          << '\n' << std::defaultfloat;
   }
}

void PopulationReporter::writeJson(std::ostream& out,
                                   const Population& population,
                                   const Population::RunResult& result,
                                   const std::string& extraResultMembers)
{
   StreamStateGuard guard(out);
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
   out << "    \"tournament_size\": " << settings.tournamentSize << ",\n";
   out << "    \"deletion\": \"" << deletionLabel(settings.deletion) << "\",\n";
   out << "    \"fitness\": \"" << fitnessLabel(settings.fitness) << "\",\n";
   out << "    \"variable_length\": \"" << variableLengthLabel(settings.variableLength) << "\"\n";
   out << "  },\n";
   out << "  \"result\": {\n";
   out << "    \"random_seed\": " << result.randomSeed << ",\n";
   out << "    \"used_configured_seed\": " << (result.usedConfiguredSeed ? "true" : "false") << ",\n";
   out << "    \"stopped_early\": " << (result.stoppedEarly ? "true" : "false") << ",\n";
   out << "    \"solution_found\": " << (result.solutionFound ? "true" : "false") << ",\n";
   out << "    \"stalled\": " << (result.stalled ? "true" : "false") << ",\n";
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
      out << "        \"best_candidate_fitness\": " << std::fixed << std::setprecision(8)
          << report.bestCandidateFitness << ",\n" << std::defaultfloat;
      out << "        \"best_candidate_genes\": ";
      writeJsonIntArray(out, report.bestCandidateGenes);
      out << ",\n";
      out << "        \"visualization\": ";
      population.writeVisualizationJson(buildSnapshotGenes(population, report.bestCandidateGenes), out);
      out << ",\n";
      out << "        \"most_fit\": ";
      writeJsonArray(out, report.summary.mostFit);
      out << ",\n";
      out << "        \"least_fit\": ";
      writeJsonArray(out, report.summary.leastFit);
      out << ",\n";
      // The spread of the whole population: fitness at five ranks, and a few
      // members sampled at evenly spaced ranks (best first).
      out << "        \"fitness_quartiles\": ";
      writeJsonArray(out, report.fitnessQuartiles);
      out << ",\n";
      out << "        \"samples\": [";
      for (std::size_t s = 0 ; s < report.samples.size() ; s++)
      {
         const Population::MemberSample& sample = report.samples[s];
         out << (s == 0 ? "\n" : ",\n") << "          { \"rank\": " << sample.rank
             << ", \"fitness\": " << std::fixed << std::setprecision(8) << sample.fitness << std::defaultfloat
             << ", \"visualization\": ";
         population.writeSampleJson(buildSnapshotGenes(population, sample.genes), out);
         out << " }";
      }
      out << (report.samples.empty() ? "]\n" : "\n        ]\n");
      out << "      }";
      if (i + 1 < result.generationReports.size())
      {
         out << ',';
      }
      out << '\n';
   }
   out << "    ]";
   if (!extraResultMembers.empty())
   {
      out << ",\n    " << extraResultMembers;
   }
   out << "\n  }\n";
   out << "}\n";
}

void PopulationReporter::writeGenerationCsv(std::ostream& out,
                                            const Population&,
                                            const Population::RunResult& result)
{
   StreamStateGuard guard(out);
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
