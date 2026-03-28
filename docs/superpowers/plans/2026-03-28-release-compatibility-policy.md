# Release Compatibility Policy Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Enforce and document a strict same-version compatibility policy across the C++, Python, and Java deliverables.

**Architecture:** Keep the C API as the native version source, then have Python and Java compare their own package/jar version against `tsp_solver_version_string()` immediately after loading native code. Prefer early, explicit mismatch failures plus focused regression tests over attempting cross-version compatibility.

**Tech Stack:** C API, ctypes, JNI, CMake/CTest, Python unittest, Java test mains.

---

## File Structure

- `src/c_api.cpp` - keeps the native version string implementation consistent with the C header macros.
- `bindings/python/tsp_solver/_errors.py` - adds a dedicated Python version mismatch error type.
- `bindings/python/tsp_solver/_native.py` - loads the native version string and enforces exact version matching after library load.
- `bindings/python/tsp_solver/__init__.py` - continues to expose package version metadata.
- `tests/python/test_version_compat.py` - covers Python mismatch behavior and message quality.
- `bindings/java/src/main/java/tsp/solver/Version.java.in` - template for the jar version constant.
- `bindings/java/CMakeLists.txt` - generates the Java version source from the top-level project version.
- `bindings/java/src/main/java/tsp/solver/NativeLibrary.java` - loads the JNI bridge and enforces same-version matching.
- `bindings/java/src/main/cpp/tsp_solver_java_jni.cpp` - exposes the native version string through JNI.
- `bindings/java/src/test/java/tsp/solver/BindingTestMain.java` - keeps happy-path coverage and adds mismatch regression coverage.
- `README.md` - states the compatibility policy and asset pairing rules for C++, Python, and Java users.

### Task 1: Make the native version source explicit and reusable

**Files:**
- Modify: `src/c_api.cpp`
- Test: `tests/c_api_runtime_test.cpp`

- [ ] **Step 1: Write a failing native version regression assertion**

Add an assertion to `tests/c_api_runtime_test.cpp` that the runtime version string matches the release macros:

```cpp
constexpr char kExpectedVersion[] = TSP_SOLVER_STRINGIFY(TSP_SOLVER_VERSION_MAJOR)
                                    "."
                                    TSP_SOLVER_STRINGIFY(TSP_SOLVER_VERSION_MINOR)
                                    "."
                                    TSP_SOLVER_STRINGIFY(TSP_SOLVER_VERSION_PATCH);
EXPECT_STREQ(kExpectedVersion, tsp_solver_version_string());
```

- [ ] **Step 2: Run the focused test to verify the assertion shape**

Run: `ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_c_api_runtime_test`

Expected: either PASS immediately or FAIL only if the runtime string is not kept in sync.

- [ ] **Step 3: Replace the hard-coded native version string with a macro-built constant**

Update `src/c_api.cpp` so the runtime string is assembled from the header macros instead of a raw literal:

```cpp
#define TSP_SOLVER_STRINGIFY_IMPL(value) #value
#define TSP_SOLVER_STRINGIFY(value) TSP_SOLVER_STRINGIFY_IMPL(value)

namespace {
constexpr char kVersionString[] = TSP_SOLVER_STRINGIFY(TSP_SOLVER_VERSION_MAJOR)
                                  "."
                                  TSP_SOLVER_STRINGIFY(TSP_SOLVER_VERSION_MINOR)
                                  "."
                                  TSP_SOLVER_STRINGIFY(TSP_SOLVER_VERSION_PATCH);
} // namespace

const char* tsp_solver_version_string(void) {
  return kVersionString;
}
```

- [ ] **Step 4: Run the focused C API regression test**

Run: `ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_c_api_runtime_test`

Expected: PASS.

- [ ] **Step 5: Commit the native version sync change**

```bash
git add src/c_api.cpp tests/c_api_runtime_test.cpp
git commit -m "fix: derive native version string from release macros"
```

### Task 2: Enforce strict same-version compatibility in Python

**Files:**
- Modify: `bindings/python/tsp_solver/_errors.py`
- Modify: `bindings/python/tsp_solver/_native.py`
- Modify: `bindings/python/tsp_solver/__init__.py`
- Create: `tests/python/test_version_compat.py`

- [ ] **Step 1: Write a failing Python mismatch test**

Create `tests/python/test_version_compat.py` with a focused mismatch test that exercises comparison logic through a test seam instead of requiring historical artifacts:

