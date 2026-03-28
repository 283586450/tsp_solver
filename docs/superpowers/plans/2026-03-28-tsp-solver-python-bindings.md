# TSP Solver Python Bindings Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Expose the stable C API to Python with a thin, idiomatic wrapper for model, options, solve, and result access.

**Architecture:** Keep Python pure and lightweight. A private `ctypes` loader resolves the native shared library and binds the C functions; public Python classes own opaque native handles and translate error codes into exceptions. CMake only wires the package into tests and development builds; all solver behavior continues to live in the native library.

**Tech Stack:** Python 3, stdlib `ctypes`, stdlib `unittest`, CMake, CTest.

---

### Task 1: Scaffold the Python package and native loader

**Files:**
- Create: `bindings/python/tsp_solver/__init__.py`
- Create: `bindings/python/tsp_solver/_errors.py`
- Create: `bindings/python/tsp_solver/_native.py`
- Create: `tests/python/test_import.py`

- [ ] **Step 1: Write the failing import test**

```python
import unittest


class TestPublicExports(unittest.TestCase):
    def test_exports(self):
        import tsp_solver

        self.assertTrue(hasattr(tsp_solver, "Model"))
        self.assertTrue(hasattr(tsp_solver, "Options"))
        self.assertTrue(hasattr(tsp_solver, "Result"))
        self.assertTrue(hasattr(tsp_solver, "Algorithm"))
        self.assertTrue(hasattr(tsp_solver, "Status"))
        self.assertTrue(hasattr(tsp_solver, "solve"))


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run the import test before the package exists**

Run: `python -m unittest discover -s tests/python -p "test_*.py" -v`

Expected: fail with `ModuleNotFoundError: No module named 'tsp_solver'`.

- [ ] **Step 3: Add the package entry points, exception mapping, and C API loader**

Create `bindings/python/tsp_solver/_errors.py`:

```python
class SolverError(RuntimeError):
    pass


class InvalidArgumentError(ValueError, SolverError):
    pass


class OutOfRangeError(IndexError, SolverError):
    pass


class InvalidModelError(ValueError, SolverError):
    pass


class AllocationFailedError(SolverError):
    pass


class InternalError(SolverError):
    pass
```

Create `bindings/python/tsp_solver/_native.py` with one lazy loader that binds these C functions via `ctypes.CDLL`:

```python
import ctypes
import os
from ctypes.util import find_library

_LIB = None


def load_library():
    global _LIB
    if _LIB is not None:
        return _LIB

    candidates = []
    override = os.environ.get("TSP_SOLVER_LIBRARY_PATH")
    if override:
        candidates.append(override)
    found = find_library("tsp_solver")
    if found:
        candidates.append(found)

    last_error = None
    for candidate in candidates:
        try:
            _LIB = ctypes.CDLL(candidate)
            return _LIB
        except OSError as exc:
            last_error = exc

    raise RuntimeError("Unable to load tsp_solver native library") from last_error
```

Create `bindings/python/tsp_solver/__init__.py` so it re-exports the public API from the implementation module:

```python
from ._binding import Algorithm, Model, Options, Result, Status, solve
from ._errors import (
    AllocationFailedError,
    InternalError,
    InvalidArgumentError,
    InvalidModelError,
    OutOfRangeError,
    SolverError,
)

__all__ = [
    "Algorithm",
    "Model",
    "Options",
    "Result",
    "Status",
    "solve",
    "SolverError",
    "InvalidArgumentError",
    "OutOfRangeError",
    "InvalidModelError",
    "AllocationFailedError",
    "InternalError",
]
```

- [ ] **Step 4: Re-run the import test**

Run: `python -m unittest discover -s tests/python -p "test_*.py" -v`

Expected: pass; the package imports and exports the public names.

---

### Task 2: Implement `Model`, `Options`, `Result`, enums, and solve flow

**Files:**
- Create: `bindings/python/tsp_solver/_binding.py`
- Modify: `bindings/python/tsp_solver/__init__.py`
- Create: `tests/python/test_model.py`

- [ ] **Step 1: Write the failing model test**

```python
import unittest

import tsp_solver


class TestModelSolve(unittest.TestCase):
    def test_complete_graph_solve(self):
        model = tsp_solver.Model()
        options = tsp_solver.Options()
        options.algorithm = tsp_solver.Algorithm.LOCAL_SEARCH_2OPT

        for _ in range(4):
            model.add_node()

        distances = (
            (0, 2, 9, 10),
            (1, 0, 6, 4),
            (15, 7, 0, 8),
            (6, 3, 12, 0),
        )
        for from_id, row in enumerate(distances):
            for to_id, distance in enumerate(row):
                model.set_distance(from_id, to_id, distance)

        model.validate()
        result = model.solve(options)

        self.assertEqual(result.status, tsp_solver.Status.FEASIBLE)
        self.assertEqual(result.tour_size, 4)
        self.assertEqual(sorted(result.tour), [0, 1, 2, 3])
        self.assertIsInstance(result.objective, int)

    def test_validation_errors(self):
        model = tsp_solver.Model()
        model.add_node()
        model.add_node()

        with self.assertRaises(ValueError):
            model.validate()

        with self.assertRaises(IndexError):
            model.set_distance(2, 0, 5)


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run the model test before the wrapper exists**

