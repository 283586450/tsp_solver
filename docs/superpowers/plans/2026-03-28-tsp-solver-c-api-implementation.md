# TSP Solver C API Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship the first stable C API for the solver with opaque model/options/result handles, a solve entry point, and result accessors that can later back Python and Java bindings.

**Architecture:** Keep the native C++23 solver core private and expose only a C11-compatible boundary in `include/tsp_solver/c_api.h`. The C layer owns the ABI, error codes, and object lifetimes; it converts C models into the existing C++ `Problem` type, dispatches to the current local-search backend, and copies the tour back into caller-owned storage. The first version stays generic: `node` and `distance` are the primitive concepts, while any coordinate-based convenience layer remains out of scope.

**Tech Stack:** C11-compatible public header, C++23 implementation, CMake, CTest, current native local-search solver.

---

### Task 1: Publish the C API contract and make it buildable

**Files:**
- Create: `include/tsp_solver/c_api.h`
- Create: `tests/c_api_header_test.c`
- Modify: `CMakeLists.txt`
- Modify: `src/CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`

- [ ] **Step 1: Write the failing compile-only header test**

Create `tests/c_api_header_test.c`:

```c
#include "tsp_solver/c_api.h"

#include <assert.h>
#include <stdint.h>

int main(void) {
  _Static_assert(TSP_SOLVER_VERSION_MAJOR == 0, "major version");
  _Static_assert(TSP_SOLVER_VERSION_MINOR == 1, "minor version");
  _Static_assert(TSP_SOLVER_VERSION_PATCH == 0, "patch version");
  _Static_assert(sizeof(tsp_solver_cost_t) == 8, "cost width");

  tsp_solver_model_t* model = NULL;
  tsp_solver_options_t* options = NULL;
  tsp_solver_result_t* result = NULL;
  (void)model;
  (void)options;
  (void)result;
  return 0;
}
```

- [ ] **Step 2: Run the target before the header exists**

Run: `cmake --build --preset windows-msvc-debug --target tsp_solver_c_api_header_test`

Expected: fail because `include/tsp_solver/c_api.h` and the test target do not exist yet.

- [ ] **Step 3: Add the public header and wire C into the build**

Create `include/tsp_solver/c_api.h`:

```c
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TSP_SOLVER_VERSION_MAJOR 0
#define TSP_SOLVER_VERSION_MINOR 1
#define TSP_SOLVER_VERSION_PATCH 0
#define TSP_SOLVER_ABI_VERSION 1

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

const char* tsp_solver_version_string(void);

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

#ifdef __cplusplus
}  // extern "C"
#endif
```

Update `CMakeLists.txt` to enable the C language and set the C standard:

```cmake
project(tsp_solver VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
```

Update `src/CMakeLists.txt` so the library compiles the new wrapper later:

```cmake
target_sources(tsp_solver
  PRIVATE
    algorithms/local_search.cpp
    c_api.cpp)
```

Update `tests/CMakeLists.txt` to add the header test target:

```cmake
add_executable(tsp_solver_c_api_header_test c_api_header_test.c)
add_test(NAME tsp_solver_c_api_header_test COMMAND tsp_solver_c_api_header_test)
```

- [ ] **Step 4: Re-run the header test target**

Run: `cmake --build --preset windows-msvc-debug --target tsp_solver_c_api_header_test`

Expected: pass; the header should compile as C and the target should link without needing the implementation.

---

### Task 2: Implement the model/options/result bridge and solve path

**Files:**
- Create: `src/c_api.cpp`
- Modify: `src/CMakeLists.txt`
- Create: `tests/c_api_runtime_test.cpp`
- Modify: `tests/CMakeLists.txt`

- [ ] **Step 1: Write the failing runtime test**

Create `tests/c_api_runtime_test.cpp`:

