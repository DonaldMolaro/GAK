#pragma once

#include <iosfwd>

#include "population.hh"

class PopulationRunReportOptions
{
public:
   PopulationRunReportOptions(bool includeSettingsValue = false,
                              bool includeGenerationSummariesValue = false)
      : includeSettings(includeSettingsValue),
        includeGenerationSummaries(includeGenerationSummariesValue)
   {
   }

   bool includeSettings;
   bool includeGenerationSummaries;
};

class PopulationReporter
{
public:
   static void write(std::ostream& out,
                     const Population& population,
                     const Population::RunResult& result,
                     const PopulationRunReportOptions& options = PopulationRunReportOptions());
   static void writeJson(std::ostream& out,
                         const Population& population,
                         const Population::RunResult& result);
   static void writeGenerationCsv(std::ostream& out,
                                  const Population& population,
                                  const Population::RunResult& result);
};
