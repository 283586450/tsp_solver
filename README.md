# TSP Solver

Portable C++ framework for solving the traveling salesman problem.

This repository is organized for long-term extension:
- a native C++ core for problem models and solver logic
- algorithm modules that can grow from local search into richer heuristics
- thin Python and Java bindings built on top of a stable native API

## Project layout

- `include/` - public headers for the native API
- `src/` - core implementations and algorithm code
- `tests/` - native tests
- `bindings/` - future Python and Java integration layers
- `docs/` - workflow notes, branch protection guidance, and planning docs

## Current state

The repository currently provides:
- a CMake-based native build
- a first local-search solver implementation
- a minimal test target
- GitHub Actions CI and release workflows

## C API

The first public native boundary is available in `include/tsp_solver/c_api.h`.
The library is configured to build as a shared library by default, with a stable
C install package and exported target (`tsp_solver::tsp_solver`). For a short
usage example and result-handling notes, see `docs/c_api.md`. Consumers can use
`find_package(tsp_solver CONFIG REQUIRED)` and link `tsp_solver::tsp_solver`.

## Python bindings

The Python package lives under `bindings/python/tsp_solver` and wraps the C API.
For local development, point `TSP_SOLVER_LIBRARY_PATH` at the built shared library
and add `bindings/python` to `PYTHONPATH` before running:

```bash
python -m unittest discover -s tests/python -p "test_*.py" -v
```

## Java bindings

The Java bindings live under `bindings/java/` and are built through CMake as a
thin JNI layer over the C API. Run the Java binding tests with:

```bash
ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_java_tests
```

For local development, point `TSP_SOLVER_JAVA_LIBRARY_PATH` at the built JNI
bridge or pass `-Dtsp.solver.library.path=/absolute/path/to/tsp_solver_java_jni`.

## Build and test

```bash
cmake --list-presets
cmake --preset <your-host-appropriate-preset>
cmake --build --preset <your-host-appropriate-preset>
ctest --preset <your-host-appropriate-preset> --output-on-failure
```

Run a single test with:

```bash
ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_tests
```

## Workflow notes

- Branch protection recommendations live in `docs/branch-protection.md`.
- CI is defined in `.github/workflows/ci.yml`.
- Release packaging is defined in `.github/workflows/release.yml`.
- Formatting is defined in `.clang-format`.
- Static analysis defaults live in `.clang-tidy`.

## Extension roadmap

- expand local search with more neighborhood moves and better seeding
- add shared solver abstractions for future metaheuristics
- expose a stable C API for Python and Java bindings
- package native artifacts for release distribution
