# What Kinds Of Problems Fit GAK

`GAK` is a general-purpose genetic algorithm engine with a small but useful
set of extension points. It is a good fit for problems where:

- candidates can be represented as fixed-length or variable-length gene
  strings
- fitness can compare "better" and "worse" partial solutions
- random variation plus recombination can explore the search space usefully

It is not a universal optimizer. Some problems fit the generic operators well;
others need problem-specific initialization, crossover, and mutation.

## Strong Fits

### Binary decision problems

These are often the easiest problems for the library.

Examples:

- knapsack
- subset selection
- feature flags / on-off configuration search
- simple scheduling decisions

Why they fit:

- one bit per decision is a natural encoding
- mutation is easy to interpret
- crossover tends to preserve useful building blocks reasonably well

Reference example:

- [examples/knapsack.cc](/Users/donaldmolaro/src/GAK/examples/knapsack.cc)

### Symbolic fixed-length search

These are problems where each position has one of several symbolic values.

Examples:

- alphabet ordering
- board placements
- symbolic pattern construction
- small assignment problems
- graph coloring
- timetable slot assignment

Why they fit:

- `BaseString` supports multi-state genes directly
- the engine already handles symbolic chromosomes well
- exact solutions can often be detected cleanly

Reference examples:

- [examples/alpha.cc](/Users/donaldmolaro/src/GAK/examples/alpha.cc)
- [examples/nqueens.cc](/Users/donaldmolaro/src/GAK/examples/nqueens.cc)
- [examples/graphcoloring.cc](/Users/donaldmolaro/src/GAK/examples/graphcoloring.cc)

### Constraint satisfaction with scoreable partial progress

These are problems where a candidate can be incomplete or imperfect, but still
meaningfully ranked.

Examples:

- N-Queens
- Latin squares
- Sudoku
- timetable construction
- graph coloring

Why they fit:

- the engine can improve partial solutions over time
- exact solutions can stop the run early
- constraint-aware operators can preserve important structure

Reference examples:

- [examples/latinsquare.cc](/Users/donaldmolaro/src/GAK/examples/latinsquare.cc)
- [examples/timetable.cc](/Users/donaldmolaro/src/GAK/examples/timetable.cc)
- [examples/graphcoloring.cc](/Users/donaldmolaro/src/GAK/examples/graphcoloring.cc)
- [examples/sudoku.cc](/Users/donaldmolaro/src/GAK/examples/sudoku.cc)
- [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)

### Black-box numeric optimization

These are problems where the objective is numeric, but derivatives or closed
forms are not useful to the engine.

Examples:

- benchmark functions
- parameter tuning
- noisy objective search

Why they fit:

- the engine only needs an ordering over candidate quality
- binary encodings can work for compact demos
- symbolic encodings can work when the domain is discretized

Reference examples:

- [examples/dome.cc](/Users/donaldmolaro/src/GAK/examples/dome.cc)
- [examples/f6.cc](/Users/donaldmolaro/src/GAK/examples/f6.cc)

## Fits That Usually Need Custom Operators

These are still good candidates for `GAK`, but the generic operators are often
not enough on their own.

### Locked-cell or partially fixed board problems

Examples:

- Sudoku with givens
- Latin squares with prefilled cells
- crossword-like fill problems

Why custom operators help:

- generic mutation breaks locked positions
- structure-preserving operators waste less work

### Permutation-like problems

Examples:

- routing
- sequencing
- ordering
- assignment with uniqueness constraints

Why custom operators help:

- generic crossover often duplicates values or breaks uniqueness
- swap-based mutation and row/order-aware crossover are usually better

Reference example:

- [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)

### Problems With Exact Known Wins

Examples:

- spelling a known target
- sorting into a known order
- solving N-Queens
- solving Sudoku exactly

These are good fits because the engine can stop early through
`hasReachedSolution(...)`.

## Weak Fits

These are cases where `GAK` is usually not the best first tool.

### Problems without meaningful partial fitness

If almost every invalid candidate looks equally bad, the GA has very little
signal to work with.

Examples:

- pure yes/no constraints with no graded progress
- encodings where almost every random candidate is nonsense and equally scored

### Problems dominated by exact graph/DP/search structure

Some problems have specialized exact algorithms that are simply better.

Examples:

- shortest path on a normal graph
- classic dynamic-programming problems
- linear programming
- small exact SAT-style tasks

This does not mean GA cannot be used, but it usually should not be the default
choice.

### Problems needing rich floating-point vector operators

`GAK` is not a modern real-valued evolutionary-strategy framework. It can
address numeric problems, but it does so through the project’s gene/string
model rather than specialized continuous operators.

## Good Questions To Ask Before Choosing GAK

1. Can I encode a candidate as a gene string cleanly?
2. Can I score partial solutions meaningfully?
3. Do I need generic operators, or do I need structure-preserving ones?
4. Can the problem recognize an exact solution and stop early?
5. Is there a much simpler exact or domain-specific algorithm that should be
   used instead?

## Recommended Starting Points

- If the problem is binary and combinatorial, start from
  [examples/knapsack.cc](/Users/donaldmolaro/src/GAK/examples/knapsack.cc)
- If it is symbolic with soft constraints, start from
  [examples/nqueens.cc](/Users/donaldmolaro/src/GAK/examples/nqueens.cc)
- If it has hard structure that must be preserved, start from
  [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)
- If it is a numeric benchmark or black-box objective, start from
  [examples/dome.cc](/Users/donaldmolaro/src/GAK/examples/dome.cc) or
  [examples/f6.cc](/Users/donaldmolaro/src/GAK/examples/f6.cc)