```cpp
#include "tsp_solver/c_api.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

static tsp_solver_cost_t tour_cost(
    const std::array<std::array<tsp_solver_cost_t, 4>, 4>& distances,
    const std::vector<tsp_solver_node_id_t>& tour) {
  tsp_solver_cost_t total = 0;
  for (std::size_t index = 0; index < tour.size(); ++index) {
    const std::size_t next = (index + 1) % tour.size();
    total += distances[tour[index]][tour[next]];
  }
  return total;
}

int main() {
  const std::array<std::array<tsp_solver_cost_t, 4>, 4> distances{{
      {{0, 2, 9, 10}},
      {{1, 0, 6, 4}},
      {{15, 7, 0, 8}},
      {{6, 3, 12, 0}},
  }};

  tsp_solver_model_t* model = nullptr;
  tsp_solver_options_t* options = nullptr;
  tsp_solver_result_t* result = nullptr;

  assert(tsp_solver_model_create(&model) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_options_create(&options) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_options_set_algorithm(options,
         TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT) == TSP_SOLVER_ERROR_OK);

  tsp_solver_node_id_t ids[4] = {};
  for (std::size_t index = 0; index < 4; ++index) {
    assert(tsp_solver_model_add_node(model, &ids[index]) == TSP_SOLVER_ERROR_OK);
  }

  for (tsp_solver_node_id_t from = 0; from < 4; ++from) {
    for (tsp_solver_node_id_t to = 0; to < 4; ++to) {
      assert(tsp_solver_model_set_distance(model, from, to, distances[from][to]) ==
             TSP_SOLVER_ERROR_OK);
    }
  }

  assert(tsp_solver_model_validate(model) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_solve(model, options, &result) == TSP_SOLVER_ERROR_OK);

  tsp_solver_status_t status = TSP_SOLVER_STATUS_NOT_SOLVED;
  tsp_solver_cost_t objective = 0;
  std::size_t tour_size = 0;
  assert(tsp_solver_result_get_status(result, &status) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_result_get_objective(result, &objective) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_result_get_tour_size(result, &tour_size) == TSP_SOLVER_ERROR_OK);
  assert(status == TSP_SOLVER_STATUS_FEASIBLE);
  assert(tour_size == 4);

  std::vector<tsp_solver_node_id_t> tour(tour_size);
  std::size_t written = 0;
  assert(tsp_solver_result_get_tour(result, tour.data(), tour.size() - 1, &written) ==
         TSP_SOLVER_ERROR_INVALID_ARGUMENT);
  assert(tsp_solver_result_get_tour(result, tour.data(), tour.size(), &written) ==
         TSP_SOLVER_ERROR_OK);
  assert(written == tour.size());

  std::vector<tsp_solver_node_id_t> sorted_tour = tour;
  std::sort(sorted_tour.begin(), sorted_tour.end());
  assert((sorted_tour == std::vector<tsp_solver_node_id_t>{0, 1, 2, 3}));
  assert(objective == tour_cost(distances, tour));

  tsp_solver_result_destroy(result);
  tsp_solver_options_destroy(options);
  tsp_solver_model_destroy(model);
  return 0;
}
```

- [ ] **Step 2: Run the runtime target before the wrapper exists**

Run: `cmake --build --preset windows-msvc-debug --target tsp_solver_c_api_runtime_test`

Expected: fail with unresolved symbols for the C API functions.

- [ ] **Step 3: Add the C++ wrapper implementation**

Create `src/c_api.cpp` with these internal data structures and exported functions:

```cpp
namespace {

struct tsp_solver_model {
  std::vector<std::vector<std::optional<tsp_solver::Cost>>> distances;
};

struct tsp_solver_options {
  uint64_t time_limit_ms = UINT64_MAX;
  uint64_t random_seed = 0;
  tsp_solver_algorithm_t algorithm = TSP_SOLVER_ALGORITHM_DEFAULT;
};

struct tsp_solver_result {
  tsp_solver_status_t status = TSP_SOLVER_STATUS_NOT_SOLVED;
  tsp_solver::Cost objective = 0;
  std::vector<tsp_solver::NodeId> tour;
};

[[nodiscard]] bool is_square(const tsp_solver_model& model);
[[nodiscard]] tsp_solver::Problem to_problem(const tsp_solver_model& model);

}  // namespace

extern "C" {
const char* tsp_solver_version_string(void);
tsp_solver_error_code_t tsp_solver_model_create(tsp_solver_model_t** out_model);
void tsp_solver_model_destroy(tsp_solver_model_t* model);
tsp_solver_error_code_t tsp_solver_model_add_node(tsp_solver_model_t* model,
                                                  tsp_solver_node_id_t* out_node_id);
tsp_solver_error_code_t tsp_solver_model_set_distance(tsp_solver_model_t* model,
                                                      tsp_solver_node_id_t from,
                                                      tsp_solver_node_id_t to,
                                                      tsp_solver_cost_t distance);
tsp_solver_error_code_t tsp_solver_model_validate(const tsp_solver_model_t* model);
tsp_solver_error_code_t tsp_solver_options_create(tsp_solver_options_t** out_options);
void tsp_solver_options_destroy(tsp_solver_options_t* options);
tsp_solver_error_code_t tsp_solver_options_set_time_limit_ms(
    tsp_solver_options_t* options,
    uint64_t time_limit_ms);
tsp_solver_error_code_t tsp_solver_options_set_random_seed(tsp_solver_options_t* options,
                                                           uint64_t random_seed);
tsp_solver_error_code_t tsp_solver_options_set_algorithm(tsp_solver_options_t* options,
                                                         tsp_solver_algorithm_t algorithm);
tsp_solver_error_code_t tsp_solver_solve(const tsp_solver_model_t* model,
                                         const tsp_solver_options_t* options,
                                         tsp_solver_result_t** out_result);
void tsp_solver_result_destroy(tsp_solver_result_t* result);
tsp_solver_error_code_t tsp_solver_result_get_status(const tsp_solver_result_t* result,
                                                     tsp_solver_status_t* out_status);
tsp_solver_error_code_t tsp_solver_result_get_objective(const tsp_solver_result_t* result,
                                                        tsp_solver_cost_t* out_objective);
tsp_solver_error_code_t tsp_solver_result_get_tour_size(const tsp_solver_result_t* result,
                                                        size_t* out_size);
tsp_solver_error_code_t tsp_solver_result_get_tour(const tsp_solver_result_t* result,
                                                   tsp_solver_node_id_t* out_nodes,
                                                   size_t capacity,
                                                   size_t* out_written);
}
```

