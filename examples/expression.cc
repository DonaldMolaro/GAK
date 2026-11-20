#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "base.hh"
#include "except.hh"
#include "expression.hh"

namespace
{
const double kConstants[] = {0.1, 0.5, 1.0, 2.0, 3.0, 5.0, 10.0};
const Operation kUnary[] = {Operation::Sin, Operation::Cos, Operation::Exp, Operation::Log,
                            Operation::Sqrt, Operation::Square, Operation::Negate};
const Operation kBinary[] = {Operation::Add, Operation::Subtract, Operation::Multiply, Operation::Divide};

// Values with magnitude above this are treated as overflow: they are finite
// but would make the least-squares scaling numerically meaningless.
constexpr double kMaximumMagnitude = 1e12;
constexpr double kDivisionEpsilon = 1e-12;

int arity(Operation operation)
{
   switch (operation)
   {
   case Operation::Variable:
   case Operation::Constant:
      return 0;
   case Operation::Add:
   case Operation::Subtract:
   case Operation::Multiply:
   case Operation::Divide:
      return 2;
   default:
      return 1;
   }
}

bool apply(Operation operation, double a, double b, double& result)
{
   switch (operation)
   {
   case Operation::Add: result = a + b; break;
   case Operation::Subtract: result = a - b; break;
   case Operation::Multiply: result = a * b; break;
   case Operation::Divide:
      if (std::fabs(b) < kDivisionEpsilon) return false;
      result = a / b;
      break;
   case Operation::Sin: result = std::sin(a); break;
   case Operation::Cos: result = std::cos(a); break;
   case Operation::Exp:
      if (a > 700.0) return false;
      result = std::exp(a);
      break;
   case Operation::Log:
      if (a <= 0.0) return false;
      result = std::log(a);
      break;
   case Operation::Sqrt:
      if (a < 0.0) return false;
      result = std::sqrt(a);
      break;
   case Operation::Square: result = a * a; break;
   case Operation::Negate: result = -a; break;
   default:
      throw GAFatalException(__FILE__, __LINE__, "Expression: unexpected operation");
   }
   return std::isfinite(result) && std::fabs(result) <= kMaximumMagnitude;
}

const char* binarySymbol(Operation operation)
{
   switch (operation)
   {
   case Operation::Add: return " + ";
   case Operation::Subtract: return " - ";
   case Operation::Multiply: return "*";
   case Operation::Divide: return "/";
   default: return "?";
   }
}

const char* functionName(Operation operation)
{
   switch (operation)
   {
   case Operation::Sin: return "sin";
   case Operation::Cos: return "cos";
   case Operation::Exp: return "exp";
   case Operation::Log: return "log";
   case Operation::Sqrt: return "sqrt";
   default: return "?";
   }
}

std::string formatNumber(double value)
{
   std::ostringstream out;
   out << std::setprecision(4) << value;
   return out.str();
}
}

InstructionSet::InstructionSet(int inputCount, int variableCopies)
   : inputCount_(inputCount)
{
   if (inputCount <= 0 || variableCopies <= 0)
   {
      throw GAFatalException(__FILE__, __LINE__, "InstructionSet needs at least one input variable");
   }
   for (int copy = 0 ; copy < variableCopies ; copy++)
   {
      for (int variable = 0 ; variable < inputCount ; variable++)
      {
         Instruction instruction{Operation::Variable};
         instruction.variable = variable;
         instructions_.push_back(instruction);
      }
   }
   for (double constant : kConstants)
   {
      Instruction instruction{Operation::Constant};
      instruction.constant = constant;
      instructions_.push_back(instruction);
   }
   for (Operation operation : kUnary)
   {
      instructions_.push_back(Instruction{operation});
   }
   for (Operation operation : kBinary)
   {
      instructions_.push_back(Instruction{operation});
   }
}

