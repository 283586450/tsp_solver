# TSP Solver C API Design

> **For agentic workers:** This spec defines the first public compatibility boundary for the solver. Keep the initial implementation behind opaque handles and preserve the API shape for Python and Java bindings.

**Goal:** Define a stable C API for building TSP models, configuring solver options, running a solve, and reading back the tour and objective without exposing C++ ABI details.

**Architecture:** The native C++ core remains private. The C layer exposes only opaque handles for `Model`, `Options`, and `Result`, plus a single `solve` entry point. Data flows one way: build a model, tune options, solve, then read immutable result data. Coordinate-based convenience helpers are intentionally deferred so the first API stays generic enough for both symmetric and asymmetric TSP.

**Tech Stack:** C++23 core, C11-compatible public C API, CMake, CTest, later thin Python and JNI bindings.

---

## Context

The repository already has a C++23 native core, a local-search implementation, preset-driven builds, CI, and release workflows. The next public milestone is a stable C boundary that can support Python and Java wrappers without tying them to C++ types or compiler-specific ABI behavior.

## Design Goals

- Keep the public surface small: model, options, solve, result.
- Make ownership explicit: every heap-owned object has a matching destroy function.
- Make errors explicit: API calls return status codes instead of throwing.
- Preserve future flexibility: the first implementation can be local search, but the API must not expose that assumption.
- Stay language-friendly: Python and Java wrappers should be thin translations over the C layer.

## Non-Goals

- No direct exposure of C++ classes, STL containers, or exceptions.
- No coordinate-specific `add_city` API in v1.
- No algorithm-specific result objects in v1.
- No solver object separate from `solve`; the free-function solve entry point is enough for the first version.

## Public Vocabulary

- Use `node` as the core modeling term.
- Use `tour` for the returned permutation of node ids.
- Use `objective` for the total tour cost.
- Avoid `schedule`; this is TSP, not a generic dispatch solver.

## High-Level API Shape

The public C API should look like this at a conceptual level:

```c
typedef struct tsp_solver_model tsp_solver_model_t;
typedef struct tsp_solver_options tsp_solver_options_t;
typedef struct tsp_solver_result tsp_solver_result_t;

typedef uint32_t tsp_solver_node_id_t;
typedef int64_t tsp_solver_cost_t;

typedef enum tsp_solver_error_code {
  TSP_SOLVER_ERROR_OK = 0,
  TSP_SOLVER_ERROR_INVALID_ARGUMENT,
  TSP_SOLVER_ERROR_OUT_OF_RANGE,
  TSP_SOLVER_ERROR_ALLOCATION_FAILED,
  TSP_SOLVER_ERROR_INVALID_MODEL,
  TSP_SOLVER_ERROR_INTERNAL_ERROR,
} tsp_solver_error_code_t;

typedef enum tsp_solver_status {
  TSP_SOLVER_STATUS_NOT_SOLVED = 0,
  TSP_SOLVER_STATUS_FEASIBLE,
  TSP_SOLVER_STATUS_OPTIMAL,
  TSP_SOLVER_STATUS_INFEASIBLE,
  TSP_SOLVER_STATUS_TIME_LIMIT,
  TSP_SOLVER_STATUS_INVALID_MODEL,
  TSP_SOLVER_STATUS_INTERNAL_ERROR,
} tsp_solver_status_t;

typedef enum tsp_solver_algorithm {
  TSP_SOLVER_ALGORITHM_DEFAULT = 0,
  TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT,
} tsp_solver_algorithm_t;

tsp_solver_error_code_t tsp_solver_model_create(tsp_solver_model_t** out_model);
void tsp_solver_model_destroy(tsp_solver_model_t* model);
tsp_solver_error_code_t tsp_solver_model_add_node(
    tsp_solver_model_t* model,
    tsp_solver_node_id_t* out_node_id);
tsp_solver_error_code_t tsp_solver_model_set_distance(
    tsp_solver_model_t* model,
    tsp_solver_node_id_t from,
    tsp_solver_node_id_t to,
    tsp_solver_cost_t distance);
tsp_solver_error_code_t tsp_solver_model_validate(
    const tsp_solver_model_t* model);

tsp_solver_error_code_t tsp_solver_options_create(
    tsp_solver_options_t** out_options);
void tsp_solver_options_destroy(tsp_solver_options_t* options);
tsp_solver_error_code_t tsp_solver_options_set_time_limit_ms(
    tsp_solver_options_t* options,
    uint64_t time_limit_ms);
tsp_solver_error_code_t tsp_solver_options_set_random_seed(
    tsp_solver_options_t* options,
    uint64_t random_seed);
tsp_solver_error_code_t tsp_solver_options_set_algorithm(
    tsp_solver_options_t* options,
    tsp_solver_algorithm_t algorithm);

tsp_solver_error_code_t tsp_solver_solve(
    const tsp_solver_model_t* model,
    const tsp_solver_options_t* options,
    tsp_solver_result_t** out_result);

void tsp_solver_result_destroy(tsp_solver_result_t* result);
tsp_solver_error_code_t tsp_solver_result_get_status(
    const tsp_solver_result_t* result,
    tsp_solver_status_t* out_status);
tsp_solver_error_code_t tsp_solver_result_get_objective(
    const tsp_solver_result_t* result,
    tsp_solver_cost_t* out_objective);
tsp_solver_error_code_t tsp_solver_result_get_tour_size(
    const tsp_solver_result_t* result,
    size_t* out_size);
tsp_solver_error_code_t tsp_solver_result_get_tour(
    const tsp_solver_result_t* result,
    tsp_solver_node_id_t* out_nodes,
    size_t capacity,
    size_t* out_written);
```

