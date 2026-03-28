# TSP Solver Java Bindings Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a first Java binding layer that exposes `Model`, `Options`, `Result`, `Algorithm`, and `Status` over the stable C API.

**Architecture:** Keep Java thin and C-driven. Java classes own opaque `long` native handles and expose an idiomatic `AutoCloseable` API; a single JNI bridge library translates those calls into the C API and centralizes error handling. CMake remains the top-level build entry point and drives Java compilation, JNI compilation, and Java test execution.

**Tech Stack:** Java 8, `javac`, `java`, JNI, CMake, CTest, existing native C API.

---

## File Structure

- Create: `bindings/java/CMakeLists.txt` - Java compile commands, JNI library target, and CTest wiring.
- Create: `bindings/java/src/main/java/tsp/solver/Algorithm.java` - Java enum mirroring the C algorithm enum.
- Create: `bindings/java/src/main/java/tsp/solver/Status.java` - Java enum mirroring the C status enum.
- Create: `bindings/java/src/main/java/tsp/solver/SolverNativeException.java` - Java exception for native failures.
- Create: `bindings/java/src/main/java/tsp/solver/NativeLibrary.java` - library loading helper.
- Create: `bindings/java/src/main/java/tsp/solver/NativeResource.java` - package-private base for `nativeHandle` ownership.
- Create: `bindings/java/src/main/java/tsp/solver/Model.java` - Java model wrapper.
- Create: `bindings/java/src/main/java/tsp/solver/Options.java` - Java options wrapper.
- Create: `bindings/java/src/main/java/tsp/solver/Result.java` - Java result wrapper.
- Create: `bindings/java/src/main/java/tsp/solver/Solver.java` - convenience `solve` entry point.
- Create: `bindings/java/src/test/java/tsp/solver/BindingTestMain.java` - plain Java test runner.
- Create: `bindings/java/src/main/cpp/tsp_solver_java_jni.cpp` - JNI bridge implementation.
- Modify: `CMakeLists.txt` - include the Java bindings build when testing is enabled.
- Modify: `README.md` - document local Java binding build/test usage.

### Task 1: Write the Java tests and the public Java API skeleton

**Files:**
- Create: `bindings/java/src/test/java/tsp/solver/BindingTestMain.java`
- Create: `bindings/java/src/main/java/tsp/solver/Algorithm.java`
- Create: `bindings/java/src/main/java/tsp/solver/Status.java`
- Create: `bindings/java/src/main/java/tsp/solver/SolverNativeException.java`
- Create: `bindings/java/src/main/java/tsp/solver/NativeLibrary.java`
- Create: `bindings/java/src/main/java/tsp/solver/NativeResource.java`
- Create: `bindings/java/src/main/java/tsp/solver/Model.java`
- Create: `bindings/java/src/main/java/tsp/solver/Options.java`
- Create: `bindings/java/src/main/java/tsp/solver/Result.java`
- Create: `bindings/java/src/main/java/tsp/solver/Solver.java`

- [ ] **Step 1: Write the failing Java test runner**

Create `bindings/java/src/test/java/tsp/solver/BindingTestMain.java`:

