# Lab: Build Your First PopulationProblem

This lab walks through the design work needed to add a new problem to `GAK`
without changing the engine.

Estimated time: 45 to 75 minutes

## Goals

By the end of this lab, you should be able to:

- choose an encoding before writing fitness
- sketch a minimal `PopulationProblem`
- decide whether generic operators are enough
- identify the first tests a new example should have

## Before You Start

Read:

- [docs/WRITING_EXAMPLES.md](/Users/donaldmolaro/src/GAK/docs/WRITING_EXAMPLES.md)
- [docs/PROBLEM_FIT.md](/Users/donaldmolaro/src/GAK/docs/PROBLEM_FIT.md)

Reference examples:

- [examples/knapsack.cc](/Users/donaldmolaro/src/GAK/examples/knapsack.cc)
- [examples/nqueens.cc](/Users/donaldmolaro/src/GAK/examples/nqueens.cc)
- [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)

## Step 1: Pick A Small Problem

Choose a problem you can explain in one paragraph.

Good candidates:

- choose a subset of projects under a budget
- place pieces on a board with soft conflicts
- build a short symbolic sequence that matches a target pattern
- assign a small set of tasks to a few slots

Avoid for this lab:

- problems with no partial scoring
- problems that require a large external data model
- problems where a clean exact algorithm is obviously simpler

Write down:

1. What is the problem?
2. What is the goal?
3. What makes one candidate better than another?

## Step 2: Choose An Encoding

Answer these before writing any code:

1. What does one gene mean?
2. How many genes are there?
3. How many values can each gene take?
4. Is the chromosome fixed-length or variable-length?

Use these examples as anchors:

- `Knapsack`: one binary gene per item
- `NQueens`: one symbolic gene per column
- `LatinSquare`: one symbolic gene per cell

Write a short sentence in this form:

`One gene represents ... and the chromosome length is ...`

## Step 3: Decide Whether Generic Operators Are Enough

Ask:

1. Can arbitrary per-gene mutation still produce usable candidates?
2. Does crossover preserve anything meaningful?
3. Are there locked cells, uniqueness constraints, or permutations that should
   be preserved?

Decision rule:

- if arbitrary edits are fine, start with generic operators
- if generic edits mostly destroy validity, plan to override
  `initializeCandidate(...)`, `mateCandidates(...)`, or
  `mutateCandidate(...)`

## Step 4: Sketch The Minimal Problem Class

Start from this template:

```cpp
class MyProblem : public PopulationProblem
{
public:
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
};
```

If the problem has an exact known win, add:

```cpp
bool hasReachedSolution(const Population& population,
                        const BaseString& genes,
                        double fitness) const override;
```

If the problem needs structured operators, add:

```cpp
std::unique_ptr<Chromosome> initializeCandidate(Population& population) override;
std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
   mateCandidates(Population& population, Chromosome& mother, Chromosome& father) override;
void mutateCandidate(Population& population, Chromosome& chromosome) override;
```

## Step 5: Write The Settings First

Before worrying about implementation details, choose a plausible baseline:

```cpp
Population::Settings settings;
settings.operation = Population::OperationMode::Maximize;
settings.numberOfIndividuals = 120;
settings.numberOfTrials = 8000;
settings.chromosomeLength = /* your length */;
settings.baseStates = /* your gene state count */;
settings.bitMutationRate = 0.01;
settings.crossOverRate = 0.70;
```

Questions:

1. Why did you choose this chromosome length?
2. Why did you choose this base state count?
3. Is the problem a maximize or minimize problem?

## Step 6: Design The First Tests

Before running a long search, decide what must be true.

Minimum useful tests:

- construction works
- one or two hand-built candidates score as expected
- exact-solution detection works, if supported
- structured operators preserve their invariants, if any
- a short run completes

Examples:

- for a binary selector, test that an overweight choice is penalized
- for a board problem, test that a known good board scores better than a bad
  one
- for a constrained operator, test that it preserves locked cells or row
  validity

## Step 7: Run The Design Review On Yourself

Answer these honestly:

1. Is my encoding clean, or am I hiding repair logic in fitness?
2. Does my fitness reward partial progress?
3. Can the GA tell the difference between "bad" and "almost good"?
4. Am I using custom operators because I need them, or because the encoding is
   weak?

If your answers are shaky, revise the encoding before writing much code.

## Deliverable

At the end of this lab, you should have:

- a one-paragraph problem statement
- a one-sentence encoding summary
- a first-pass `Population::Settings` block
- a minimal class sketch
- a list of the first tests you would add

That is enough to start implementing a new example without guessing blindly.

## Optional Extension

Take your design one step further:

1. add a new markdown file describing the problem
2. identify which existing example is your closest template
3. write the exact `evaluateFitness(...)` scoring rule in prose
