# Multi-Algorithm TSP Framework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Refactor `tsp_solver` into a multi-algorithm TSP framework with one shared API across C++, C API, Python, and Java, then add the first wave of algorithms and examples.

**Architecture:** Introduce framework-level algorithm, options, result, and dispatch types first. Migrate the current 2-opt implementation behind that dispatch layer. Then add Held-Karp, constructive heuristics, and iterated local search as independent modules. Bindings and examples consume the shared API rather than any single algorithm implementation.

**Tech Stack:** C++23, C API, CMake/CTest, Python ctypes bindings, Java JNI bindings.

---

## File Structure

- `include/tsp_solver/core/algorithm.hpp` - framework-level public algorithm enum
- `include/tsp_solver/core/options.hpp` - framework-level solve options
- `include/tsp_solver/core/result.hpp` - framework-level solve result
- `include/tsp_solver/solver.hpp` - unified public native solve entrypoint
- `src/solver/dispatch.cpp` - algorithm dispatch and default-selection policy
- `src/algorithms/*.cpp` - one implementation file per algorithm
- `include/tsp_solver/algorithms/*.hpp` - optional algorithm-specific internal/public declarations
- `include/tsp_solver/c_api.h` / `src/c_api.cpp` - native ABI surface for multi-algorithm solving
- `bindings/python/tsp_solver/_binding.py` - Python enum, options, result mapping
- `bindings/java/src/main/java/tsp/solver/*.java` - Java enum, options, result mapping
- `examples/` - cross-language algorithm comparison samples
- `docs/algorithms.md` - algorithm catalog and trade-offs

### Task 1: Add framework-level public solver types

**Files:**
- Create: `include/tsp_solver/core/algorithm.hpp`
- Create: `include/tsp_solver/core/options.hpp`
- Create: `include/tsp_solver/core/result.hpp`
- Create: `include/tsp_solver/solver.hpp`
- Modify: `include/tsp_solver/core/tour.hpp`
- Modify: `CMakeLists.txt`
- Test: `tests/solver_api_test.cpp`

- [ ] Add `enum class Algorithm` with the first-phase public algorithms.
- [ ] Add `SolveOptions` with `algorithm`, `time_limit_ms`, and `random_seed`.
- [ ] Add `SolveResult` with `status`, `objective`, `tour`, and `selected_algorithm`.
- [ ] Add a public `solve(const Problem&, const SolveOptions&)` declaration.
- [ ] Add a compile/runtime smoke test that exercises the new public API.
- [ ] Commit with `feat: add framework-level solver API`.

### Task 2: Introduce solver dispatch and migrate the current 2-opt solver

**Files:**
- Create: `src/solver/dispatch.cpp`
- Create: `src/algorithms/local_search_2opt.cpp`
- Create: `include/tsp_solver/algorithms/local_search_2opt.hpp`
- Modify: `src/algorithms/local_search.cpp`
- Modify: `include/tsp_solver/algorithms/local_search.hpp`
- Test: `tests/local_search_test.cpp`
- Test: `tests/solver_dispatch_test.cpp`

- [ ] Move the current 2-opt implementation behind the new dispatch layer.
- [ ] Make explicit `Algorithm::LOCAL_SEARCH_2OPT` route through dispatch.
- [ ] Define the initial `DEFAULT` policy inside dispatch.
- [ ] Return `selected_algorithm` in the native result.
- [ ] Add tests for explicit 2-opt and default dispatch behavior.
- [ ] Commit with `refactor: route native solving through algorithm dispatch`.

### Task 3: Expand the native C API for multi-algorithm solving

**Files:**
- Modify: `include/tsp_solver/c_api.h`
- Modify: `src/c_api.cpp`
- Test: `tests/c_api_runtime_test.cpp`
- Test: `tests/c_api_error_test.cpp`

- [ ] Extend `tsp_solver_algorithm_t` with the first-phase algorithms.
- [ ] Route `tsp_solver_solve()` through framework dispatch instead of constructing `TwoOptLocalSearch` directly.
- [ ] Add `tsp_solver_result_get_selected_algorithm(...)`.
- [ ] Keep `tsp_solver_options_set_algorithm(...)` as the one algorithm-selection entrypoint.
- [ ] Add native tests for explicit algorithm selection and reported selected algorithm.
- [ ] Commit with `feat: expose multi-algorithm solving through C API`.

### Task 4: Update Python bindings to the framework API

**Files:**
- Modify: `bindings/python/tsp_solver/_binding.py`
- Modify: `bindings/python/tsp_solver/__init__.py`
- Create: `tests/python/test_algorithms.py`
- Modify: `tests/python/test_model.py`

- [ ] Expand Python `Algorithm` to match the native enum.
- [ ] Keep `Options.algorithm` as the public selector.
- [ ] Add `Result.selected_algorithm`.
- [ ] Add tests for enum mapping, explicit selection, and default selected-algorithm reporting.
- [ ] Commit with `feat: expose framework algorithms in Python bindings`.

