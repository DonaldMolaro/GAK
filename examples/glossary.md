# Glossary

## Chromosome

A candidate solution. In this repo, a chromosome is a string of genes.

## Gene

One position in a chromosome. Its meaning depends on the problem:

- one item selection bit
- one row choice
- one board cell value

## Chromosome Length

How many genes a candidate contains.

## Base States

How many distinct values each gene can represent.

- `2` means binary
- larger values mean symbolic genes

## Fitness

The score the GA uses to compare candidates.

## Population

The set of candidates currently being evolved.

## Mutation

A random change to a candidate.

## Crossover

A recombination step that produces children from two parents.

## Constraint-Aware Operator

A mutation or crossover rule designed to preserve structure that the generic
operators do not understand.

## Partial Progress

A score that distinguishes “closer to solved” from “farther from solved” even
when the candidate is not fully valid yet.

## Exact-Solution Early Stop

A hook that lets a problem stop the run as soon as a known exact solution is
found.