```python
from __future__ import annotations

import unittest

from tsp_solver._errors import VersionMismatchError
from tsp_solver import _native


class VersionCompatibilityTest(unittest.TestCase):
    def test_python_package_and_native_versions_must_match(self) -> None:
        with self.assertRaisesRegex(
            VersionMismatchError,
            r"Python package version 0\.1\.0 does not match native library version 9\.9\.9",
        ):
            _native._ensure_version_match("0.1.0", "9.9.9")
```

- [ ] **Step 2: Run the new Python test to verify it fails first**

Run: `python -m unittest tests.python.test_version_compat -v`

Expected: FAIL because `VersionMismatchError` or `_ensure_version_match()` does not exist yet.

- [ ] **Step 3: Add a dedicated Python mismatch error type**

Extend `bindings/python/tsp_solver/_errors.py`:

```python
__all__ = [
    "SolverError",
    "LibraryLoadError",
    "NativeCallError",
    "VersionMismatchError",
]


class VersionMismatchError(SolverError):
    """Raised when Python package and native library versions differ."""
```

- [ ] **Step 4: Implement native version reading and exact-match enforcement**

In `bindings/python/tsp_solver/_native.py`, add:

```python
def native_version_string() -> str:
    library = load_library()
    library.tsp_solver_version_string.argtypes = []
    library.tsp_solver_version_string.restype = ctypes.c_char_p
    value = library.tsp_solver_version_string()
    if value is None:
        raise NativeCallError("native library did not provide a version string")
    return value.decode("utf-8")


def _ensure_version_match(package_version: str, native_version: str) -> None:
    if package_version == native_version:
        return
    raise VersionMismatchError(
        "Python package version "
        f"{package_version} does not match native library version {native_version}. "
        "Use Python and native artifacts from the same tsp_solver release."
    )
```

Then call `_ensure_version_match()` immediately after the native library is loaded, using `__version__` imported lazily from `tsp_solver` or passed from a small helper.

- [ ] **Step 5: Run the focused Python mismatch test again**

Run: `python -m unittest tests.python.test_version_compat -v`

Expected: PASS.

- [ ] **Step 6: Run the existing Python binding tests**

Run: `ctest --preset <your-host-appropriate-preset> --output-on-failure -R "tsp_solver_python_tests|tsp_solver_python_package_smoke"`

Expected: PASS.

- [ ] **Step 7: Commit the Python compatibility enforcement**

```bash
git add bindings/python/tsp_solver/_errors.py bindings/python/tsp_solver/_native.py bindings/python/tsp_solver/__init__.py tests/python/test_version_compat.py
git commit -m "feat: enforce Python native version compatibility"
```

### Task 3: Enforce strict same-version compatibility in Java

**Files:**
- Modify: `bindings/java/CMakeLists.txt`
- Create: `bindings/java/src/main/java/tsp/solver/Version.java.in`
- Modify: `bindings/java/src/main/java/tsp/solver/NativeLibrary.java`
- Modify: `bindings/java/src/main/cpp/tsp_solver_java_jni.cpp`
- Modify: `bindings/java/src/test/java/tsp/solver/BindingTestMain.java`

- [ ] **Step 1: Write a failing Java mismatch regression**

Add a new test method to `bindings/java/src/test/java/tsp/solver/BindingTestMain.java` that validates the mismatch message through a test seam:

```java
private static void testVersionMismatchMessage() {
  expectThrows(IllegalStateException.class, new ThrowingRunnable() {
    @Override
    public void run() {
      NativeLibrary.ensureVersionMatch("0.1.0", "9.9.9");
    }
  }, "mismatched Java and native versions must fail");
}
```

Call it from `main()`.

- [ ] **Step 2: Run the Java test target to verify it fails first**

Run: `ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_java_tests`

Expected: FAIL because `ensureVersionMatch()` does not exist yet.

- [ ] **Step 3: Generate a Java version constant and add a JNI version accessor**

Update `bindings/java/CMakeLists.txt` to generate `Version.java` from `PROJECT_VERSION`, then compile that generated source with the rest of the Java API:

```cmake
set(TSP_SOLVER_JAVA_GENERATED_SOURCES_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated-java)
set(TSP_SOLVER_JAVA_VERSION_SOURCE
    ${TSP_SOLVER_JAVA_GENERATED_SOURCES_DIR}/tsp/solver/Version.java)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Version.java.in
               ${TSP_SOLVER_JAVA_VERSION_SOURCE} @ONLY)
```

