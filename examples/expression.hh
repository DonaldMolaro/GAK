#pragma once

// Linear (postfix) encoding of formulas for symbolic regression.
//
// A chromosome is a sequence of instruction genes read left to right by a
// stack machine: variables and constants push a value, unary operators
// replace the top value, binary operators combine the top two. Any
// instruction that would underflow the stack is skipped, so EVERY gene
// string decodes to a valid formula and the generic crossover and mutation
// operators never produce an unusable candidate. The formula is whatever is
// left on top of the stack; instructions that do not feed it are ignored.

#include <cstddef>
#include <string>
#include <vector>

class BaseString;

enum class Operation
{
   Variable, Constant,
   Add, Subtract, Multiply, Divide,
   Sin, Cos, Exp, Log, Sqrt, Square, Negate
};

struct Instruction
{
   Operation operation;
   int variable = -1;     // input column, for Operation::Variable
   double constant = 0.0; // value, for Operation::Constant
};

// The gene alphabet: gene value v means instructions()[v].
class InstructionSet
{
public:
   // Each input variable appears `variableCopies` times so formulas are not
   // dominated by constants when there are few inputs.
   explicit InstructionSet(int inputCount, int variableCopies = 3);

   int size() const { return static_cast<int>(instructions_.size()); }
   int inputCount() const { return inputCount_; }
   const Instruction& at(int gene) const { return instructions_[gene]; }

private:
   int inputCount_;
   std::vector<Instruction> instructions_;
};

// A decoded formula: only the nodes that feed the result, children before
// parents, with the root last.
struct ExpressionNode
{
   Operation operation;
   int variable = -1;
   double constant = 0.0;
   int left = -1;   // child node index (unary operand, or binary left)
   int right = -1;  // binary right operand
};

// Input rows stored one column per input, the layout the evaluator reads.
// Build it once per data set and reuse it.
struct InputColumns
{
   std::size_t rows = 0;
   std::vector<std::vector<double> > columns; // columns[input][row]

   InputColumns() = default;
   explicit InputColumns(const std::vector<std::vector<double> >& rowMajor);
};

class Expression
{
public:
   // Any value whose magnitude exceeds this counts as overflow, which makes
   // the formula undefined.
   static constexpr double kMaximumMagnitude = 1e12;

   static Expression compile(const BaseString& genes, const InstructionSet& instructions);

   bool empty() const { return nodes_.empty(); }
   int size() const { return static_cast<int>(nodes_.size()); }
   const std::vector<ExpressionNode>& nodes() const { return nodes_; }

   // Evaluate on every row. Returns false if the formula is undefined on any
   // row (division by ~0, log or sqrt of an out-of-domain value, overflow).
   bool evaluate(const std::vector<std::vector<double> >& inputs,
                 std::vector<double>& predictions) const;
   // Evaluate on every row and return the values of the given nodes,
   // values[i][row] for nodes[i]. Same failure rule as evaluate().
   bool evaluateNodes(const std::vector<std::vector<double> >& inputs,
                      const std::vector<int>& nodes,
                      std::vector<std::vector<double> >& values) const;
   // As evaluateNodes, on column-major inputs and without copying: values[i]
   // points at nodes[i]'s values in a per-thread buffer, valid until the next
   // evaluation on this thread. With no nodes it only checks the formula is
   // defined. This is the fast path for repeated fitting.
   bool evaluateNodes(const InputColumns& inputs,
                      const std::vector<int>& nodes,
                      std::vector<const double*>& values) const;

   // Evaluate every node into `values` (node i at values[i * rows]), a
   // buffer the caller keeps; false if the formula is undefined on any row.
   bool evaluateInto(const InputColumns& inputs, std::vector<double>& values) const;
   // Recompute just `nodes` (ascending, each after its children) in a buffer
   // filled by evaluateInto, e.g. after a constant changed. False if any of
   // them is undefined; the buffer is then partly updated.
   bool reevaluate(const InputColumns& inputs, const std::vector<int>& nodes,
                   std::vector<double>& values) const;
   // `node` and every node above it up to the root, in ascending order: the
   // nodes whose values change when `node` does.
   std::vector<int> nodeAndAncestors(int node) const;

   // The top-level additive terms: the nodes reached from the root through
   // +, - and negation, e.g. x*y + sin(x) - 3 gives x*y, sin(x) and 3. Their
   // signs are dropped (each term gets its own fitted coefficient). Returns
   // just the root when there would be more than `maxTerms` terms.
   std::vector<int> additiveTerms(int maxTerms) const;
   // Whether the subtree at `node` depends on any input variable.
   bool usesVariable(int node) const;
   // The input variables the formula uses, in ascending order.
   std::vector<int> variablesUsed() const;
   // The input variables the subtree at `node` uses, in ascending order.
   std::vector<int> variablesUsedBy(int node) const;

   // Indices of constant nodes, and write access to their values (used to
   // tune constants numerically after the GA has chosen the structure).
   std::vector<int> constantNodes() const;
   double constantAt(int node) const { return nodes_[node].constant; }
   void setConstant(int node, double value) { nodes_[node].constant = value; }

   // Infix text using the given variable names; variable-free subtrees are
   // folded to a single number.
   std::string toString(const std::vector<std::string>& variableNames) const;
   // The same text for the subtree at `node`.
   std::string nodeText(const std::vector<std::string>& variableNames, int node) const;

private:
   // Evaluate every node on every row into a per-thread buffer laid out
   // node-major (buffer[node * rows + row]); nullptr if undefined anywhere.
   const double* evaluateAll(const InputColumns& inputs) const;
   bool evaluateNode(std::size_t index, const InputColumns& inputs, double* values) const;
   std::vector<std::string> allTexts(const std::vector<std::string>& variableNames) const;

   std::vector<ExpressionNode> nodes_;
};