Expression Expression::compile(const BaseString& genes, const InstructionSet& instructions)
{
   if (genes.states() != instructions.size())
   {
      throw GAFatalException(__FILE__, __LINE__, "Expression: gene alphabet does not match the instruction set");
   }

   // First pass: run the stack machine over node indices, building every
   // node the program creates.
   std::vector<ExpressionNode> all;
   std::vector<int> stack;
   for (int position = 0 ; position < genes.length() ; position++)
   {
      const Instruction& instruction = instructions.at(genes.valueAt(position));
      const int needed = arity(instruction.operation);
      if (static_cast<int>(stack.size()) < needed)
      {
         continue; // underflow: skip, keeping every program valid
      }
      ExpressionNode node;
      node.operation = instruction.operation;
      node.variable = instruction.variable;
      node.constant = instruction.constant;
      if (needed == 2)
      {
         node.right = stack.back(); stack.pop_back();
         node.left = stack.back(); stack.pop_back();
      }
      else if (needed == 1)
      {
         node.left = stack.back(); stack.pop_back();
      }
      all.push_back(node);
      stack.push_back(static_cast<int>(all.size()) - 1);
   }

   Expression expression;
   if (stack.empty())
   {
      return expression;
   }

   // Second pass: keep only the nodes that feed the top of the stack,
   // renumbered so children still precede parents.
   std::vector<char> reachable(all.size(), 0);
   reachable[stack.back()] = 1;
   for (int index = stack.back() ; index >= 0 ; --index)
   {
      if (!reachable[index]) continue;
      if (all[index].left >= 0) reachable[all[index].left] = 1;
      if (all[index].right >= 0) reachable[all[index].right] = 1;
   }
   std::vector<int> renumber(all.size(), -1);
   for (int index = 0 ; index <= stack.back() ; index++)
   {
      if (!reachable[index]) continue;
      ExpressionNode node = all[index];
      if (node.left >= 0) node.left = renumber[node.left];
      if (node.right >= 0) node.right = renumber[node.right];
      renumber[index] = static_cast<int>(expression.nodes_.size());
      expression.nodes_.push_back(node);
   }
   return expression;
}

bool Expression::evaluate(const std::vector<std::vector<double> >& inputs,
                          std::vector<double>& predictions) const
{
   const std::size_t rows = inputs.size();
   const double* values = evaluateAll(inputs);
   if (values == nullptr)
   {
      predictions.assign(rows, 0.0);
      return false;
   }
   const double* result = &values[(nodes_.size() - 1) * rows];
   predictions.assign(result, result + rows);
   return true;
}

bool Expression::evaluateNodes(const std::vector<std::vector<double> >& inputs,
                               const std::vector<int>& nodes,
                               std::vector<std::vector<double> >& values) const
{
   const std::size_t rows = inputs.size();
   const double* all = evaluateAll(inputs);
   values.resize(nodes.size());
   if (all == nullptr)
   {
      return false;
   }
   for (std::size_t i = 0 ; i < nodes.size() ; i++)
   {
      const double* start = &all[static_cast<std::size_t>(nodes[i]) * rows];
      values[i].assign(start, start + rows);
   }
   return true;
}

std::vector<int> Expression::additiveTerms(int maxTerms) const
{
   std::vector<int> terms;
   if (nodes_.empty())
   {
      return terms;
   }
   std::vector<int> pending(1, static_cast<int>(nodes_.size()) - 1);
   while (!pending.empty())
   {
      const int index = pending.back();
      pending.pop_back();
      const ExpressionNode& node = nodes_[index];
      if (node.operation == Operation::Add || node.operation == Operation::Subtract)
      {
         pending.push_back(node.right);
         pending.push_back(node.left);
      }
      else if (node.operation == Operation::Negate)
      {
         pending.push_back(node.left);
      }
      else
      {
         terms.push_back(index);
      }
   }
   if (static_cast<int>(terms.size()) > maxTerms)
   {
      terms.assign(1, static_cast<int>(nodes_.size()) - 1);
   }
   return terms;
}

bool Expression::usesVariable(int node) const
{
   std::vector<int> pending(1, node);
   while (!pending.empty())
   {
      const ExpressionNode& current = nodes_[pending.back()];
      pending.pop_back();
      if (current.operation == Operation::Variable)
      {
         return true;
      }
      if (current.left >= 0) pending.push_back(current.left);
      if (current.right >= 0) pending.push_back(current.right);
   }
   return false;
}

