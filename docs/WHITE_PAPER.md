# GAK White Paper

## A Modern, Lightweight Genetic Algorithm Framework In C++14

## Abstract

`GAK` is a small, portable genetic algorithm framework written in standard
C++14. It is designed for engineers, researchers, and advanced students who
want a compact GA engine that is easy to understand, extend, and embed into
problem-specific code without bringing in a large dependency stack.

The framework is built around a composition-based model:

- `Population` is the search engine
- `PopulationProblem` describes a problem domain
- `Population::Settings` defines runtime behavior

This paper describes the motivation behind `GAK`, the design of its core
engine, the representation and operator model it uses, the kinds of problems
it addresses well, and the architectural lessons demonstrated by its example
suite, especially the contrast between generic and constraint-aware operators.

## 1. Motivation

Many genetic algorithm libraries fall into one of two camps:

- research code that proves an idea but is hard to reuse
- large frameworks that are powerful but heavy for small engineering tasks

`GAK` aims for a middle ground. It is intentionally:

- small enough to read end to end
- modern enough to be safe and portable
- flexible enough to handle both generic and structure-aware search
- practical enough to use as a teaching platform

The project is especially suited to users who want to answer questions such as:

- Can this engineering search problem be encoded as a chromosome?
- Is a basic GA enough, or do I need custom operators?
- How much does representation matter relative to parameter tuning?
- How do I build a reproducible GA experiment without a large framework?

## 2. Design Goals

The modernized `GAK` codebase is organized around a small number of explicit
goals.

### 2.1 Minimal operational footprint

The framework uses only the C++ standard library. There are no runtime
dependencies beyond a modern C++14 compiler and standard build tools.

### 2.2 Clear engine/problem separation

The GA engine should not need to know what problem it is solving. Problem
definitions provide fitness, candidate formatting, and optional custom
operators through a narrow interface.

### 2.3 Support for both generic and constrained search

Many toy GA problems work well with generic mutation and crossover. Many real
problems do not. The system must support both:

- generic gene-level search
- custom initialization, mating, and mutation for constrained domains

### 2.4 Reproducibility

Search algorithms are difficult to reason about when randomness is implicit or
uncontrolled. `GAK` treats seed control as a first-class feature so runs can
be repeated and compared meaningfully.

### 2.5 Teachability

The codebase doubles as an instructional platform. The examples are not merely
smoke tests; they are case studies in encoding, fitness design, and operator
selection.

## 3. Architectural Overview

At the highest level, `GAK` consists of three layers.

### 3.1 Core engine

The engine lives primarily in [src/population.hh](/Users/donaldmolaro/src/GAK/src/population.hh) and
[src/population.cc](/Users/donaldmolaro/src/GAK/src/population.cc).

Its responsibilities include:

- holding the active population
- evaluating candidate fitness
- selecting parents
- producing children
- applying mutation
- replacing candidates according to the configured policy
- stopping on trial limits or exact-solution detection
- returning structured run results

The engine does not own domain knowledge about knapsack, Sudoku, routing, or
numeric functions. It only owns the search loop.

### 3.2 Problem interface

Problem-specific behavior is supplied through `PopulationProblem`.

At minimum, a problem must define:

- `evaluateFitness(const BaseString&)`
- `printCandidate(const BaseString&, std::ostream&) const`

It may also optionally define:

- `hasReachedSolution(...)`
- `initializeCandidate(...)`
- `mateCandidates(...)`
- `mutateCandidate(...)`
- `validatePopulation(...)`

This keeps the engine generic while allowing domain-aware control when the
default operators are not sufficient.

### 3.3 Reporting and experiments

The framework supports both human-readable runs and structured experiment
output. The reporting layer can emit:

- formatted terminal summaries
- JSON run reports
- per-generation CSV reports

This makes the same engine suitable for:

- interactive demos
- instructional labs
- parameter sweeps
- reproducible experiments

## 4. Candidate Representation

The fundamental gene storage type is `BaseString`, implemented in
[src/base.hh](/Users/donaldmolaro/src/GAK/src/base.hh) and [src/base.cc](/Users/donaldmolaro/src/GAK/src/base.cc).

### 4.1 Packed symbolic representation

`BaseString` stores genes in a packed bit representation. Rather than treating
a chromosome as a vector of heap-allocated objects, it computes the number of
bits required to encode a gene with `baseStates` possible values and packs the
data tightly.

