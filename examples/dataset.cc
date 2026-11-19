#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "dataset.hh"
#include "except.hh"

namespace
{
std::string trim(const std::string& text)
{
   const std::string whitespace = " \t\r\n";
   const std::size_t first = text.find_first_not_of(whitespace);
   if (first == std::string::npos)
   {
      return std::string();
   }
   const std::size_t last = text.find_last_not_of(whitespace);
   return text.substr(first, last - first + 1);
}

std::vector<std::string> splitFields(const std::string& line)
{
   std::vector<std::string> fields;
   std::string field;
   std::istringstream stream(line);
   while (std::getline(stream, field, ','))
   {
      fields.push_back(trim(field));
   }
   // A trailing comma means a final empty field.
   if (!line.empty() && line[line.size() - 1] == ',')
   {
      fields.push_back(std::string());
   }
   return fields;
}

[[noreturn]] void fail(const std::string& source, int line, const std::string& reason)
{
   std::ostringstream message;
   message << source;
   if (line > 0)
   {
      message << ':' << line;
   }
   message << ": " << reason;
   throw GAFatalException(__FILE__, __LINE__, message.str());
}

bool parseNumber(const std::string& text, double& value)
{
   if (text.empty())
   {
      return false;
   }
   errno = 0;
   char* end = nullptr;
   const double parsed = std::strtod(text.c_str(), &end);
   if (errno != 0 || *end != '\0' || !std::isfinite(parsed))
   {
      return false;
   }
   value = parsed;
   return true;
}
}

Dataset loadCsvDataset(std::istream& in,
                       const std::string& sourceName,
                       const std::string& targetColumn,
                       int minimumRows)
{
   Dataset dataset;
   dataset.source = sourceName;

   std::string line;
   int lineNumber = 0;
   std::vector<std::string> header;
   while (std::getline(in, line))
   {
      ++lineNumber;
      if (!trim(line).empty())
      {
         header = splitFields(line);
         break;
      }
   }
   if (header.empty())
   {
      fail(sourceName, 0, "no header row found");
   }
   if (header.size() < 2)
   {
      fail(sourceName, lineNumber, "need at least one input column and one target column");
   }
   for (const std::string& name : header)
   {
      if (name.empty())
      {
         fail(sourceName, lineNumber, "header has an empty column name");
      }
   }

   int targetIndex = static_cast<int>(header.size()) - 1;
   if (!targetColumn.empty())
   {
      targetIndex = -1;
      for (int column = 0 ; column < static_cast<int>(header.size()) ; column++)
      {
         if (header[column] == targetColumn)
         {
            targetIndex = column;
         }
      }
      if (targetIndex < 0)
      {
         fail(sourceName, lineNumber, "target column '" + targetColumn + "' is not in the header");
      }
   }

   dataset.targetName = header[targetIndex];
   for (int column = 0 ; column < static_cast<int>(header.size()) ; column++)
   {
      if (column != targetIndex)
      {
         dataset.inputNames.push_back(header[column]);
      }
   }

   while (std::getline(in, line))
   {
      ++lineNumber;
      if (trim(line).empty())
      {
         continue;
      }
      const std::vector<std::string> fields = splitFields(line);
      if (fields.size() != header.size())
      {
         std::ostringstream reason;
         reason << "expected " << header.size() << " values, found " << fields.size();
         fail(sourceName, lineNumber, reason.str());
      }
      std::vector<double> row;
      row.reserve(fields.size() - 1);
      double target = 0.0;
      for (int column = 0 ; column < static_cast<int>(fields.size()) ; column++)
      {
         double value = 0.0;
         if (!parseNumber(fields[column], value))
         {
            fail(sourceName, lineNumber,
                 "column '" + header[column] + "' has a non-numeric value '" + fields[column] + "'");
         }
         if (column == targetIndex)
         {
            target = value;
         }
         else
         {
            row.push_back(value);
         }
      }
      dataset.inputs.push_back(row);
      dataset.targets.push_back(target);
   }

   if (dataset.rowCount() < minimumRows)
   {
      std::ostringstream reason;
      reason << "need at least " << minimumRows << " data rows, found " << dataset.rowCount();
      fail(sourceName, 0, reason.str());
   }
   return dataset;
}

Dataset loadCsvDataset(const std::string& path, const std::string& targetColumn, int minimumRows)
{
   std::ifstream in(path.c_str());
   if (!in)
   {
      fail(path, 0, "cannot open file");
   }
   return loadCsvDataset(in, path, targetColumn, minimumRows);
}