std::vector<int> Expression::variablesUsedBy(int node) const
{
   std::vector<int> variables;
   std::vector<int> pending(1, node);
   while (!pending.empty())
   {
      const ExpressionNode& current = nodes_[pending.back()];
      pending.pop_back();
      if (current.operation == Operation::Variable &&
          std::find(variables.begin(), variables.end(), current.variable) == variables.end())
      {
         variables.push_back(current.variable);
      }
      if (current.left >= 0) pending.push_back(current.left);
      if (current.right >= 0) pending.push_back(current.right);
   }
   std::sort(variables.begin(), variables.end());
   return variables;
}

std::vector<int> Expression::variablesUsed() const
{
   std::vector<int> variables;
   for (const ExpressionNode& node : nodes_)
   {
      if (node.operation == Operation::Variable &&
          std::find(variables.begin(), variables.end(), node.variable) == variables.end())
      {
         variables.push_back(node.variable);
      }
   }
   std::sort(variables.begin(), variables.end());
   return variables;
}

const double* Expression::evaluateAll(const std::vector<std::vector<double> >& inputs) const
{
   const std::size_t rows = inputs.size();
   if (nodes_.empty())
   {
      return nullptr;
   }

   // Node-major evaluation: one pass over all rows per node, in a reused
   // per-thread buffer (this runs millions of times per search).
   thread_local std::vector<double> values;
   values.resize(nodes_.size() * rows);
   for (std::size_t index = 0 ; index < nodes_.size() ; index++)
   {
      const ExpressionNode& node = nodes_[index];
      double* out = &values[index * rows];
      if (node.operation == Operation::Variable)
      {
         for (std::size_t row = 0 ; row < rows ; row++) out[row] = inputs[row][node.variable];
         continue;
      }
      if (node.operation == Operation::Constant)
      {
         for (std::size_t row = 0 ; row < rows ; row++) out[row] = node.constant;
         continue;
      }
      const double* left = &values[node.left * rows];
      const double* right = node.right >= 0 ? &values[node.right * rows] : nullptr;
      // One tight loop per operator; domain errors are checked inline and
      // overflow/non-finite results in a single pass afterwards. The
      // semantics match apply() exactly.
      std::size_t row = 0;
      switch (node.operation)
      {
      case Operation::Add: for (; row < rows ; row++) out[row] = left[row] + right[row]; break;
      case Operation::Subtract: for (; row < rows ; row++) out[row] = left[row] - right[row]; break;
      case Operation::Multiply: for (; row < rows ; row++) out[row] = left[row] * right[row]; break;
      case Operation::Divide:
         for (; row < rows ; row++)
         {
            if (std::fabs(right[row]) < kDivisionEpsilon) return nullptr;
            out[row] = left[row] / right[row];
         }
         break;
      case Operation::Sin: for (; row < rows ; row++) out[row] = std::sin(left[row]); break;
      case Operation::Cos: for (; row < rows ; row++) out[row] = std::cos(left[row]); break;
      case Operation::Exp:
         for (; row < rows ; row++)
         {
            if (left[row] > 700.0) return nullptr;
            out[row] = std::exp(left[row]);
         }
         break;
      case Operation::Log:
         for (; row < rows ; row++)
         {
            if (left[row] <= 0.0) return nullptr;
            out[row] = std::log(left[row]);
         }
         break;
      case Operation::Sqrt:
         for (; row < rows ; row++)
         {
            if (left[row] < 0.0) return nullptr;
            out[row] = std::sqrt(left[row]);
         }
         break;
      case Operation::Square: for (; row < rows ; row++) out[row] = left[row] * left[row]; break;
      case Operation::Negate: for (; row < rows ; row++) out[row] = -left[row]; break;
      default:
         throw GAFatalException(__FILE__, __LINE__, "Expression: unexpected operation");
      }
      for (row = 0 ; row < rows ; row++)
      {
         if (!(std::fabs(out[row]) <= kMaximumMagnitude)) return nullptr; // also rejects NaN
      }
   }
   return values.data();
}

std::vector<int> Expression::constantNodes() const
{
   std::vector<int> constants;
   for (int index = 0 ; index < static_cast<int>(nodes_.size()) ; index++)
   {
      if (nodes_[index].operation == Operation::Constant)
      {
         constants.push_back(index);
      }
   }
   return constants;
}

std::string Expression::toString(const std::vector<std::string>& variableNames) const
{
   if (nodes_.empty())
   {
      return "0";
   }
   return allTexts(variableNames).back();
}

