# Visual Walkthroughs

This file is a simple map of how to think about the major example categories in
the repo.

## Binary Decision Problems

Example:

- `K` (Knapsack)

Mental model:

- one gene controls one yes/no choice
- fitness rewards good combinations
- penalties discourage infeasible ones

## Symbolic Search Problems

Examples:

- `A` (Alphabet ordering)
- `S` (Spell)

Mental model:

- one gene stores one symbol
- success depends on the right sequence, not just the right count of values

## Numeric Black-Box Optimization

Examples:

- `D`
- `F6`

Mental model:

- chromosomes encode candidate coordinates
- the GA only sees score, not gradients or formulas

## Constraint Satisfaction

Examples:

- `Q`
- `L`

Mental model:

- candidates are often imperfect
- the score must still tell the GA what “better” looks like

## Structure-Preserving Search

Examples:

- `U`
- `C`

Mental model:

- the representation is not enough on its own
- operator design decides whether useful structure survives from generation to generation

## Best Comparison To Study

If you only study one pair, compare:

- `U` generic Sudoku
- `C` constraint-aware Sudoku

That pair teaches the most important lesson in the repo:

representation and operators often matter more than parameter tuning.
