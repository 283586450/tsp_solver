# tsp_solver Java bindings

Release builds produce two Java-facing artifacts:

- `tsp-solver-java-<version>.jar` with the public `tsp.solver.*` API classes
- `tsp-solver-java-native-<os>-<arch>-<version>` containing `tsp_solver_java_jni`
  plus the core `tsp_solver` shared library

Use the API jar and native bundle from the same release version.

Point `-Dtsp.solver.library.path` at the JNI bridge inside the native bundle when
running against packaged artifacts.

The Linux native bundle is currently validated on the matching GitHub-hosted
runner image and does not yet promise a broader glibc compatibility baseline.