Create `bindings/java/src/main/java/tsp/solver/Version.java.in`:

```java
package tsp.solver;

public final class Version {
  public static final String STRING = "@PROJECT_VERSION@";

  private Version() {}
}
```

Add a JNI-exported version getter in `bindings/java/src/main/cpp/tsp_solver_java_jni.cpp`:

```cpp
JNIEXPORT jstring JNICALL Java_tsp_solver_NativeLibrary_nativeVersionString(JNIEnv* env, jclass) {
  const char* version = tsp_solver_version_string();
  return env->NewStringUTF(version == nullptr ? "" : version);
}
```

- [ ] **Step 4: Enforce same-version matching in `NativeLibrary.load()`**

Extend `bindings/java/src/main/java/tsp/solver/NativeLibrary.java`:

```java
private static native String nativeVersionString();

static void ensureVersionMatch(String javaVersion, String nativeVersion) {
  if (javaVersion.equals(nativeVersion)) {
    return;
  }
  throw new IllegalStateException(
      "Java binding version " + javaVersion
          + " does not match native library version " + nativeVersion
          + ". Use the jar and native bundle from the same tsp_solver release.");
}
```

Call `ensureVersionMatch(Version.STRING, nativeVersionString())` immediately after successful native loading and before setting `loaded = true`.

- [ ] **Step 5: Run the Java mismatch regression again**

Run: `ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_java_tests`

Expected: PASS.

- [ ] **Step 6: Run the packaged Java smoke test**

Run: `ctest --preset <your-host-appropriate-preset> --output-on-failure -R "tsp_solver_java_tests_env|tsp_solver_java_package_smoke"`

Expected: PASS.

- [ ] **Step 7: Commit the Java compatibility enforcement**

```bash
git add bindings/java/CMakeLists.txt bindings/java/src/main/java/tsp/solver/Version.java.in bindings/java/src/main/java/tsp/solver/NativeLibrary.java bindings/java/src/main/cpp/tsp_solver_java_jni.cpp bindings/java/src/test/java/tsp/solver/BindingTestMain.java
git commit -m "feat: enforce Java native version compatibility"
```

### Task 4: Document the compatibility policy for users

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Add a failing doc check by identifying the exact missing section**

Re-read `README.md` and confirm it does not yet state the strict same-version policy in one dedicated place.

- [ ] **Step 2: Add a `Compatibility Policy` section near the binding docs**

Insert content like:

```md
## Compatibility Policy

- C++ consumers should use the headers, CMake package files, and native library from the same release archive.
- Python wheels must be used with the native library from the exact same release version.
- Java API jars must be used with the JNI and native bundle from the exact same release version.
- `TSP_SOLVER_ABI_VERSION` is maintainer-facing compatibility metadata; it is not currently a promise that different release artifacts can be mixed.
```

- [ ] **Step 3: Make the release asset pairing explicit**

Update the Python and Java sections to say:

```md
Use the wheel from the same release as the packaged native library.
Use the Java API jar and native bundle from the same release.
```

- [ ] **Step 4: Review the README for consistency**

Check that the README does not suggest cross-version mixing anywhere else.

- [ ] **Step 5: Commit the policy docs update**

```bash
git add README.md
git commit -m "docs: define release compatibility policy"
```

### Task 5: Run the full verification suite

**Files:**
- Modify: any files needed if verification reveals regressions

- [ ] **Step 1: Run the full native and binding suite**

Run: `ctest --preset <your-host-appropriate-preset> --output-on-failure`

Expected: all native, Python, and Java tests pass.

- [ ] **Step 2: Run focused packaging smoke verification again**

Run: `ctest --preset <your-host-appropriate-preset> --output-on-failure -R "tsp_solver_python_package_smoke|tsp_solver_java_package_smoke"`

Expected: PASS.

- [ ] **Step 3: Run formatting check if the JNI C++ file changed**

Run: `clang-format --dry-run --Werror bindings/java/src/main/cpp/tsp_solver_java_jni.cpp`

Expected: PASS.

- [ ] **Step 4: Commit any final verification fixes**

```bash
git add <any changed files>
git commit -m "fix: finish release compatibility enforcement"
```
