# Built-In Crossover Modes

The built-in chromosome crossover strategies live in
[src/chromosome.cc](/Users/donaldmolaro/src/GAK/src/chromosome.cc) and are
selected through `Chromosome::CrossOverType` in
[src/chromosome.hh](/Users/donaldmolaro/src/GAK/src/chromosome.hh).

Available built-in modes:

- `SinglePoint`
- `TwoPoint`
- `Uniform`

These are generic operators. They work well for many toy problems and some
practical problems, but they do not understand domain invariants. When a
problem needs structure-preserving behavior, prefer overriding
`Population::mateChromosomes(...)`.

## SinglePoint

Single-point crossover chooses one split location in each parent and swaps the
trailing sections.

For fixed-length chromosomes:

- both parents split at the same gene index
- the first child gets the father's prefix and the mother's suffix
- the second child gets the mother's prefix and the father's suffix

For variable-length chromosomes:

- each parent gets its own crossover point
- child lengths can change based on the chosen split points

This is the classic crossover mode and usually the easiest one to reason about.

## TwoPoint

The built-in two-point crossover is implemented as two single-point crossovers
in sequence.

Conceptually:

1. perform one single-point crossover
2. cross the intermediate children again

This gives a more mixed result than a single split while still keeping the code
path close to the original implementation style.

## Uniform

Uniform crossover decides each gene position independently.

For fixed-length chromosomes:

- each child gene is chosen from either parent with equal probability
- both children stay the same length as the parents

For variable-length chromosomes:

- genes are chosen position-by-position until the shorter parent ends
- any remaining tail from the longer parent is copied into the corresponding
  child

This mode gives the most local mixing, but it also destroys larger contiguous
structure more aggressively than `SinglePoint` or `TwoPoint`.

## Which Mode To Use

As a rough rule:

- `SinglePoint` is a good default for simple fixed-length experiments
- `TwoPoint` is useful when you want more mixing without going fully uniform
- `Uniform` is useful when each position is relatively independent

Avoid relying on the built-in modes when:

- rows, routes, or schedules need to stay valid by construction
- some genes must never change
- local structure matters more than individual gene independence

Those are usually signs you want a problem-specific mating override instead.

## Relationship To Constraint-Aware Examples

The built-in modes are intentionally generic. They are a good fit for examples
such as:

- `D`
- `F6`
- `K`
- some symbolic search problems

They are a poor fit for highly constrained problems such as Sudoku, which is
why [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)
implements row-wise crossover through `Population::mateChromosomes(...)`
instead of relying on `Chromosome::Mate(...)`.