Run: `python -m unittest discover -s tests/python -p "test_*.py" -v`

Expected: fail with `AttributeError` or `ImportError` because `Model`, `Options`, and `Result` are not implemented yet.

- [ ] **Step 3: Implement the wrapper classes and enum values**

Create `bindings/python/tsp_solver/_binding.py` with the opaque-handle wrappers and the native calls that back them. The module should define the enums, then build the three handle-owning classes around `ctypes.c_void_p`:

```python
class Algorithm(enum.IntEnum):
    DEFAULT = 0
    LOCAL_SEARCH_2OPT = 1


class Status(enum.IntEnum):
    NOT_SOLVED = 0
    FEASIBLE = 1
    OPTIMAL = 2
    INFEASIBLE = 3
    TIME_LIMIT = 4
    INVALID_MODEL = 5
    INTERNAL_ERROR = 6


class Model:
    def __init__(self):
        self._handle = _native.model_create()
        self._closed = False

    def add_node(self) -> int:
        return _native.model_add_node(self._handle)

    def solve(self, options=None):
        return _native.solve_model(self._handle, options)
```

Use the same pattern for `Options` and `Result`: `Options` owns the option handle and mirrors `time_limit_ms`, `random_seed`, and `algorithm`; `Result` exposes `status`, `objective`, `tour_size`, and `tour`. Every wrapper should support `close()`, `with` statements, and idempotent cleanup. Map each C error code to the exception classes in `_errors.py`.

- [ ] **Step 4: Re-run the model test**

Run: `python -m unittest discover -s tests/python -p "test_*.py" -v`

Expected: pass; the wrapper can build a model, solve it, and read the result.

---

### Task 3: Wire Python tests into CMake and load the shared library

**Files:**
- Create: `bindings/python/CMakeLists.txt`
- Modify: `CMakeLists.txt`
- Modify: `tests/python/test_import.py`
- Modify: `tests/python/test_model.py`

- [ ] **Step 1: Add the failing CTest wiring**

Create `bindings/python/CMakeLists.txt`:

```cmake
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(NAME tsp_solver_python_tests
  COMMAND ${Python3_EXECUTABLE} -m unittest discover
          -s ${CMAKE_SOURCE_DIR}/tests/python
          -p test_*.py -v)

set_tests_properties(tsp_solver_python_tests PROPERTIES
  ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}/bindings/python;TSP_SOLVER_LIBRARY_PATH=$<TARGET_FILE:tsp_solver>")
```

Modify the root `CMakeLists.txt` to include the Python bindings only when testing is enabled:

```cmake
if(BUILD_TESTING)
  add_subdirectory(tests)
  add_subdirectory(bindings/python)
endif()
```

- [ ] **Step 2: Run the Python CTest entry before the wiring exists**

Run: `ctest --preset windows-msvc-debug --output-on-failure -R tsp_solver_python_tests`

Expected: fail because the test entry does not exist yet.

- [ ] **Step 3: Keep the tests importable from the source tree**

Leave the Python tests as plain `unittest` modules under `tests/python/` so the CTest command can discover them without a packaging step.

- [ ] **Step 4: Re-run the Python CTest entry**

Run: `ctest --preset windows-msvc-debug --output-on-failure -R tsp_solver_python_tests`

Expected: pass; CTest sets `PYTHONPATH` and `TSP_SOLVER_LIBRARY_PATH` correctly.

---

### Task 4: Update docs and verify the full Python path

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Add a short Python bindings note**

Document the source-tree import path and the native library override:

```md
## Python Bindings

The Python package lives under `bindings/python/tsp_solver`. During local development, set `PYTHONPATH=bindings/python` and `TSP_SOLVER_LIBRARY_PATH` to the built native library, then run `python -m unittest discover -s tests/python -p "test_*.py" -v`.
```

- [ ] **Step 2: Run formatting and the full verification set**

Run:

```bash
clang-format --dry-run --Werror $(git ls-files "*.c" "*.cpp" "*.h" "*.hpp")
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug --output-on-failure
```

Expected: all checks pass, including the new Python tests.

- [ ] **Step 3: Commit the finished Python binding slice**

```bash
git add CMakeLists.txt README.md bindings/python tests/python docs/superpowers/plans/2026-03-28-tsp-solver-python-bindings.md
git commit -m "feat: add Python bindings over the C API"
```
