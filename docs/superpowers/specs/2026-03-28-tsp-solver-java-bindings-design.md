# TSP Solver Java Bindings Design

> **For agentic workers:** This spec defines the first Java-facing API. Keep the Java layer thin, keep ownership explicit, and route all solver behavior through the stable C API.

**Goal:** Expose the solver to Java with a small JNI-based wrapper over the stable C API, including model construction, options, solve, and result access.

**Architecture:** Use a thin JNI layer to bridge Java classes to the native C API. Java owns the public object model and lifetime helpers, while the JNI code translates Java arguments into C calls, maps error codes to Java exceptions, and preserves the C handles as opaque native pointers stored in Java `long` fields. The first pass should expose the full `Model` / `Options` / `Result` surface rather than only a single convenience entry point.

**Tech Stack:** Java 8-compatible API surface, JNI, `javac` / `java`, CMake-built native libraries, existing C API.

---

## Context

The repository already has a stable C API and a Python binding that wraps it directly. The Java binding should mirror that layering choice: Java talks to JNI, JNI talks only to the C API, and the native C++ solver core remains private.

## Design Goals

- Keep the Java API small and predictable.
- Mirror the C API closely so behavior is easy to compare across languages.
- Keep ownership explicit with `AutoCloseable`.
- Avoid duplicating solver logic or model validation in Java.
- Translate native failures into actionable Java exceptions.

## Non-Goals

- No direct binding to C++ classes or STL types.
- No third-party FFI layer such as JNA in v1.
- No coordinate-based convenience builders in v1.
- No Java packaging or publishing automation in v1.

## Public API Shape

The Java package should expose:

- `tsp.solver.Model`
- `tsp.solver.Options`
- `tsp.solver.Result`
- `tsp.solver.Algorithm`
- `tsp.solver.Status`
- `tsp.solver.NativeLibrary`
- `tsp.solver.SolverNativeException`
- `tsp.solver.Solver.solve(Model, Options)` as a convenience wrapper

### Model

- `new Model()` creates an empty model.
- `int addNode()` appends a node and returns its id.
- `void setDistance(int from, int to, long distance)` stores a directed edge.
- `void validate()` checks completeness before solve.
- `Result solve(Options options)` solves the current model.
- `close()` releases the native handle.

### Options

- `new Options()` creates default options.
- `setTimeLimitMs(long value)` configures the time limit.
- `setRandomSeed(long value)` configures the stable seed option.
- `setAlgorithm(Algorithm algorithm)` selects the backend.
- `close()` releases the native handle.

### Result

- `Status getStatus()` returns the solve status.
- `long getObjective()` returns the total tour cost.
- `int getTourSize()` returns the number of nodes in the tour.
- `int[] getTour()` returns the node order.
- `close()` releases the native handle.

### Enums

- `Algorithm.DEFAULT` and `Algorithm.LOCAL_SEARCH_2OPT` mirror the C values.
- `Status` mirrors the C status enum exactly.

## Native Loading

Java should load the JNI library through a single helper, `NativeLibrary`.

Load order:

1. Explicit override via `tsp.solver.library.path` system property.
2. Explicit override via `TSP_SOLVER_LIBRARY_PATH` environment variable.
3. Standard `System.loadLibrary(...)` lookup for the JNI bridge library.

The JNI bridge library should be a dedicated Java-facing library, distinct from the core `tsp_solver` shared library. If loading fails, throw a clear `UnsatisfiedLinkError` or `SolverNativeException` with the attempted sources.

## JNI Boundary Design

- JNI code should call only the public C API declared in `include/tsp_solver/c_api.h`.
- Each Java object owns exactly one native handle stored as a `long nativeHandle` field.
- JNI helper functions should convert that handle back to the correct opaque C pointer type.
- JNI should validate `nativeHandle != 0` and throw `IllegalStateException` on use-after-close.
- JNI should centralize error translation so the mapping stays consistent.

## Error Handling

- `INVALID_ARGUMENT` maps to `IllegalArgumentException`.
- `OUT_OF_RANGE` maps to `IndexOutOfBoundsException`.
- `INVALID_MODEL` maps to `IllegalStateException`.
- `ALLOCATION_FAILED` and `INTERNAL_ERROR` map to `SolverNativeException`.
- Library-load failures should surface as `UnsatisfiedLinkError` unless a more specific wrapper adds context.

The Java layer should not expose raw native error codes to application code.

## Ownership and Lifetime

- `Model`, `Options`, and `Result` implement `AutoCloseable`.
- `close()` is idempotent.
- After `close()`, all instance methods except `close()` throw `IllegalStateException`.
- `Result` owns its native data until closed.
- `try-with-resources` should be the preferred usage style in examples and tests.

## Packaging and Layout

- Put Java sources under `bindings/java/src/main/java/`.
- Put JNI sources under `bindings/java/src/main/cpp/`.
- Put Java tests under `bindings/java/src/test/java/`.
- Keep the Java package small and focused on the public API and library loading.

The first pass should not add Gradle or Maven. CMake remains the top-level build entry point and should drive Java compilation and test execution with `javac` and `java`. The Java layer should consume the already-built native library rather than reimplementing native build logic.

## Testing Strategy

- Load test: the library loads and version-compatible classes initialize.
- Happy-path test: build a small complete graph, solve it, and inspect status, objective, and tour.
- Validation test: incomplete models raise the expected Java exception.
- Range test: invalid node indices raise `IndexOutOfBoundsException`.
- Lifecycle test: `close()` is idempotent and use-after-close throws `IllegalStateException`.

The first pass should keep tests deterministic and small, matching the same graph shape used in native and Python tests. Tests should run without third-party Java test frameworks, using a small Java test runner class invoked from CTest.

## Implementation Boundaries

- Java should not inspect or depend on native C++ internals.
- JNI should stay thin enough that future API changes are driven by the C API, not by Java-specific native logic.
- The Java surface should intentionally resemble the Python surface where language conventions allow it.

## Rollout Plan

1. Add the Java project skeleton and native library loader.
2. Add JNI entry points for `Model`, `Options`, and `Result`.
3. Add Java tests for load, solve, errors, and lifetime.
4. Wire Java tests into the repository build and test flow.
