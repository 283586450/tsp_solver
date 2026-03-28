# Examples Directory Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a top-level `examples/` tree with one shared 20-city dataset and runnable C++, Python, and Java comparison examples that demonstrate the same three solver algorithms.

**Architecture:** Keep the example data in a plain-text matrix file that all three languages can read without extra dependencies. Put one runnable example in each language-specific subdirectory, make them print the same comparison table, and expose them from a single `examples/README.md` plus a top-level README link.

**Tech Stack:** CMake, CTest, C++, Python, Java, Markdown.

---

## File Structure

- `examples/README.md` - single entry point that explains the shared dataset, the three algorithms, and how to run each example.
- `examples/data/20_city_distance_matrix.txt` - shared 20-city distance matrix used by all three languages.
- `examples/CMakeLists.txt` - examples subtree entry point for the C++ build target.
- `examples/cpp/CMakeLists.txt` - C++ example build target and any example-local wiring.
- `examples/cpp/compare_algorithms.cpp` - native example program that builds a model, runs the comparison, and prints a table.
- `examples/python/compare_algorithms.py` - Python example script that runs the same comparison.
- `examples/java/src/main/java/tsp/solver/examples/CompareAlgorithmsMain.java` - Java example entry point.
- `README.md` - top-level link to the examples entry point.
- `CMakeLists.txt` - add the `examples/` subtree to the top-level build if needed.
- `bindings/java/CMakeLists.txt` - compile the Java example alongside the existing Java binding sources if the Java build needs an extra source list.
- `tests/CMakeLists.txt` - register the example smoke tests.
- `tests/examples_cpp_smoke.cmake` - run and validate the C++ example output.
- `tests/examples_python_smoke.py` - run and validate the Python example output.
- `tests/examples_java_smoke.cmake` - run and validate the Java example output.

### Task 1: Add the shared dataset and the examples entry point

**Files:**
- Create: `examples/README.md`
- Create: `examples/data/20_city_distance_matrix.txt`
- Modify: `README.md`

- [ ] **Step 1: Write the examples entry-point docs and shared dataset format**

Create `examples/README.md` with these sections, in this order:

```md
# Examples

## What this shows
## Shared dataset
## C++ example
## Python example
## Java example
## Expected output
## Prerequisites
```

Use `examples/data/20_city_distance_matrix.txt` as a plain-text 20x20 integer matrix with a zero diagonal: the first line is `20`, followed by 20 rows of 20 space-separated integers so C++, Python, and Java can all parse it the same way.

Show the run commands in `examples/README.md` with the matrix path passed as the first argument for each language, for example:

```bash
examples/cpp/compare_algorithms examples/data/20_city_distance_matrix.txt
python examples/python/compare_algorithms.py examples/data/20_city_distance_matrix.txt
java -cp <java classpath> tsp.solver.examples.CompareAlgorithmsMain examples/data/20_city_distance_matrix.txt
```

Add a short `README.md` section that points readers to `examples/README.md`:

```md
## Examples

See `examples/README.md` for the C++, Python, and Java comparison examples that use the same 20-city dataset.
```

- [ ] **Step 2: Verify the new docs and dataset paths are cleanly formatted**

Run:

```bash
git diff --check
```

Expected: no whitespace or patch-format errors.

### Task 2: Add the C++ comparison example and smoke test

**Files:**
- Create: `examples/CMakeLists.txt`
- Create: `examples/cpp/CMakeLists.txt`
- Create: `examples/cpp/compare_algorithms.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/examples_cpp_smoke.cmake`

- [ ] **Step 1: Write the C++ example and its build target**

Create `examples/cpp/compare_algorithms.cpp` as a small command-line program that:

1. reads the shared matrix from the first command-line argument and prints `usage: compare_algorithms <matrix_path>` if it is missing;
2. builds one model;
3. runs `TSP_SOLVER_ALGORITHM_DEFAULT`, `TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR`, and `TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT`;
4. prints one line per algorithm with the algorithm name, objective, and tour.

Add `examples/CMakeLists.txt` so the examples subtree is visible to the top-level build, then add `examples/cpp/CMakeLists.txt` so the C++ example builds as a dedicated target, for example `tsp_solver_examples_cpp`.

Add the `examples/` subtree from the top-level `CMakeLists.txt` if it is not already included.

- [ ] **Step 2: Add a CTest smoke script that checks the printed comparison table**

Create `tests/examples_cpp_smoke.cmake` so it runs the built C++ example and asserts that the output contains all three algorithm labels. Pass the built binary path in from `tests/CMakeLists.txt` as `EXAMPLES_CPP_BINARY`:

```cmake
execute_process(COMMAND "${EXAMPLES_CPP_BINARY}" "${CMAKE_CURRENT_LIST_DIR}/../examples/data/20_city_distance_matrix.txt" WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/.." OUTPUT_VARIABLE output RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "C++ example failed")
endif()
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_DEFAULT" default_pos)
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR" greedy_pos)
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT" two_opt_pos)
```

Register it from `tests/CMakeLists.txt` as `tsp_solver_examples_cpp_smoke`.

- [ ] **Step 3: Verify the example target and smoke test are wired correctly**

