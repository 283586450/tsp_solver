# Examples Directory Design

## Goal

Add a top-level `examples/` tree that shows how to use the solver library from C++, Python, and Java against the same 20-city TSP instance and the same algorithm comparison flow.

## Scope

- Provide one shared example dataset that all three languages read.
- Provide one runnable example per language.
- Show the same algorithm comparison flow in all three languages.
- Make the examples easy to find from a single entry-point README.

## Non-Goals

- Adding new solver algorithms.
- Adding new bindings or changing the public API.
- Building a tutorial site or visual docs.
- Shipping the examples as release artifacts.

## Design

### Directory Layout

Create a single `examples/` tree with a shared dataset and per-language subdirectories:

- `examples/README.md` - the main entry point and run instructions.
- `examples/data/20_city_distance_matrix.txt` - the shared 20-city distance matrix.
- `examples/cpp/` - one C++ example program plus local build wiring.
- `examples/python/` - one Python example script.
- `examples/java/` - one Java example program.

The dataset stays in plain text so C++, Python, and Java can all parse it without extra libraries.

### Shared Scenario

Use one fixed 20-city instance for every language. Each example should:

1. load the same matrix;
2. build the same model;
3. run the same three algorithms: `TSP_SOLVER_ALGORITHM_DEFAULT`, `TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR`, and `TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT`;
4. print the objective value and tour for each run.

This makes the examples useful both as usage samples and as a cross-language comparison point.

### C++ Example

The C++ sample should be a small command-line program built through CMake. It reads the shared matrix, constructs a model with the native API, executes the three algorithms, and prints a short comparison table.

### Python Example

The Python sample should be a single script that can run from the source tree with the existing Python binding setup. It reads the shared matrix, constructs a model through the Python API, runs the same three algorithms, and prints the same comparison table format as the C++ sample.

### Java Example

The Java sample should be a small command-line program that runs against the existing Java binding setup. It reads the shared matrix, constructs a model through the Java API, runs the same three algorithms, and prints the same comparison table format as the C++ and Python samples.

### Documentation and Discovery

`examples/README.md` should explain:

- what the shared dataset is;
- which algorithms are demonstrated;
- how to run each language example;
- what output to expect;
- which environment variables or library paths are needed for Python and Java.

The top-level `README.md` should link to `examples/README.md` so the examples are easy to discover.

### Error Handling

Each sample should fail fast with a clear message if:

- the shared matrix file is missing;
- the matrix cannot be parsed;
- the solver returns a non-success status;
- the binding library cannot be loaded.

The samples should not try to recover from malformed input or partial parse errors.

### Testing

Add simple smoke coverage for each sample:

- C++: compile and run the example against the shared matrix.
- Python: run the script with the existing Python binding environment.
- Java: run the example main class with the existing Java binding environment.

Tests should confirm that all three algorithm labels appear in the output and that the examples complete on the shared 20-city dataset.

## Risks

- Keeping the shared matrix format simple enough for all three languages without adding parsing dependencies.
- Avoiding duplicate parsing logic that drifts between languages.
- Keeping the Java example lightweight enough to fit the repository's existing build style.

## Acceptance Criteria

- `examples/README.md` exists and points to all three language examples.
- The same 20-city matrix is used by the C++, Python, and Java examples.
- Each language example demonstrates the same three algorithms and prints comparable results.
- The top-level README links to the examples entry point.
