# Release Compatibility Policy Design

## Goal

Define a clear release-quality compatibility contract for the C++, Python, and Java deliverables without expanding scope to manylinux, PyPI, or Maven Central.

## Scope

- Document which artifacts are intended for C++, Python, and Java consumers.
- Define the version-matching rule between bindings and native libraries.
- Add runtime checks so Python and Java fail early when artifacts from different releases are mixed.
- Add regression tests that verify the mismatch failures are explicit and actionable.

## Non-Goals

- Supporting cross-version mixing between bindings and native libraries.
- Supporting manylinux or other broader Linux portability baselines beyond current runner-validated artifacts.
- Publishing to PyPI or Maven Central.
- Treating `TSP_SOLVER_ABI_VERSION` as a public promise that different binding or package versions can be mixed.

## Compatibility Contract

### Core Rule

The project uses a strict same-version policy for released artifacts.

- A Python wheel must be used with the native library from the same release version.
- A Java API jar must be used with the JNI and native bundle from the same release version.
- A C++ consumer should use the install archive produced by the same release version as the headers and library package it consumes.

The project does not promise that `0.1.0` bindings can work with `0.1.1` native artifacts, even if the ABI has not changed.

### C API and ABI

The C API in `include/tsp_solver/c_api.h` remains the stable native boundary. `TSP_SOLVER_ABI_VERSION` is still useful for maintainers when making compatibility decisions, but it is not the public compatibility contract for Python or Java consumers at this stage.

During the current early release phase, artifact compatibility is determined by exact release version, not by ABI version alone.

## Runtime Behavior

### Python

The Python package exposes its package version through package metadata. After the native library is loaded, the binding layer compares the package version with `tsp_solver_version_string()` from the native library.

If the versions do not match exactly, the Python bindings raise a clear error before making solver calls. The error message must include:

- the Python package version;
- the native library version;
- guidance that both artifacts must come from the same release.

### Java

The Java API jar exposes its version through a generated or packaged version constant. After `NativeLibrary.load()` succeeds, the Java layer compares the jar version with `tsp_solver_version_string()` from the native library through JNI.

If the versions do not match exactly, the Java bindings throw a clear exception before solver operations continue. The message must include:

- the Java API jar version;
- the native library version;
- guidance that the jar and native bundle must come from the same release.

### C++

The native C++ and C API delivery stays documentation-driven rather than loader-driven. The project documents that consumers should keep headers, CMake package files, and shared libraries from the same release archive together.

## Testing Strategy

The project keeps all existing success-path smoke tests and adds focused mismatch tests.

### Python mismatch test

Add a regression test that exercises the Python version comparison logic and verifies that a mismatch raises a clear error. The test can use a small test seam or controlled override rather than constructing historical release artifacts.

### Java mismatch test

Add a regression test that exercises the Java version comparison logic and verifies that a mismatch throws a clear exception. As with Python, the test should validate the comparison and message rather than requiring multiple real release artifacts.

The purpose of these tests is not to simulate a full historical release matrix. The purpose is to prove that mismatched artifacts fail early and explain how to fix the problem.

## Documentation Updates

Update the top-level README with a short `Compatibility Policy` section that states:

- the current policy is strict same-version compatibility;
- Python and Java artifacts are not supported across release versions;
- `TSP_SOLVER_ABI_VERSION` is maintainers' metadata today, not a consumer mixing guarantee.

Release-facing documentation should also make it clear which asset type is intended for each consumer:

- C++ consumers use the native install archive;
- Python consumers use the wheel from the same release;
- Java consumers use the API jar and matching native bundle from the same release.

## Acceptance Criteria

- README clearly states the strict same-version compatibility rule.
- Python bindings fail early with a clear mismatch error when package and native versions differ.
- Java bindings fail early with a clear mismatch error when jar and native versions differ.
- Regression tests cover both mismatch paths.
- Release asset descriptions make the expected artifact pairing explicit for C++, Python, and Java consumers.