This yields several advantages:

- compact storage
- no per-gene heap allocation
- support for both binary and multi-state genes
- predictable memory layout

### 4.2 Symbolic rather than purely floating-point design

`GAK` is not designed as a real-valued evolutionary-strategy toolkit. Its core
representation is symbolic and discrete, though numeric problems can still be
addressed by decoding packed genes into coordinates or parameter values.

This design is a strength for:

- binary decision vectors
- symbolic sequences
- board layouts
- constrained combinatorial encodings

It is a limitation for problems that demand rich continuous operators directly
on floating-point vectors.

## 5. Chromosomes And Built-In Operators

`Chromosome`, in [src/chromosome.hh](/Users/donaldmolaro/src/GAK/src/chromosome.hh) and
[src/chromosome.cc](/Users/donaldmolaro/src/GAK/src/chromosome.cc), wraps a
`BaseString` and provides the built-in operator set used by generic problems.

### 5.1 Built-in mutation

The default mutation operator applies per-gene random edits according to a
configured mutation rate.

- for binary genes, mutation keeps values binary
- for symbolic genes, mutation assigns another value in range

This is appropriate when arbitrary local edits still produce meaningful
candidates.

### 5.2 Built-in crossover

The framework includes three crossover modes:

- single-point
- two-point
- uniform

These provide a useful generic baseline, but they are not assumed to be ideal
for every domain. One of the key lessons of `GAK` is that generic crossover is
often acceptable for simple problems and often actively harmful for structured
ones.

## 6. Search Loop And Population Dynamics

The `Population` engine implements a conventional generational GA loop with
configurable policies.

### 6.1 Selection policies

The framework supports:

- roulette-wheel parent selection
- random parent selection

Fitness weighting can be based on:

- direct evaluation values
- windowed fitness
- linearly normalized ranks

### 6.2 Replacement policies

Supported deletion/replacement modes include:

- replace all
- replace all but best
- replace half
- replace quarter
- replace last

This lets the engine express different exploration/exploitation balances
without changing problem code.

### 6.3 Early termination

Some problems know exactly when a solution has been found. `GAK` supports this
through `hasReachedSolution(...)`, allowing runs to stop early once the best
candidate satisfies a problem-defined success condition.

This is especially important for:

- spelling exact targets
- alphabet ordering
- N-Queens
- Sudoku

Without this hook, the engine would continue spending trials after the problem
was already solved.

## 7. Comparison With Standard Genetic Algorithms

To understand `GAK`, it helps to compare it with the “standard” genetic
algorithm model commonly presented in textbooks and introductory software.

### 7.1 What a standard GA usually looks like

A standard GA is often described as:

- a fixed-length chromosome, frequently binary
- random initial population generation
- generic crossover and mutation applied uniformly
- fitness-only problem customization
- a generational replacement loop with limited problem-specific structure

That model is useful because it is simple, widely understood, and often enough
for small demonstrations or loosely structured search problems.

### 7.2 Where GAK follows the standard model

`GAK` still preserves the classical outer loop:

- initialize a population
- evaluate fitness
- select parents
- create children
- mutate
- replace part of the population
- repeat until a stopping condition is reached

It also keeps standard GA ideas visible and configurable:

- roulette-style selection
- mutation and crossover rates
- multiple replacement policies
- symbolic or binary chromosome encodings

This means `GAK` remains recognizable as a conventional GA framework rather
than a fundamentally different search method.

### 7.3 Where GAK deliberately departs from the standard model

The key departures are architectural.

First, `GAK` treats the problem interface as more than a fitness callback.
In a standard GA, the problem often only supplies fitness. In `GAK`, a
problem can also supply:

- exact-solution detection
- custom initialization
- custom mating
- custom mutation
- population-level validation

Second, `GAK` treats representation as central rather than incidental.
Many standard GA descriptions implicitly assume that the generic chromosome and
operator model should be reused as-is. `GAK` takes the opposite lesson from
practical work: for many important problems, the encoding and operator design
matter at least as much as the outer loop.

Third, `GAK` includes explicit support for structured experiment output and
reproducibility. Standard teaching implementations often stop at terminal
output and ad hoc random seeding. `GAK` treats seed control and machine-readable
run reports as part of normal engineering practice.

### 7.4 Why this difference matters

For lightly structured problems such as knapsack or simple symbolic search,
the standard GA model is often perfectly adequate. In those cases, `GAK`
behaves much like a conventional GA library.

