# TSP Solver Initialization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Initialize a portable C++ TSP solver repository with a buildable core, a first local-search algorithm path, and a minimal test harness.

**Architecture:** Keep the native core in `include/` and `src/` with a narrow public API that can later be wrapped by Python and Java bindings. Start with a small but useful solver foundation: problem/tour data types, a local-search interface, and one concrete local-search implementation. Use `CMake` as the single build entry point and `CTest` for native test execution.

**Tech Stack:** C++20, CMake, CTest, Ninja, MSVC/Clang/GCC-compatible standard library code, plain C++ test executable.

---

### Task 1: Bootstrap repository metadata

**Files:**
- Create: `.gitignore`
- Create: `README.md`
- Modify: `AGENTS.md` only if the new build commands differ from the current guidance

- [ ] **Step 1: Add the repository ignores and basic project overview**

Create `.gitignore` with common generated artifacts:

```gitignore
/build/
/out/
/cmake-build-*/
/CMakeCache.txt
/CMakeFiles/
/Testing/
/install/
/compile_commands.json
*.suo
*.user
*.vcxproj.user
*.log
*.pyc
__pycache__/
.pytest_cache/
.idea/
.vscode/
```

Create `README.md` with a short description and initial commands:

```md
# TSP Solver

Portable C++ solver for the traveling salesman problem.

## Build

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build

## Test

ctest --test-dir build --output-on-failure
```

- [ ] **Step 2: Verify git sees only the intended metadata files**

Run: `git status --short`
Expected: only new metadata files and the new plan document are listed.

---

### Task 2: Create the CMake build skeleton

**Files:**
- Create: `CMakeLists.txt`
- Create: `cmake/ProjectWarnings.cmake`
- Create: `src/CMakeLists.txt`
- Create: `tests/CMakeLists.txt`

- [ ] **Step 1: Write the root build entry point**

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.24)

project(tsp_solver VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tsp_solver)
add_library(tsp_solver::tsp_solver ALIAS tsp_solver)

target_include_directories(tsp_solver
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_subdirectory(src)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
```

Create `cmake/ProjectWarnings.cmake` with a small helper for warning flags per compiler:

```cmake
function(tsp_solver_enable_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endfunction()
```

Create `src/CMakeLists.txt`:

```cmake
target_sources(tsp_solver
  PRIVATE
    algorithms/local_search.cpp)

target_include_directories(tsp_solver
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/ProjectWarnings.cmake)
tsp_solver_enable_warnings(tsp_solver)
```

Create `tests/CMakeLists.txt`:

```cmake
add_executable(tsp_solver_tests local_search_test.cpp)
target_link_libraries(tsp_solver_tests PRIVATE tsp_solver)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/ProjectWarnings.cmake)
tsp_solver_enable_warnings(tsp_solver_tests)

add_test(NAME tsp_solver_tests COMMAND tsp_solver_tests)
```

- [ ] **Step 2: Verify CMake config succeeds after the source tree is in place**

Run:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

Expected: configure succeeds once the core headers, sources, and test target are added.

---

### Task 3: Add core TSP types and the first solver interface

**Files:**
- Create: `include/tsp_solver/core/problem.hpp`
- Create: `include/tsp_solver/core/tour.hpp`
- Create: `include/tsp_solver/algorithms/local_search.hpp`
- Create: `src/algorithms/local_search.cpp`

- [ ] **Step 1: Define the problem and tour data structures**

Create `include/tsp_solver/core/problem.hpp`:

```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace tsp_solver {

using NodeId = std::uint32_t;
using Cost = std::int64_t;

struct Problem {
  std::vector<std::vector<Cost>> distances;

  [[nodiscard]] std::size_t size() const noexcept {
    return distances.size();
  }

  [[nodiscard]] bool is_square() const noexcept;
};

}  // namespace tsp_solver
```

Create `include/tsp_solver/core/tour.hpp`:

```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "tsp_solver/core/problem.hpp"

namespace tsp_solver {

struct Tour {
  std::vector<NodeId> order;
  Cost total_cost = 0;

  [[nodiscard]] bool empty() const noexcept { return order.empty(); }
};

[[nodiscard]] Cost compute_tour_cost(const Problem& problem,
                                     const Tour& tour);

}  // namespace tsp_solver
```

- [ ] **Step 2: Define the local-search API**

Create `include/tsp_solver/algorithms/local_search.hpp`:

```cpp
#pragma once

#include "tsp_solver/core/problem.hpp"
#include "tsp_solver/core/tour.hpp"

namespace tsp_solver {

class LocalSearchSolver {
 public:
  virtual ~LocalSearchSolver() = default;
  [[nodiscard]] virtual Tour solve(const Problem& problem) const = 0;
};

class TwoOptLocalSearch final : public LocalSearchSolver {
 public:
  [[nodiscard]] Tour solve(const Problem& problem) const override;
};

}  // namespace tsp_solver
```

- [ ] **Step 3: Implement the first concrete solver**

Create `src/algorithms/local_search.cpp` with a small, deterministic 2-opt improvement loop and a simple nearest-neighbor seed. Keep the implementation isolated so later heuristics can share the same helpers.

Expected behavior:
- return a valid tour that visits each node once
- evaluate the current tour cost using the distance matrix
- improve the seed tour only when a swap lowers the cost

- [ ] **Step 4: Wire the implementation into the library target**

Update `src/CMakeLists.txt` so the new source file is compiled into `tsp_solver`.

---

### Task 4: Add a native test harness

**Files:**
- Create: `tests/local_search_test.cpp`

- [ ] **Step 1: Write a small deterministic test**

Create `tests/local_search_test.cpp`:

```cpp
#include <cassert>

#include "tsp_solver/algorithms/local_search.hpp"

int main() {
  tsp_solver::Problem problem{{
      {0, 2, 9, 10},
      {1, 0, 6, 4},
      {15, 7, 0, 8},
      {6, 3, 12, 0},
  }};

  tsp_solver::TwoOptLocalSearch solver;
  const auto tour = solver.solve(problem);

  assert(tour.order.size() == 4);
  assert(tour.total_cost > 0);
  assert(tsp_solver::compute_tour_cost(problem, tour) == tour.total_cost);
  return 0;
}
```

- [ ] **Step 2: Run the test binary directly after the first successful build**

Run: `build/tests/tsp_solver_tests` or the platform-correct executable path.
Expected: exit code `0`.

- [ ] **Step 3: Run the CTest entry point**

Run: `ctest --test-dir build --output-on-failure`
Expected: one passing test named `tsp_solver_tests`.

---

### Task 5: Verify the toolchain and capture project status

**Files:**
- Modify: `README.md` if any toolchain requirement needs to be documented

- [ ] **Step 1: Check the required tools again in the initialized repo**

Run:

```bash
git --version
cmake --version
ninja --version
python --version
java -version
javac -version
cl
```

If Python tests are expected later, also run:

```bash
python -m pytest --version
```

Expected: document any missing or partial toolchain pieces in the final handoff.

- [ ] **Step 2: Capture the final repository state**

Run:

```bash
git status --short
git branch --show-current
```

Expected: a clean `main` branch once the scaffold is committed.