std::string Expression::nodeText(const std::vector<std::string>& variableNames, int node) const
{
   return allTexts(variableNames)[node];
}

std::vector<std::string> Expression::allTexts(const std::vector<std::string>& variableNames) const
{

   // Which subtrees depend on an input variable; the rest fold to numbers.
   std::vector<char> variable(nodes_.size(), 0);
   std::vector<double> constant(nodes_.size(), 0.0);
   std::vector<char> foldable(nodes_.size(), 1);
   std::vector<std::string> text(nodes_.size());
   // Binding strength of each subtree's top operator: 1 for + and -, 2 for
   // * and /, 3 for anything that never needs parentheses as an operand.
   std::vector<int> precedence(nodes_.size(), 3);

   for (std::size_t index = 0 ; index < nodes_.size() ; index++)
   {
      const ExpressionNode& node = nodes_[index];
      if (node.operation == Operation::Variable)
      {
         variable[index] = 1;
         text[index] = (node.variable < static_cast<int>(variableNames.size()))
                          ? variableNames[node.variable]
                          : "x" + std::to_string(node.variable + 1);
         continue;
      }
      if (node.operation == Operation::Constant)
      {
         constant[index] = node.constant;
         text[index] = formatNumber(node.constant);
         // A negative constant (possible after tuning) needs parentheses
         // as an operand, like any other leading minus.
         precedence[index] = node.constant < 0.0 ? 1 : 3;
         continue;
      }

      const bool usesVariable = variable[node.left] || (node.right >= 0 && variable[node.right]);
      variable[index] = usesVariable ? 1 : 0;
      if (!usesVariable && foldable[node.left] && (node.right < 0 || foldable[node.right]))
      {
         double value = 0.0;
         if (apply(node.operation, constant[node.left],
                   node.right >= 0 ? constant[node.right] : 0.0, value))
         {
            constant[index] = value;
            text[index] = formatNumber(value);
            precedence[index] = value >= 0.0 ? 3 : 1;
            continue;
         }
         foldable[index] = 0;
      }
      else
      {
         foldable[index] = 0;
      }

      // Parenthesize an operand that binds more loosely than its parent, or
      // equally on the right of a non-associative "-" or "/".
      auto operand = [&](int child, int parentPrecedence, bool rightOfNonAssociative) {
         const bool wrap = precedence[child] < parentPrecedence ||
                           (rightOfNonAssociative && precedence[child] == parentPrecedence);
         return wrap ? "(" + text[child] + ")" : text[child];
      };
      switch (node.operation)
      {
      case Operation::Square:
         text[index] = operand(node.left, 3, false) + "^2";
         precedence[index] = 3;
         break;
      case Operation::Negate:
         text[index] = "-" + operand(node.left, 3, false);
         precedence[index] = 1;
         break;
      case Operation::Add:
      case Operation::Subtract:
         // "a - (-3)" reads better as "a + 3", and "a + (-3)" as "a - 3".
         if (nodes_[node.right].operation == Operation::Negate)
         {
            // "a + -b" reads better as "a - b", and "a - -b" as "a + b".
            const int inner = nodes_[node.right].left;
            text[index] = operand(node.left, 1, false) +
                          (node.operation == Operation::Add ? " - " : " + ") +
                          operand(inner, 1, node.operation == Operation::Add);
         }
         else if (foldable[node.right] && !variable[node.right] && constant[node.right] < 0.0)
         {
            text[index] = operand(node.left, 1, false) +
                          (node.operation == Operation::Add ? " - " : " + ") +
                          formatNumber(-constant[node.right]);
         }
         else
         {
            text[index] = operand(node.left, 1, false) + binarySymbol(node.operation) +
                          operand(node.right, 1, node.operation == Operation::Subtract);
         }
         precedence[index] = 1;
         break;
      case Operation::Multiply:
      case Operation::Divide:
         text[index] = operand(node.left, 2, false) + binarySymbol(node.operation) +
                       operand(node.right, 2, node.operation == Operation::Divide);
         precedence[index] = 2;
         break;
      default:
         text[index] = std::string(functionName(node.operation)) + "(" + text[node.left] + ")";
         precedence[index] = 3;
         break;
      }
   }
   return text;
}