### Task 5: Update Java bindings to the framework API

**Files:**
- Modify: `bindings/java/src/main/java/tsp/solver/Algorithm.java`
- Modify: `bindings/java/src/main/java/tsp/solver/Result.java`
- Modify: `bindings/java/src/main/cpp/tsp_solver_java_jni.cpp`
- Modify: `bindings/java/src/test/java/tsp/solver/BindingTestMain.java`

- [ ] Expand Java `Algorithm` to match the native enum.
- [ ] Add `Result.getSelectedAlgorithm()`.
- [ ] Add JNI plumbing for selected-algorithm retrieval.
- [ ] Add tests for explicit algorithm selection and default reporting.
- [ ] Commit with `feat: expose framework algorithms in Java bindings`.

### Task 6: Implement Held-Karp for small exact solves

**Files:**
- Create: `src/algorithms/exact_held_karp.cpp`
- Create: `include/tsp_solver/algorithms/exact_held_karp.hpp`
- Test: `tests/exact_held_karp_test.cpp`
- Modify: `src/solver/dispatch.cpp`

- [ ] Implement exact Held-Karp for small instances.
- [ ] Register `EXACT_HELD_KARP` in dispatch.
- [ ] Add fixed-size correctness tests with known optimal tours.
- [ ] Commit with `feat: add exact held-karp solver`.

### Task 7: Implement constructive heuristics

**Files:**
- Create: `src/algorithms/greedy_nearest_neighbor.cpp`
- Create: `include/tsp_solver/algorithms/greedy_nearest_neighbor.hpp`
- Create: `src/algorithms/greedy_cheapest_insertion.cpp`
- Create: `include/tsp_solver/algorithms/greedy_cheapest_insertion.hpp`
- Test: `tests/greedy_algorithms_test.cpp`
- Modify: `src/solver/dispatch.cpp`

- [ ] Implement nearest-neighbor construction as a standalone algorithm.
- [ ] Implement cheapest-insertion construction as a standalone algorithm.
- [ ] Register both in dispatch.
- [ ] Add legality and small-instance regression tests.
- [ ] Commit with `feat: add constructive TSP heuristics`.

### Task 8: Implement iterated local search as the first metaheuristic

**Files:**
- Create: `src/algorithms/iterated_local_search.cpp`
- Create: `include/tsp_solver/algorithms/iterated_local_search.hpp`
- Test: `tests/iterated_local_search_test.cpp`
- Modify: `src/solver/dispatch.cpp`

- [ ] Implement ILS using the migrated 2-opt improvement logic.
- [ ] Keep perturbation and acceptance parameters internal in phase 1.
- [ ] Register `METAHEURISTIC_ITERATED_LOCAL_SEARCH`.
- [ ] Decide whether `DEFAULT` switches to ILS in this phase.
- [ ] Add legality and deterministic-seed tests.
- [ ] Commit with `feat: add iterated local search solver`.

### Task 9: Add examples for algorithm comparison

**Files:**
- Create: `examples/cpp/compare_algorithms.cpp`
- Create: `examples/python/compare_algorithms.py`
- Create: `examples/java/CompareAlgorithmsMain.java`
- Modify: `CMakeLists.txt`
- Modify: `bindings/java/CMakeLists.txt`

- [ ] Add one example per language that solves the same graph with multiple algorithms.
- [ ] Print chosen algorithm, selected algorithm, objective, and tour size.
- [ ] Keep examples deterministic and small.
- [ ] Commit with `docs: add multi-algorithm usage examples`.

### Task 10: Rewrite documentation around the multi-algorithm goal

**Files:**
- Modify: `README.md`
- Create: `docs/algorithms.md`
- Modify: `docs/c_api.md`

- [ ] Rewrite README current-state and roadmap sections around the framework goal.
- [ ] Document the first-phase algorithm catalog and trade-offs.
- [ ] Explain `DEFAULT` and explicit algorithm selection across languages.
- [ ] Link the new examples.
- [ ] Commit with `docs: describe the multi-algorithm framework`.

### Task 11: Run full framework verification

**Files:**
- Modify as needed based on verification output

- [ ] Run the full native test suite.
- [ ] Run Python tests and package smoke.
- [ ] Run Java tests and package smoke.
- [ ] Run exact-version package smoke.
- [ ] Run formatting on touched C++ files.
- [ ] Verify the examples build or run as documented.
- [ ] Commit any final fixes with `fix: finish multi-algorithm framework rollout`.

## Recommended execution order

1. Tasks 1-5: framework/API foundation
2. Task 6: exact baseline
3. Task 7: constructive heuristics
4. Task 8: first metaheuristic
5. Tasks 9-10: examples and docs
6. Task 11: final verification

## Why this order

This matches the project direction agreed during brainstorming:

- first build the API and extension structure;
- then add algorithms incrementally behind it;
- then show users how to use the resulting framework.
