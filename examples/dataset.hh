#pragma once

// Numeric tabular data for regression examples, loaded from CSV.

#include <iosfwd>
#include <string>
#include <vector>

struct Dataset
{
   std::string source;                       // file name, for messages
   std::vector<std::string> inputNames;      // one per input column
   std::string targetName;
   std::vector<std::vector<double> > inputs; // inputs[row][column]
   std::vector<double> targets;              // targets[row]

   int rowCount() const { return static_cast<int>(targets.size()); }
   int inputCount() const { return static_cast<int>(inputNames.size()); }
};

// Load a CSV file with a header row and numeric values. `targetColumn` names
// the output column; when empty the last column is used. Every other column
// is an input. Blank lines are ignored. Any malformed value, ragged row,
// unknown target or file with fewer than `minimumRows` rows throws a
// GAFatalException naming the source and line.
Dataset loadCsvDataset(const std::string& path,
                       const std::string& targetColumn = std::string(),
                       int minimumRows = 3);

// The same, reading from an already-open stream; `sourceName` is only used
// in error messages.
Dataset loadCsvDataset(std::istream& in,
                       const std::string& sourceName,
                       const std::string& targetColumn = std::string(),
                       int minimumRows = 3);
