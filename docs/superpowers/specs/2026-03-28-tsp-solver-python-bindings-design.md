# TSP Solver Python Bindings Design

> **For agentic workers:** This spec defines the first Python-facing API. Keep it thin, keep it in sync with the C API, and do not duplicate solver logic in Python.

**Goal:** Expose the solver to Python with a small, idiomatic wrapper over the stable C API, including model construction, options, solve, and result access.

**Architecture:** Use pure Python with `ctypes` against the native shared library. Python owns no solver logic; it only loads the library, maps error codes to exceptions, and presents an object-oriented facade over the C handles. The first pass should expose the full model API rather than a single convenience function so Python users can build and inspect graphs the same way C consumers do.

**Tech Stack:** Python 3, `ctypes`, CMake-built shared library, stdlib `unittest` for tests.

---

## Context

The native core already exposes a stable C API with opaque model/options/result handles. Python bindings should stay as a thin translation layer so the ABI boundary remains the C API, not Python-specific C++ bindings or generated glue.

## Design Goals

- Keep the Python surface small and predictable.
- Mirror the C API closely so behavior is easy to reason about.
- Avoid extra compiled dependencies in the first pass.
- Make resource ownership explicit and safe.
- Translate native failures into clear Python exceptions.

## Non-Goals

- No `pybind11` or CPython extension in v1.
- No coordinate-specific helper API in v1.
- No solver heuristics implemented in Python.
- No wheel publishing or packaging automation in v1.

## Public API Shape

The Python package should expose:

- `tsp_solver.Model`
- `tsp_solver.Options`
- `tsp_solver.Result`
- `tsp_solver.Algorithm`
- `tsp_solver.Status`
- `tsp_solver.SolverError` and derived exceptions
- `tsp_solver.solve(model, options=None)` as a convenience wrapper

### Model

- `Model()` creates an empty model.
- `add_node() -> int` appends a node and returns its id.
- `set_distance(from_id, to_id, distance) -> None` stores a directed edge.
- `validate() -> None` checks completeness before solve.
- `solve(options=None) -> Result` solves the current model.

### Options

- `Options()` creates default options.
- `time_limit_ms` default matches the C API behavior.
- `random_seed` is optional and defaults to `0`.
- `algorithm` is an `Algorithm` enum.

### Result

- `status` returns a `Status` enum.
- `objective` returns the total tour cost.
- `tour_size` returns the number of nodes in the tour.
- `tour` returns the node order as a tuple of ints.

## Library Loading

The Python package should resolve the native shared library lazily on first use.

Search order:

1. Explicit override via `TSP_SOLVER_LIBRARY_PATH`.
2. Library placed alongside the Python package during local development.
3. System lookup via `ctypes.util.find_library`.

If loading fails, raise a clear exception that names the expected library and the attempted locations.

## Error Handling

- Native error codes map to Python exceptions.
- `INVALID_ARGUMENT` maps to `ValueError`.
- `OUT_OF_RANGE` maps to `IndexError`.
- `INVALID_MODEL` maps to `ValueError`.
- `ALLOCATION_FAILED` and `INTERNAL_ERROR` map to `RuntimeError`.
- Destroy/close methods must be idempotent.

The wrapper should raise exceptions immediately instead of returning error codes to Python callers.

## Ownership and Lifetime

- Each Python object owns exactly one native handle.
- `close()` explicitly releases the handle.
- `__enter__` / `__exit__` should support context-manager use.
- `__del__` should be a best-effort fallback only.
- `Result` objects should remain valid until closed or garbage-collected.

## Packaging and Layout

- Put the package under `bindings/python/tsp_solver/`.
- Keep the public API in `__init__.py` and the native loader in a private helper module.
- Keep tests alongside the package or under `tests/python/`, whichever best matches the repository layout after implementation.

## Testing Strategy

- Import test: the package loads and exposes the public symbols.
- Happy-path test: build a small complete model, solve it, and inspect `status`, `objective`, and `tour`.
- Validation test: incomplete models raise the correct exception.
- Argument test: null-equivalent and out-of-range inputs raise the correct exception type.
- Lifecycle test: explicit `close()` and context-manager use do not leak handles.

Use `unittest` for the first pass so the Python side does not depend on extra test packages.

## Implementation Boundaries

- Python should call only the public C API.
- No Python code should depend on C++ headers or exported C++ classes.
- The binding layer should stay small enough that future Java bindings can mirror its shape.

## Rollout Plan

1. Add the Python package skeleton and native loader.
2. Wrap `Model`, `Options`, and `Result` handles.
3. Add tests for import, solve, and errors.
4. Wire the package into the CMake build and test flow.