For highly structured problems, however, a standard GA often wastes effort by:

- mutating locked or semantically fixed positions
- crossing over incompatible structure
- generating large numbers of equally bad candidates
- continuing long after an exact solution is already known

`GAK` is designed to handle those cases without abandoning the GA framework
itself. The engine stays conventional, but the problem interface is allowed to
carry more of the domain structure.

### 7.5 Practical interpretation

The simplest way to read `GAK` is:

- it is standard GA machinery at the outer loop
- it is non-standard in how seriously it treats representation, constraints,
  and problem-aware operators

That combination is intentional. It preserves the explanatory clarity of a
classical GA while making the system more useful for real symbolic and
constraint-heavy problems.

## 8. Generic Operators Versus Constraint-Aware Operators

This is the most important architectural idea in the system.

### 8.1 Generic operator model

The generic model assumes:

- random initialization is acceptable
- arbitrary gene mutation is acceptable
- generic crossover preserves enough useful structure

This works well for:

- knapsack
- alphabet ordering
- small symbolic or numeric benchmark problems

### 8.2 Constraint-aware model

Structured problems often violate those assumptions. In these domains, generic
operators spend much of their effort destroying structure that the search then
has to rediscover.

Examples include:

- Sudoku with fixed givens
- permutation-like problems
- routing and sequencing problems
- partially locked board problems

For these, `GAK` allows the problem to override initialization, mating, and
mutation directly.

### 8.3 Sudoku as a central case study

The repo intentionally contains both:

- generic Sudoku
- constraint-aware Sudoku

The generic version demonstrates how far a symbolic GA can go with fitness
alone. The constraint-aware version demonstrates a more important lesson:

representation and operator design often matter more than parameter tuning.

In the constrained version:

- givens remain locked
- rows are initialized with valid local structure
- mutation preserves row-level invariants
- crossover exchanges larger structured units

This reduces wasted search and produces better behavior under the same general
GA framework.

## 9. Problem Classes Addressed By GAK

`GAK` is not a universal optimizer. It is best understood as a compact engine
for classes of problems that fit its representation and operator model.

### 9.1 Strong fits

The framework is especially well suited to:

- binary selection problems such as knapsack
- symbolic sequence problems such as spelling and ordering
- board and constraint problems such as N-Queens and Latin squares
- black-box benchmark optimization with discrete encodings

### 9.2 Problems that usually need custom operators

The framework is also useful for harder constrained problems when custom
operators are supplied, especially:

- Sudoku-like board problems
- sequencing or route problems with uniqueness constraints
- tasks with partially fixed structure

### 9.3 Weak fits

The framework is a weaker fit for:

- problems with no meaningful partial fitness
- domains dominated by exact algorithms such as dynamic programming or linear
  programming
- problems requiring sophisticated continuous operators over real-valued
  vectors

## 10. Reproducibility, Testing, And Portability

The project emphasizes engineering discipline rather than treating stochastic
search as inherently untestable.

### 10.1 Seed control

Runs can be fixed-seed and reported with their active seed, making both
debugging and experimental comparison much easier.

### 10.2 Test coverage

The repository includes:

- library-focused unit tests
- example-target tests
- coverage targets
- course and lab smoke checks

This is important because GA code can appear to “work” even when subtle
selection or evaluation bugs exist.

### 10.3 Portability

The framework is intentionally dependency-light and has been validated on:

- macOS with Apple Clang
- Fedora Linux on `aarch64`

That portability is a direct consequence of the codebase’s narrow dependency
surface and standard-library-only design.

## 11. Educational Value

One of the unusual strengths of `GAK` is that it is not only a library. It is
also a guided teaching environment.

The repository includes:

- runnable demos
- example tests
- a self-study course
- guided labs
- experiment scripts

This makes the project useful in several modes:

- as a compact GA library
- as a teaching repo for engineers learning evolutionary search
- as a sandbox for comparing encodings and operators

The examples are intentionally diverse enough to show that “genetic algorithm”
is not one technique but a family of design choices around:

- representation
- fitness
- variation
- selection
- stopping criteria

## 12. Current Limitations

`GAK` is intentionally small, and that brings tradeoffs. The most important
limitations fall into three categories.

### 12.1 Technical limits

- The core representation strongly favors discrete and symbolic encodings over
  rich continuous vector operators.
