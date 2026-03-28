# Binding Distribution Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build distributable Python and Java binding artifacts, verify them in CI, and publish them from GitHub Releases.

**Architecture:** Keep CMake as the native build entrypoint, add lightweight packaging layers in `bindings/python` and `bindings/java`, and verify packaged consumption separately from source-tree tests. Python ships a self-contained platform wheel, while Java ships a platform-independent API jar plus per-platform native bundles.

**Tech Stack:** CMake, setuptools/build, Python unittest/venv, Java `jar`, GitHub Actions.

---

### Task 1: Add Python packaging metadata and bundle hooks

**Files:**
- Create: `bindings/python/pyproject.toml`
- Create: `bindings/python/setup.py`
- Create: `bindings/python/README.md`
- Modify: `bindings/python/tsp_solver/__init__.py`
- Modify: `bindings/python/tsp_solver/_native.py`

- [ ] Define setuptools metadata and wheel backend in `bindings/python/pyproject.toml`.
- [ ] Add a `setup.py` hook that copies the built native library into the package when `TSP_SOLVER_PYTHON_NATIVE_LIBRARY` is provided.
- [ ] Keep package-local loading as the primary runtime path and environment variables as fallback.

### Task 2: Add Python packaged-consumer verification

**Files:**
- Create: `tests/python/package_smoke.py`
- Modify: `bindings/python/CMakeLists.txt`
- Modify: `.github/workflows/ci.yml`

- [ ] Add a smoke script that imports the installed wheel and solves a small graph.
- [ ] Add CMake helper targets/tests that build a wheel and install it into a clean virtual environment.
- [ ] Wire the same flow into CI on every platform.

### Task 3: Add Java jar and native bundle targets

**Files:**
- Modify: `bindings/java/CMakeLists.txt`
- Create: `bindings/java/README.md`

- [ ] Add a jar packaging target using the compiled class output.
- [ ] Add a native bundle target that copies the JNI bridge, core native library, and README into a versioned layout.

### Task 4: Add Java packaged-consumer verification

**Files:**
- Create: `tests/java/package_smoke/PackageSmokeMain.java`
- Modify: `bindings/java/CMakeLists.txt`
- Modify: `.github/workflows/ci.yml`

- [ ] Add a smoke main that runs from the packaged jar and native bundle.
- [ ] Add CMake helper targets/tests that execute the smoke main against packaged artifacts only.
- [ ] Reuse the same commands in CI.

### Task 5: Publish artifacts and update docs

**Files:**
- Modify: `.github/workflows/release.yml`
- Modify: `README.md`
- Modify: `docs/branch-protection.md`
- Modify: `AGENTS.md`

- [ ] Extend release packaging to upload Python wheels, Java API jar, and Java native bundles.
- [ ] Update docs to make packaged artifact consumption the default user path.
- [ ] Document the new package verification command in `AGENTS.md`.

### Task 6: Verify the complete matrix

**Files:**
- Modify as needed based on verification output

- [ ] Run the local Windows build and test suite.
- [ ] Run the Python packaging smoke flow locally.
- [ ] Run the Java packaging smoke flow locally.
- [ ] Re-run formatting checks for touched C++/header files if any changed.
