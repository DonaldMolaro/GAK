# Generic Vs Constraint-Aware Sudoku

This repo contains two Sudoku examples:

- [examples/sudoku.cc](/Users/donaldmolaro/src/GAK/examples/sudoku.cc)
- [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)

They solve the same fixed 9x9 puzzle, but they use very different genetic
operator strategies. Reading them together is the clearest way to understand
why the modern operator override hooks were added to `Population`.

## The Generic Sudoku Example

The generic Sudoku example keeps the problem close to the original GA engine:

- one symbolic gene per cell
- `81` total genes
- `9` symbol states
- default initialization
- default crossover
- default mutation

Its fitness function rewards:

- matching the puzzle givens
- row uniqueness
- column uniqueness
- 3x3 box uniqueness

This is a valid and useful baseline because it demonstrates that the generic GA
engine can attack Sudoku at all. It also keeps the example simple.

### The main limitation

The generic operators do not know anything about Sudoku structure.

That means:

- givens can be overwritten by mutation
- crossover can immediately destroy promising substructure
- rows that were temporarily good can become invalid again for free
- much of the search budget goes into repairing clearly bad boards

In other words, the fitness function is carrying too much responsibility.

## The Constraint-Aware Sudoku Example

The constraint-aware example uses the same overall `Population` engine, but it
overrides the places where generic operators are a bad fit for the domain.

It customizes:

- `createInitialChromosome()`
- `mateChromosomes(...)`
- `mutateChromosome(...)`

### Representation strategy

The board is still represented as `81` symbolic cells, but the operators treat
the rows as structured units.

Important invariants:

- givens are always copied in place
- each row is initialized as a valid permutation of the missing row digits
- mutation swaps only mutable cells within a row
- crossover copies whole rows from parents

This means the search starts from a much better part of the space and stays
there.

## Why This Works Better

The constraint-aware example moves Sudoku rules out of "fitness-only repair"
and into the representation plus operator design.

That gives the GA three big advantages:

1. It wastes less time producing obviously invalid boards.
2. Good partial structure survives longer.
3. Fitness can spend more of its signal on what still matters: column and box
   quality, plus given consistency.

The row constraint is no longer something the algorithm has to rediscover after
every mutation.

## Practical Comparison

### Generic Sudoku is best for:

- demonstrating the baseline engine
- showing how far a symbolic chromosome plus scoring can go
- comparing naive and specialized GA design on the same problem

### Constraint-aware Sudoku is best for:

- showing the value of custom operators
- solving problems with hard structural invariants
- acting as a template for future constrained examples

If you are building new examples for:

- Latin squares with locked cells
- timetabling
- permutation search
- routing with feasibility rules

the constraint-aware Sudoku example is the better starting point.

## Design Lessons For The Rest Of The Project

This comparison led directly to the modern extension points in
[src/population.hh](/Users/donaldmolaro/src/GAK/src/population.hh):

- `createInitialChromosome()`
- `mateChromosomes(...)`
- `mutateChromosome(...)`

Those hooks keep the main GA loop generic while letting a domain-specific
subclass preserve invariants that the library should not try to infer.

That is the intended modernization direction for this codebase:

- keep the engine reusable
- keep the problem representations explicit
- let specialized examples own their structure-aware operators

## Which Sudoku Example Should You Study First

Read [examples/sudoku.cc](/Users/donaldmolaro/src/GAK/examples/sudoku.cc) first
if you want to understand the baseline encoding and fitness approach.

Read [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)
first if you want to understand how to build strong constrained examples in the
modernized library.