```java
package tsp.solver;

import java.util.Arrays;

public final class BindingTestMain {
  public static void main(String[] args) {
    testSolveCompleteGraph();
    testValidationError();
    testRangeError();
    testCloseIsIdempotent();
  }

  private static void testSolveCompleteGraph() {
    try (Model model = new Model(); Options options = new Options()) {
      options.setAlgorithm(Algorithm.LOCAL_SEARCH_2OPT);

      for (int i = 0; i < 4; ++i) {
        assertEquals(i, model.addNode(), "node id");
      }

      long[][] distances = {
          {0, 2, 9, 10},
          {1, 0, 6, 4},
          {15, 7, 0, 8},
          {6, 3, 12, 0},
      };

      for (int from = 0; from < distances.length; ++from) {
        for (int to = 0; to < distances[from].length; ++to) {
          model.setDistance(from, to, distances[from][to]);
        }
      }

      model.validate();
      try (Result result = model.solve(options)) {
        assertEquals(Status.FEASIBLE, result.getStatus(), "status");
        assertEquals(4, result.getTourSize(), "tour size");
        int[] tour = result.getTour();
        Arrays.sort(tour);
        assertTrue(Arrays.equals(tour, new int[] {0, 1, 2, 3}), "tour permutation");
      }
    }
  }

  private static void testValidationError() {
    try (Model model = new Model()) {
      model.addNode();
      model.addNode();
      expectThrows(IllegalStateException.class, new ThrowingRunnable() {
        @Override
        public void run() {
          model.validate();
        }
      }, "incomplete model must fail validation");
    }
  }

  private static void testRangeError() {
    try (Model model = new Model()) {
      model.addNode();
      expectThrows(IndexOutOfBoundsException.class, new ThrowingRunnable() {
        @Override
        public void run() {
          model.setDistance(1, 0, 5);
        }
      }, "out of range node id must fail");
    }
  }

  private static void testCloseIsIdempotent() {
    Model model = new Model();
    model.close();
    model.close();
    expectThrows(IllegalStateException.class, new ThrowingRunnable() {
      @Override
      public void run() {
        model.addNode();
      }
    }, "closed model must reject further use");
  }

  private static void assertTrue(boolean condition, String message) {
    if (!condition) {
      throw new AssertionError(message);
    }
  }

  private static void assertEquals(Object expected, Object actual, String message) {
    if (!expected.equals(actual)) {
      throw new AssertionError(message + ": expected=" + expected + " actual=" + actual);
    }
  }

  private static void expectThrows(Class<? extends Throwable> type,
                                   ThrowingRunnable runnable,
                                   String message) {
    try {
      runnable.run();
    } catch (Throwable error) {
      if (type.isInstance(error)) {
        return;
      }
      throw new AssertionError(message + ": wrong exception " + error, error);
    }
    throw new AssertionError(message + ": no exception thrown");
  }

  private interface ThrowingRunnable {
    void run();
  }
}
```

- [ ] **Step 2: Run Java compilation before the API exists**

Run:

```bash
javac -d build/java-test-classes bindings/java/src/test/java/tsp/solver/BindingTestMain.java
```

Expected: fail with `cannot find symbol` errors for `Model`, `Options`, `Result`, `Algorithm`, and `Status`.

- [ ] **Step 3: Add the Java API skeleton**

Create these Java files:

`bindings/java/src/main/java/tsp/solver/Algorithm.java`

```java
package tsp.solver;

public enum Algorithm {
  DEFAULT(0),
  LOCAL_SEARCH_2OPT(1);

  private final int nativeValue;

  Algorithm(int nativeValue) {
    this.nativeValue = nativeValue;
  }

  int nativeValue() {
    return nativeValue;
  }
}
```

`bindings/java/src/main/java/tsp/solver/Status.java`

```java
package tsp.solver;

public enum Status {
  NOT_SOLVED(0),
  FEASIBLE(1),
  OPTIMAL(2),
  INFEASIBLE(3),
  TIME_LIMIT(4),
  INVALID_MODEL(5),
  INTERNAL_ERROR(6);

  private final int nativeValue;

  Status(int nativeValue) {
    this.nativeValue = nativeValue;
  }

  static Status fromNative(int nativeValue) {
    for (Status status : values()) {
      if (status.nativeValue == nativeValue) {
        return status;
      }
    }
    throw new SolverNativeException("Unknown native status: " + nativeValue);
  }
}
```

`bindings/java/src/main/java/tsp/solver/SolverNativeException.java`

```java
package tsp.solver;

public final class SolverNativeException extends RuntimeException {
  public SolverNativeException(String message) {
    super(message);
  }
}
```

`bindings/java/src/main/java/tsp/solver/NativeLibrary.java`

```java
package tsp.solver;

public final class NativeLibrary {
  private static volatile boolean loaded;

  private NativeLibrary() {}

  public static void load() {
    if (loaded) {
      return;
    }
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }
}
```

`bindings/java/src/main/java/tsp/solver/NativeResource.java`

```java
package tsp.solver;

abstract class NativeResource implements AutoCloseable {
  protected long nativeHandle;

  protected final long requireHandle() {
    if (nativeHandle == 0) {
      throw new IllegalStateException(getClass().getSimpleName() + " is closed");
    }
    return nativeHandle;
  }
}
```

`bindings/java/src/main/java/tsp/solver/Model.java`

```java
package tsp.solver;

public final class Model extends NativeResource {
  public Model() {
    NativeLibrary.load();
  }

  public int addNode() {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  public void setDistance(int from, int to, long distance) {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  public void validate() {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  public Result solve(Options options) {
    return Solver.solve(this, options);
  }

  @Override
  public void close() {
    nativeHandle = 0;
  }
}
```

`bindings/java/src/main/java/tsp/solver/Options.java`