Run:

```bash
cmake --build --preset <your-host-appropriate-preset> --target tsp_solver_examples_cpp
ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_examples_cpp_smoke
```

Expected: build succeeds and the smoke test passes.

### Task 3: Add the Python comparison example and smoke test

**Files:**
- Create: `examples/python/compare_algorithms.py`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/examples_python_smoke.py`

- [ ] **Step 1: Write the Python example script against the shared dataset**

Create `examples/python/compare_algorithms.py` so it:

1. loads the shared matrix from the first command-line argument and prints `usage: compare_algorithms.py <matrix_path>` if it is missing;
2. builds one model with the Python binding;
3. runs the same three algorithms as the C++ example;
4. prints the same comparison table headings and algorithm labels.

Keep the script importable from the source tree with the existing `bindings/python` setup and `TSP_SOLVER_LIBRARY_PATH` fallback.

- [ ] **Step 2: Add a Python smoke test that checks the script output**

Create `tests/examples_python_smoke.py` so it launches `examples/python/compare_algorithms.py` and checks that the output contains all three algorithm labels. Register the test with the same Python binding environment used by the existing Python package smoke test so `bindings/python` is on `PYTHONPATH` and the built shared library is available through `TSP_SOLVER_LIBRARY_PATH`:

```python
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

repo_root = Path(__file__).resolve().parents[1]
output = subprocess.check_output(
    [sys.executable, str(repo_root / "examples/python/compare_algorithms.py"), str(repo_root / "examples/data/20_city_distance_matrix.txt")],
    text=True,
)
assert "TSP_SOLVER_ALGORITHM_DEFAULT" in output
assert "TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR" in output
assert "TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT" in output
```

Register it from `tests/CMakeLists.txt` as `tsp_solver_examples_python_smoke`.

- [ ] **Step 3: Verify the Python example runs from the source tree**

Run:

```bash
python examples/python/compare_algorithms.py
ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_examples_python_smoke
```

Expected: the script prints the comparison table and the smoke test passes.

### Task 4: Add the Java comparison example and smoke test

**Files:**
- Create: `examples/java/src/main/java/tsp/solver/examples/CompareAlgorithmsMain.java`
- Modify: `bindings/java/CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/examples_java_smoke.cmake`

- [ ] **Step 1: Write the Java example entry point and wire it into the existing Java build**

Create `examples/java/src/main/java/tsp/solver/examples/CompareAlgorithmsMain.java` so it:

1. loads the shared matrix from the first command-line argument and prints `usage: CompareAlgorithmsMain <matrix_path>` if it is missing;
2. builds one model with the Java binding;
3. runs `TSP_SOLVER_ALGORITHM_DEFAULT`, `TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR`, and `TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT`;
4. prints the same comparison table headings and algorithm labels as the C++ and Python examples.

Update `bindings/java/CMakeLists.txt` so the example class is compiled into a dedicated Java example target or otherwise included in the Java build outputs in a way that CTest can run.

- [ ] **Step 2: Add a Java smoke script that checks the output labels**

Create `tests/examples_java_smoke.cmake` so it runs the Java example and asserts that all three algorithm labels appear in the output. Pass the classpath in from `tests/CMakeLists.txt` as `TSP_SOLVER_JAVA_EXAMPLE_CLASSPATH`:

```cmake
execute_process(COMMAND java -cp "${TSP_SOLVER_JAVA_EXAMPLE_CLASSPATH}" tsp.solver.examples.CompareAlgorithmsMain "${CMAKE_CURRENT_LIST_DIR}/../examples/data/20_city_distance_matrix.txt" OUTPUT_VARIABLE output RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Java example failed")
endif()
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_DEFAULT" default_pos)
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR" greedy_pos)
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT" two_opt_pos)
```

Register it from `tests/CMakeLists.txt` as `tsp_solver_examples_java_smoke`.

- [ ] **Step 3: Verify the Java example build and smoke test**

Run:

```bash
cmake --build --preset <your-host-appropriate-preset> --target tsp_solver_java_jar tsp_solver_java_native_bundle
ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_examples_java_smoke
```

Expected: the Java example runs and the smoke test passes.

### Task 5: Final documentation pass and full examples verification

**Files:**
- Modify as needed based on verification output

- [ ] **Step 1: Link the top-level README and examples entry point consistently**

Run:

```bash
git diff -- README.md examples/README.md examples/cpp/CMakeLists.txt examples/cpp/compare_algorithms.cpp examples/python/compare_algorithms.py examples/java/src/main/java/tsp/solver/examples/CompareAlgorithmsMain.java
```

Expected: the examples tree is discoverable from the top-level README and each language sample points at the same shared scenario.

- [ ] **Step 2: Run the full examples smoke matrix**

Run:

```bash
ctest --preset <your-host-appropriate-preset> --output-on-failure -R "tsp_solver_examples_cpp_smoke|tsp_solver_examples_python_smoke|tsp_solver_examples_java_smoke"
```

Expected: all three example smoke tests pass.

- [ ] **Step 3: Check the repository for formatting issues before commit**

Run:

```bash
git diff --check
```

Expected: no whitespace or patch-format errors.
