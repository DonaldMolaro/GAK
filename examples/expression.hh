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

class Expression
{
public:
   static Expression compile(const BaseString& genes, const InstructionSet& instructions);

   bool empty() const { return nodes_.empty(); }
   int size() const { return static_cast<int>(nodes_.size()); }
   const std::vector<ExpressionNode>& nodes() const { return nodes_; }

   // Evaluate on every row. Returns false if the formula is undefined on any
   // row (division by ~0, log or sqrt of an out-of-domain value, overflow).
   bool evaluate(const std::vector<std::vector<double> >& inputs,
                 std::vector<double>& predictions) const;

   // Indices of constant nodes, and write access to their values (used to
   // tune constants numerically after the GA has chosen the structure).
   std::vector<int> constantNodes() const;
   double constantAt(int node) const { return nodes_[node].constant; }
   void setConstant(int node, double value) { nodes_[node].constant = value; }

   // Infix text using the given variable names; variable-free subtrees are
   // folded to a single number.
   std::string toString(const std::vector<std::string>& variableNames) const;

private:
   std::vector<ExpressionNode> nodes_;
};
