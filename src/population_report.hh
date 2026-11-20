#pragma once

#include <iosfwd>
#include <string>

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
   // `extraResultMembers`, when not empty, is pre-rendered JSON object
   // members (for example `"selected": {...}`) appended to the "result"
   // object, for problem-specific results.
   static void writeJson(std::ostream& out,
                         const Population& population,
                         const Population::RunResult& result,
                         const std::string& extraResultMembers = std::string());
   static void writeGenerationCsv(std::ostream& out,
                                  const Population& population,
                                  const Population::RunResult& result);
};
