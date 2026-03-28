# Binding Distribution Design

## Goal

Ship the Python and Java bindings as consumable release artifacts rather than build-tree-only integrations.

## Scope

- Build a platform wheel for Python that bundles the native `tsp_solver` library.
- Build a platform-independent Java API jar plus a platform native bundle containing the JNI bridge and core library.
- Verify both artifact types in CI and publish them from the release workflow.
- Update user-facing docs and branch protection guidance to reflect the new checks.

## Non-Goals

- Publishing to PyPI.
- Publishing to Maven Central.
- Signing, SBOM generation, or package-manager metadata beyond what is needed for GitHub Release artifacts.

## Design

### Versioning

All binding artifacts use the top-level CMake project version as the single source of truth.

### Python Distribution

The Python bindings are packaged from `bindings/python/` using setuptools. The wheel bundles the built native library beside the Python package so `tsp_solver._native` can load it through its existing package-local search path. Release packaging consumes a previously built native library path supplied by the CMake build.

### Java Distribution

The Java bindings produce one API jar that contains the compiled `tsp.solver.*` classes. Each platform also produces a native bundle that contains the JNI bridge, the core `tsp_solver` shared library, and a small README describing how to point the loader at the JNI bridge.

### Verification

CI keeps the current build and test matrix, then adds a separate packaging job that validates the real install/consumer experience:

- build and install a Python wheel into a clean virtual environment, then run a smoke solve;
- build a Java jar and native bundle, then run a smoke solve using only those packaged artifacts.

### Release Layout

The release workflow continues publishing the native install archive and additionally uploads:

- one Python wheel per platform;
- one Java API jar;
- one Java native bundle per platform.

## Risks

- Wheel building must copy the correct platform native library into the wheel at build time.
- Java native bundle layout must match the assumptions in `NativeLibrary.load()` and the host dynamic loader.
- Windows local builds need explicit tool paths because this environment does not expose CMake/Ninja on the default shell PATH.

## Acceptance Criteria

- A release build emits the expected Python and Java artifacts with versioned names.
- The packaging CI job verifies Python install-and-import plus Java jar-plus-native-bundle execution on all three platforms.
- README instructions describe the release artifacts as the primary consumption path.
