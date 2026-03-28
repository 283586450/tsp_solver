# Repository Guidelines

This repository currently has no checked-in source files or build scripts in the workspace snapshot, so these instructions are intentionally conservative and aimed at a cross-platform C++ library with Python and Java bindings.

## Scope

- Prefer small, focused changes.
- Keep the core C++ library portable across Linux, macOS, and Windows.
- Treat Python and Java bindings as thin layers over a stable native core.
- If you add new tooling, document the exact command in this file.

## Instruction Sources

- No `.cursor/rules/` files were present in the workspace snapshot.
- No `.cursorrules` file was present in the workspace snapshot.
- No `.github/copilot-instructions.md` file was present in the workspace snapshot.
- If any of those files are added later, update this document so the guidance is not duplicated or contradictory.

## Build System Assumptions

- Prefer `CMake` as the top-level build system.
- Prefer a generator like Ninja for local development and CI.
- Keep compiler-specific flags centralized in CMake, not scattered across scripts.
- Avoid requiring developers to memorize platform-specific build commands.
- Prefer `CMakePresets.json` for day-to-day configure/build/test commands when available.

## Build Commands

If the project uses the expected CMake layout:

```bash
cmake --list-presets
cmake --preset <your-host-appropriate-preset>
cmake --build --preset <your-host-appropriate-preset>
ctest --preset <your-host-appropriate-preset> --output-on-failure
```

If presets exist, prefer them:

```bash
cmake --list-presets
cmake --preset <your-host-appropriate-preset>
cmake --build --preset <your-host-appropriate-preset>
ctest --preset <your-host-appropriate-preset> --output-on-failure
```

For a clean rebuild, remove the build directory instead of editing generated files.

## Test Commands

Prefer `ctest` for native tests:

```bash
ctest --preset <your-host-appropriate-preset> --output-on-failure
```

Run a single C++ test by name:

```bash
ctest --preset <your-host-appropriate-preset> --output-on-failure -R <test_name>
```

Run one test executable directly when you know the binary path:

```bash
./build/<your-host-appropriate-preset>/path/to/test_binary --gtest_filter=SuiteName.TestName
```

If the project uses GoogleTest, prefer `--gtest_filter` for one case and `-R` for one binary.

If Python tests exist:

```bash
pytest
pytest tests/test_file.py
pytest -k test_name
```

If Java tests exist with Maven:

```bash
mvn test
mvn -Dtest=ClassName test
```

If Java tests exist with Gradle:

```bash
gradle test
gradle test --tests ClassName
```

## Lint and Formatting

- Run formatting before review; do not rely on CI to fix style.
- Prefer a single formatter configuration shared by all contributors.
- For C++, use `clang-format` if available.
- For C++, use `clang-tidy` when the project defines a supported checks set.
- For Python, use the project’s configured formatter/linter if present (`ruff`, `black`, `isort`, or equivalent).
- For Java, use the project’s configured formatter/linter if present (`spotless`, `checkstyle`, or equivalent).

Example commands:

```bash
clang-format -i <file>
clang-tidy <file> -- -Iinclude
```

- If `.clang-format` and `.clang-tidy` exist, keep them in sync with CMake and CI.

## C++ Style

- Prefer modern C++ and the newest standard the project supports consistently.
- Keep headers self-contained and source files minimal.
- Include what you use; do not depend on transitive includes.
- Avoid `using namespace` in headers.
- Prefer `namespace`-scoped utilities over globals.
- Use `std::string_view` for read-only string parameters when lifetime is clear.
- Prefer `std::span` for non-owning contiguous ranges.
- Prefer RAII for resource management.
- Prefer `const` by default for locals, parameters, and member functions when practical.
- Use explicit `nullptr` instead of `NULL` or `0` for pointers.
- Avoid exceptions across ABI boundaries; convert errors at language bindings.
- Keep template metaprogramming readable; prefer simple code over clever code.

## Naming Conventions

- Types: `PascalCase`.
- Functions and variables: `snake_case` unless the existing codebase uses a different established convention.
- Constants: `kCamelCase` or `k_snake_case`, but stay consistent within a module.
- Macros: `UPPER_SNAKE_CASE`; avoid macros unless necessary.
- Namespaces: short, lower-case, and project-scoped.

## Imports and Includes

- Order includes as: local project headers, third-party headers, standard library headers, unless the formatter enforces a different order.
- Keep include lists minimal.
- Remove unused includes immediately.
- Prefer forward declarations in headers when they reduce coupling.

## Types and Interfaces

- Prefer strongly typed enums (`enum class`).
- Prefer fixed-width integer types for serialized or cross-language data.
- Avoid implicit narrowing conversions.
- Make ownership obvious in interfaces.
- Use `std::optional` for optional values and `std::expected` or a project error type for recoverable failures when supported.
- Keep public ABI stable; minimize exposed implementation details.

## Error Handling

- Fail fast on programmer errors; report recoverable errors explicitly.
- Do not swallow errors silently.
- Preserve original error context when translating between C++, Python, and Java.
- Map native failures to idiomatic errors in each binding layer.
- Prefer clear error messages with the failing operation and relevant parameters.

## Python Binding Guidance

- Keep Python wrappers thin and testable.
- Prefer `pybind11` if the project uses C++ bindings.
- Release the GIL around long-running native work when safe.
- Convert native exceptions to Python exceptions at the boundary.
- Add Python tests for import, argument validation, and error translation.

## Java Binding Guidance

- Prefer a thin JNI layer over duplicated algorithm logic.
- Keep JNI signatures and native ownership rules documented.
- Validate all Java inputs before crossing into native code.
- Translate native errors into Java exceptions with actionable messages.
- Add Java tests for loading, basic calls, and native error paths.

## Testing Expectations

- Add or update tests for every behavior change.
- Favor deterministic tests with small fixtures.
- Add regression tests for every bug fix.
- Test success paths, invalid inputs, and boundary cases.
- For cross-language features, test the native core and the binding layer separately.

## Change Hygiene

- Do not mix unrelated refactors with feature work.
- Keep pull requests small enough to review quickly.
- Update docs when public behavior changes.
- Prefer compatibility-preserving changes over breaking ones.

## When in Doubt

- Follow the existing local style if it conflicts with this file.
- If build or test commands differ from the assumptions above, update this file after verifying them.
- If you add project-specific tooling, place the canonical command here so future agents can find it quickly.