Use the existing `tsp_solver::TwoOptLocalSearch` backend for `TSP_SOLVER_ALGORITHM_DEFAULT` and `TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT`. Return `TSP_SOLVER_STATUS_FEASIBLE` for successful solves, `TSP_SOLVER_STATUS_INVALID_MODEL` for incomplete models, and preserve the internal tour/objective in the result object.

- [ ] **Step 4: Re-run the runtime target and the native test suite**

Run:

```bash
cmake --build --preset windows-msvc-debug --target tsp_solver_c_api_runtime_test
ctest --preset windows-msvc-debug --output-on-failure
```

Expected: both the C API runtime test and the existing solver test pass.

---

### Task 3: Cover negative cases and document the API

**Files:**
- Create: `tests/c_api_error_test.cpp`
- Modify: `tests/CMakeLists.txt`
- Modify: `README.md`
- Create: `docs/c_api.md`

- [ ] **Step 1: Write the failing error-path test**

Create `tests/c_api_error_test.cpp`:

```cpp
#include "tsp_solver/c_api.h"

#include <cassert>

int main() {
  tsp_solver_model_t* model = nullptr;
  tsp_solver_options_t* options = nullptr;
  tsp_solver_result_t* result = nullptr;
  tsp_solver_node_id_t node_id = 0;

  assert(tsp_solver_model_add_node(nullptr, &node_id) ==
         TSP_SOLVER_ERROR_INVALID_ARGUMENT);
  assert(tsp_solver_model_create(&model) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_options_create(&options) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_model_add_node(model, &node_id) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_model_set_distance(model, 1, 0, 1) ==
         TSP_SOLVER_ERROR_OUT_OF_RANGE);
  assert(tsp_solver_model_validate(model) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_result_get_tour(nullptr, nullptr, 0, nullptr) ==
         TSP_SOLVER_ERROR_INVALID_ARGUMENT);
  assert(tsp_solver_options_set_time_limit_ms(options, 0) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_solve(model, options, &result) == TSP_SOLVER_ERROR_OK);

  tsp_solver_status_t status = TSP_SOLVER_STATUS_NOT_SOLVED;
  assert(tsp_solver_result_get_status(result, &status) == TSP_SOLVER_ERROR_OK);
  assert(status == TSP_SOLVER_STATUS_TIME_LIMIT);

  tsp_solver_result_destroy(result);
  tsp_solver_options_destroy(options);
  tsp_solver_model_destroy(model);
  return 0;
}
```

- [ ] **Step 2: Run the error test before the guardrails exist**

Run: `cmake --build --preset windows-msvc-debug --target tsp_solver_c_api_error_test`

Expected: fail until the wrapper returns the planned error codes and time-limit behavior.

- [ ] **Step 3: Add the doc updates once the tests pass**

Create `docs/c_api.md` with a short usage example like this:

# TSP Solver C API

1. Create a model.
2. Add nodes and distances.
3. Create options and choose an algorithm.
4. Solve.
5. Copy the tour and objective from the result.

```c
tsp_solver_model_t* model = NULL;
tsp_solver_options_t* options = NULL;
tsp_solver_result_t* result = NULL;
```

Update `README.md` with a short "C API" section that points readers to `include/tsp_solver/c_api.h` and `docs/c_api.md`.

- [ ] **Step 4: Run the full preset test suite and verify the docs build cleanly**

Run:

```bash
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug --output-on-failure
```

Expected: all tests pass and the repository is ready for a PR focused on the first public C API.
