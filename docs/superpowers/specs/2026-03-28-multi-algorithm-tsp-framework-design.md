# Multi-Algorithm TSP Framework Design

## Goal

Turn `tsp_solver` from a single-algorithm project into a unified framework that supports multiple TSP solving algorithms behind one consistent API for C++, C API, Python, and Java.

## Product Direction

The project goal is not merely to improve one heuristic. The project goal is to expose multiple TSP algorithms through a stable shared interface so users can choose the trade-off they need for a given problem.

The framework should support:

- exact algorithms for small instances and correctness references;
- simple constructive heuristics for fast baseline solutions;
- local search for cheap improvement;
- stronger metaheuristics for better solution quality.

## Algorithm Scope

### First-phase algorithms

- `DEFAULT`
- `EXACT_HELD_KARP`
- `GREEDY_NEAREST_NEIGHBOR`
- `GREEDY_CHEAPEST_INSERTION`
- `LOCAL_SEARCH_2OPT`
- `METAHEURISTIC_ITERATED_LOCAL_SEARCH`

### Second-phase reserved extensions

- `LOCAL_SEARCH_3OPT`
- `METAHEURISTIC_SIMULATED_ANNEALING`
- `METAHEURISTIC_TABU_SEARCH`

### Out of near-term scope

- branch-and-cut and ILP-based exact solving;
- Lin-Kernighan;
- genetic algorithms and ant colony optimization;
- Christofides unless the project later commits to metric-TSP-specific guarantees.

## API Principles

- all algorithms use the same `Problem` / `Model` and the same `solve(...)` entrypoint;
- algorithm choice lives in options, not in separate function families;
- result objects report the actual algorithm that ran;
- the public API only exposes algorithms the project actually intends to support soon;
- algorithm-private tuning parameters stay internal in the first phase.

## Native Architecture

### Shared core types

The framework keeps `Problem` and `Tour`, but introduces framework-level types for solver dispatch:

- `Algorithm`
- `SolveOptions`
- `SolveResult`
- a single solver entrypoint in a new public header

`SolveResult` should include:

- solve status;
- objective value;
- tour;
- `selected_algorithm`.

### Dispatch layer

A dedicated dispatch layer selects the algorithm implementation from `SolveOptions.algorithm`. `DEFAULT` is resolved here rather than buried inside a specific algorithm implementation or binding.

### Algorithm modules

Algorithms should move into separate implementation files instead of accumulating in the current local-search module. The design target is one focused file per algorithm plus optional shared algorithm helpers.

## Binding Strategy

The C API, Python bindings, and Java bindings remain thin projections of the native framework.

- C API extends the algorithm enum and result accessors.
- Python extends `Algorithm`, keeps `Options.algorithm`, and adds `Result.selected_algorithm`.
- Java extends `Algorithm`, keeps `Options.setAlgorithm(...)`, and adds `Result.getSelectedAlgorithm()`.

Bindings must not know algorithm internals. They only reflect the shared native contract.

## Documentation and Examples

The repository should present itself as a multi-algorithm framework.

### README

README should describe:

- supported algorithms;
- the meaning of `DEFAULT`;
- how to choose an algorithm from each language binding;
- where to find fuller algorithm notes.

### Dedicated algorithm docs

Add `docs/algorithms.md` to document:

- algorithm category;
- exact vs heuristic vs metaheuristic status;
- intended use cases;
- expected quality/speed trade-offs;
- deterministic vs randomized behavior.

### Examples

Add side-by-side algorithm comparison examples for:

- C++
- Python
- Java

Each example should solve the same problem with several algorithms and print the chosen algorithm, selected algorithm, objective, and tour size.

## Testing Strategy

The framework needs tests at multiple levels:

- API and dispatch tests;
- exact algorithm correctness on small instances;
- legality and basic quality checks for heuristic algorithms;
- binding enum/result mapping tests;
- example smoke tests where practical.

Held-Karp should double as a small-instance truth source for regression checks against heuristic outputs.

## Delivery Strategy

The project should not implement all planned algorithms at once. It should proceed in phases:

1. framework/API refactor;
2. migrate existing 2-opt into the framework;
3. add exact and constructive algorithms;
4. add iterated local search;
5. add examples and docs;
6. expand with more advanced algorithms later.

This keeps the work aligned with the real project goal: support multiple algorithms through a stable, extensible interface.