## API Design Decisions

### 1) `node` instead of `city`

The core API uses `node` because it stays correct for asymmetric TSP, generic complete graphs, and future specialized variants. A `city` API implies coordinate geometry and would narrow the public model too early.

### 2) `set_distance` as the primitive model operation

The first C API models a complete distance matrix. `set_distance(from, to, distance)` is the primitive operation because it works for symmetric and asymmetric problems. A later convenience layer may add coordinate-based `add_city(x, y)` or `set_symmetric_distance(...)`, but those should be wrappers, not the core contract.

### 3) `result_get_tour` as the main result accessor

The primary result accessor should copy the tour into a caller-provided buffer. That is safer for C consumers than returning an internal pointer, and it maps cleanly to Python and Java bindings. `result_get_tour_size` exists so callers can allocate the exact buffer size first.

### 4) No separate solver object in v1

The first public API does not need a `Solver` handle. A free-function `solve` keeps the surface smaller and still allows future solver selection through `Options::algorithm`.

## Model Semantics

- `tsp_solver_model_create` creates an empty model.
- `tsp_solver_model_add_node` appends a new node and returns its id.
- Node ids are contiguous and zero-based.
- A newly added node has no distances defined until they are set.
- `tsp_solver_model_set_distance` stores a directed edge weight.
- Diagonal distances (`i == j`) are either fixed to zero or ignored by validation, but the solver must treat them as zero cost.
- `tsp_solver_model_validate` checks that every ordered pair of distinct nodes has a defined distance and that the model is otherwise internally consistent.

## Options Semantics

- `time_limit_ms` is the primary runtime cap.
- `random_seed` makes randomized heuristics reproducible.
- `algorithm` selects the solve backend, with `DEFAULT` choosing the best available implementation.
- More specialized knobs can be added later, but only after there is a real implementation need.

## Result Semantics

- `result_get_status` returns how the solve ended.
- `result_get_objective` returns the total tour cost.
- `result_get_tour_size` returns the number of nodes in the tour.
- `result_get_tour` copies the node order into the caller buffer in visit order.
- The returned tour does not repeat the start node at the end; it is a permutation of node ids.
- The objective is computed over the closed cycle from the last node back to the first.

## Ownership and Lifetime

- All create functions return an error code and write the new handle through an out-parameter.
- All destroy functions are `void` and accept `NULL` as a no-op.
- Result objects own their internal data until destroyed.
- Callers own any output buffers they pass to `result_get_tour`.
- The model may be reused across multiple `solve` calls if the underlying implementation supports it.

## Error Handling

- Public functions never throw across the C boundary.
- API failures are reported through `tsp_solver_error_code_t`.
- `INVALID_ARGUMENT` is used for null pointers, bad capacities, and malformed inputs.
- `OUT_OF_RANGE` is used for node ids outside the valid range.
- `INVALID_MODEL` is used when the model is incomplete or contradictory.
- `ALLOCATION_FAILED` is reserved for memory pressure and similar allocation failures.
- `INTERNAL_ERROR` is the catch-all for unexpected solver failures.

## Versioning

- The public header should expose compile-time version macros and a runtime version query.
- The ABI version must move slowly and only when the handle layout or function signatures change.
- Python and Java bindings should be able to check the version before loading native code.

## Testing Strategy

- C API tests should cover create/destroy lifecycles.
- Model tests should cover node insertion, distance assignment, and validation failures.
- Solve tests should cover a small complete graph and verify the objective and tour content.
- Result tests should confirm `get_tour_size` and `get_tour` agree.
- Regression tests should cover invalid pointers, insufficient output capacity, and incomplete models.
- Binding tests for Python and Java come later and should only wrap the C layer once it exists.

## First PR Scope

The first PR for this work should contain:

- this design document,
- a short project-goals update in the README if needed,
- no native implementation yet.

The implementation PRs that follow should add the public C header, then the C wrapper around the existing C++ core, then language bindings.
