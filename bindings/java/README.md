# tsp_solver Java 绑定

发布构建会产出两个面向 Java 的产物：

- `tsp-solver-java-<version>.jar`，包含公开的 `tsp.solver.*` API 类
- `tsp-solver-java-native-<os>-<arch>-<version>`，包含 `tsp_solver_java_jni` 以及核心
  `tsp_solver` 共享库

请使用同一发布版本的 API jar 和 native bundle。

在使用打包后的产物时，请将 `-Dtsp.solver.library.path` 指向 native bundle 中的 JNI 桥接库。

Linux native bundle 当前在匹配的 GitHub 托管 runner 镜像上验证，尚未承诺更广泛的 glibc 兼容基线。