```java
package tsp.solver;

public final class Options extends NativeResource {
  public Options() {
    NativeLibrary.load();
  }

  public void setTimeLimitMs(long value) {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  public void setRandomSeed(long value) {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  public void setAlgorithm(Algorithm algorithm) {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  @Override
  public void close() {
    nativeHandle = 0;
  }
}
```

`bindings/java/src/main/java/tsp/solver/Result.java`

```java
package tsp.solver;

public final class Result extends NativeResource {
  Result() {
    NativeLibrary.load();
  }

  public Status getStatus() {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  public long getObjective() {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  public int getTourSize() {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  public int[] getTour() {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }

  @Override
  public void close() {
    nativeHandle = 0;
  }
}
```

`bindings/java/src/main/java/tsp/solver/Solver.java`

```java
package tsp.solver;

public final class Solver {
  private Solver() {}

  public static Result solve(Model model, Options options) {
    throw new UnsatisfiedLinkError("JNI bridge not implemented yet");
  }
}
```

- [ ] **Step 4: Re-run Java compilation to verify the API compiles**

Run:

```bash
javac -d build/java-test-classes \
  bindings/java/src/main/java/tsp/solver/*.java \
  bindings/java/src/test/java/tsp/solver/BindingTestMain.java
```

Expected: compile succeeds; running the test class would still fail because native methods are not wired yet.

### Task 2: Add the JNI bridge library and connect Java calls to the C API

**Files:**
- Create: `bindings/java/src/main/cpp/tsp_solver_java_jni.cpp`
- Modify: `bindings/java/src/main/java/tsp/solver/NativeLibrary.java`
- Modify: `bindings/java/src/main/java/tsp/solver/NativeResource.java`
- Modify: `bindings/java/src/main/java/tsp/solver/Model.java`
- Modify: `bindings/java/src/main/java/tsp/solver/Options.java`
- Modify: `bindings/java/src/main/java/tsp/solver/Result.java`
- Modify: `bindings/java/src/main/java/tsp/solver/Solver.java`

- [ ] **Step 1: Run the Java test class before JNI exists**

Run:

```bash
java -cp build/java-test-classes tsp.solver.BindingTestMain
```

Expected: fail with `UnsatisfiedLinkError` from `NativeLibrary.load()`.

- [ ] **Step 2: Replace the Java stubs with native-backed wrappers**

Update `NativeLibrary.java` so it tries these load sources in order:

```java
String propertyPath = System.getProperty("tsp.solver.library.path");
String envPath = System.getenv("TSP_SOLVER_LIBRARY_PATH");
```

Then:

```java
if (propertyPath != null && !propertyPath.isEmpty()) {
  System.load(propertyPath);
} else if (envPath != null && !envPath.isEmpty()) {
  System.load(envPath);
} else {
  System.loadLibrary("tsp_solver_java_jni");
}
```

Update `NativeResource.java` to provide an idempotent final `close()` method that calls an abstract `release(long handle)` hook.

Update `Model.java`, `Options.java`, and `Result.java` so they declare private static native methods for create/destroy and public instance methods that delegate through `requireHandle()`. For example, `Model.java` should follow this shape:

```java
package tsp.solver;

public final class Model extends NativeResource {
  public Model() {
    NativeLibrary.load();
    nativeHandle = nativeCreate();
  }

  public int addNode() {
    return nativeAddNode(requireHandle());
  }

  public void setDistance(int from, int to, long distance) {
    nativeSetDistance(requireHandle(), from, to, distance);
  }

  public void validate() {
    nativeValidate(requireHandle());
  }

  public Result solve(Options options) {
    return Solver.solve(this, options);
  }

  @Override
  protected void release(long handle) {
    nativeDestroy(handle);
  }

  private static native long nativeCreate();
  private static native void nativeDestroy(long handle);
  private static native int nativeAddNode(long handle);
  private static native void nativeSetDistance(long handle, int from, int to, long distance);
  private static native void nativeValidate(long handle);
}
```

Use these concrete declarations for the remaining Java files:

`Options.java`

```java
private static native long nativeCreate();
private static native void nativeDestroy(long handle);
private static native void nativeSetTimeLimitMs(long handle, long value);
private static native void nativeSetRandomSeed(long handle, long value);
private static native void nativeSetAlgorithm(long handle, int algorithmValue);
```

`Result.java`

```java
private static native void nativeDestroy(long handle);
private static native int nativeGetStatus(long handle);
private static native long nativeGetObjective(long handle);
private static native int nativeGetTourSize(long handle);
private static native int[] nativeGetTour(long handle);
```

`Solver.java`