- The project targets C++14 and a lightweight `make` workflow rather than a
  more modern packaging or build-system story.
- The experiment tooling is useful and reproducible, but it is still small
  compared with large research platforms.

### 12.2 Practical limits

- The example suite is strong, but still narrower than a full benchmarking
  corpus.
- The project does not yet present formal comparative benchmarks against other
  GA frameworks or against domain-specific solvers.
- The install/use story is still repository-oriented rather than package-first.

### 12.3 Intentional limits

- `GAK` is designed to stay readable and teachable rather than maximizing
  feature breadth.
- It is not trying to be a giant evolutionary-computation framework.
- It is not trying to be a specialized continuous optimizer first.
- It is not trying to replace exact algorithms where those algorithms are the
  better fit.

These limits are not merely missing features. They define the project’s
current position: a compact, explicit, reproducible GA toolkit for symbolic,
discrete, and structure-aware search.

## 13. Roadmap

The future of `GAK` is best understood as a staged roadmap rather than an
unbounded feature list.

### 13.1 Near-term priorities

These items improve the project immediately without changing its identity.

- Expand the example families:
  graph coloring and timetabling are now present, and the next logical
  additions are set cover, scheduling variants, and portfolio-style constrained
  selection.
- Improve experiment workflows:
  build higher-level comparison and summarization on top of the existing JSON,
  CSV, and sweep tools.
- Deepen the instructional track:
  keep extending the course and labs so the examples become a more complete
  onboarding path for engineers learning GA design.

### 13.2 Mid-term priorities

These items improve external usability and engineering maturity.

- Strengthen the packaging and install story for outside consumers.
- Expand CI and platform validation across more compilers and systems.
- Add clearer benchmarking and experiment-comparison guidance for users who
  want to tune or compare configurations systematically.

### 13.3 Long-term priorities

These items would broaden the project’s reach while preserving the core
architecture.

- Consider optional continuous-optimization extensions where they fit the
  system cleanly.
- Consider a more polished external embedding and library-consumption path.
- Publish more formal comparisons between generic operators, custom operators,
  and problem-specific search structure across the example families.

### 13.4 Why this roadmap matters

Each roadmap area serves a different project goal:

- More examples improve both library usefulness and teaching value.
- Better experiment tooling improves research quality and reproducibility.
- Better packaging improves external adoption.
- Broader validation improves confidence in portability and maintenance.

### 13.5 Summary roadmap

| Priority | Area | Why It Matters | Likely Impact |
|---|---|---|---|
| Near-term | More example families | Expands demonstrated problem fit | Higher educational and practical value |
| Near-term | Better experiment tooling | Makes tuning and comparison easier | Better empirical workflows |
| Near-term | Deeper course material | Improves onboarding and teachability | Faster adoption by new users |
| Mid-term | Packaging and install story | Makes the library easier to consume externally | Better external usability |
| Mid-term | Broader CI and validation | Increases confidence across environments | Better maintenance posture |
| Long-term | Continuous-search extensions | Broadens addressable problem classes | Larger problem envelope |
| Long-term | Formal benchmarking | Clarifies where `GAK` is strongest | Stronger positioning and evidence |

## 14. Conclusion

`GAK` demonstrates that a genetic algorithm framework does not need to be
large to be useful. A clean separation between engine and problem, a compact
symbolic representation, and explicit hooks for constraint-aware operators are
enough to support a meaningful range of optimization and constraint problems.

Its most important lesson is architectural rather than algorithmic:

the quality of a GA system depends at least as much on representation and
operator design as it does on the outer search loop.

In that sense, `GAK` is both a usable library and an argument for a style of
genetic algorithm engineering: small, explicit, reproducible, and grounded in
problem structure.

## References In This Repo

- [README.md](/Users/donaldmolaro/src/GAK/README.md)
- [docs/PROBLEM_FIT.md](/Users/donaldmolaro/src/GAK/docs/PROBLEM_FIT.md)
- [docs/WRITING_EXAMPLES.md](/Users/donaldmolaro/src/GAK/docs/WRITING_EXAMPLES.md)
- [docs/SUDOKU_DESIGN.md](/Users/donaldmolaro/src/GAK/docs/SUDOKU_DESIGN.md)
- [docs/CROSSOVER_MODES.md](/Users/donaldmolaro/src/GAK/docs/CROSSOVER_MODES.md)
- [examples/course.md](/Users/donaldmolaro/src/GAK/examples/course.md)