```java
private static native long nativeSolve(long modelHandle, long optionsHandle);
```

- [ ] **Step 3: Implement the JNI bridge**

Create `bindings/java/src/main/cpp/tsp_solver_java_jni.cpp` and implement JNI functions for all native Java entry points. The file should:

```cpp
#include <jni.h>

#include "tsp_solver/c_api.h"
```

It should also provide helpers that:

- convert `jlong` to the correct opaque C pointer type,
- throw `IllegalArgumentException`, `IndexOutOfBoundsException`, `IllegalStateException`, or `tsp/solver/SolverNativeException`,
- throw `IllegalStateException` when a `0` handle is used,
- create `jintArray` for `Result.getTour()`.

Map the C error codes exactly as specified in the design spec.

- [ ] **Step 4: Re-run Java compilation and the Java test class**

Run:

```bash
javac -h build/java-generated -d build/java-test-classes \
  bindings/java/src/main/java/tsp/solver/*.java \
  bindings/java/src/test/java/tsp/solver/BindingTestMain.java
```

Then, after the JNI bridge library is built, run:

```bash
java -Dtsp.solver.library.path=build/windows-msvc-debug/tsp_solver_java_jni.dll \
  -cp build/java-test-classes tsp.solver.BindingTestMain
```

Expected: Java tests pass.

### Task 3: Wire Java compilation, JNI compilation, and test execution into CMake

**Files:**
- Create: `bindings/java/CMakeLists.txt`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Add the failing CMake wiring**

Create `bindings/java/CMakeLists.txt` with these responsibilities:

```cmake
find_package(Java REQUIRED COMPONENTS Development)
find_package(JNI REQUIRED)

set(TSP_SOLVER_JAVA_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java)
set(TSP_SOLVER_JAVA_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/test/java)
set(TSP_SOLVER_JAVA_CLASSES_DIR ${CMAKE_BINARY_DIR}/java/classes)
set(TSP_SOLVER_JAVA_HEADERS_DIR ${CMAKE_BINARY_DIR}/java/generated)
```

Add a custom command that runs `javac -h` over the Java sources and test runner into `${TSP_SOLVER_JAVA_CLASSES_DIR}` and `${TSP_SOLVER_JAVA_HEADERS_DIR}`.

Add a shared library target:

```cmake
add_library(tsp_solver_java_jni SHARED bindings/java/src/main/cpp/tsp_solver_java_jni.cpp)
target_include_directories(tsp_solver_java_jni PRIVATE ${JNI_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tsp_solver_java_jni PRIVATE tsp_solver)
```

Add a CTest entry that runs:

```cmake
add_test(
  NAME tsp_solver_java_tests
  COMMAND ${Java_JAVA_EXECUTABLE}
          -Dtsp.solver.library.path=$<TARGET_FILE:tsp_solver_java_jni>
          -cp ${TSP_SOLVER_JAVA_CLASSES_DIR}
          tsp.solver.BindingTestMain)
```

- [ ] **Step 2: Reconfigure and run the Java CTest entry before the wiring is complete**

Run:

```bash
cmake --preset windows-msvc-debug
ctest --preset windows-msvc-debug --output-on-failure -R tsp_solver_java_tests
```

Expected: fail before the Java targets and test entry are fully wired.

- [ ] **Step 3: Include the Java bindings directory in the root build**

Update the root `CMakeLists.txt` so `bindings/java` is added when testing is enabled:

```cmake
if(BUILD_TESTING)
  add_subdirectory(tests)
  add_subdirectory(bindings/python)
  add_subdirectory(bindings/java)
endif()
```

- [ ] **Step 4: Re-run the Java CTest entry**

Run:

```bash
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug --output-on-failure -R tsp_solver_java_tests
```

Expected: the Java classes compile, the JNI bridge builds, and the Java test runner passes.

### Task 4: Document local Java usage and run the full verification set

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Add a Java bindings section to the README**

Document the source locations and the CTest entry. Add text like:

```md
## Java bindings

The Java bindings live under `bindings/java/` and are built through CMake. Run the Java binding tests with:

```bash
ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_java_tests
```
```

- [ ] **Step 2: Run the full verification set**

Run:

```bash
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug --output-on-failure
```

Expected: all native, Python, package smoke, and Java tests pass.

- [ ] **Step 3: Commit the finished Java binding slice**

```bash
git add CMakeLists.txt README.md bindings/java docs/superpowers/plans/2026-03-28-tsp-solver-java-bindings.md
git commit -m "feat: add Java bindings over the C API"
```
