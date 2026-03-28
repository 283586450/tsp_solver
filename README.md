# TSP Solver

面向旅行商问题的可移植 C++ 框架。

这个仓库按长期扩展来组织：
- 一个原生 C++ 核心，用于问题建模和求解逻辑
- 可从局部搜索逐步扩展到更丰富启发式算法的算法模块
- 构建在稳定原生 API 之上的轻量 Python 和 Java 绑定

## 项目结构

- `include/` - 原生 API 的公共头文件
- `src/` - 核心实现和算法代码
- `tests/` - 原生测试
- `bindings/` - Python 和 Java 集成层
- `docs/` - 工作流说明、分支保护建议和规划文档

## 当前状态

仓库当前提供：
- 基于 CMake 的原生构建
- 通过共享 API 暴露多种算法的原生 C++ 求解核心
- 通过发布产物消费的 Python 和 Java 绑定
- GitHub Actions CI 和发布工作流

## C API

第一个公开的原生边界位于 `include/tsp_solver/c_api.h`。
库默认构建为共享库，并提供稳定的 C 安装包和导出目标（`tsp_solver::tsp_solver`）。
简短的用法示例和结果处理说明请见 `docs/c_api.md`。使用方可以通过
`find_package(tsp_solver <release-version> EXACT CONFIG REQUIRED)` 并链接 `tsp_solver::tsp_solver`。

## 兼容性策略

- C++ 使用方应使用同一发布归档中的头文件、CMake 包文件和原生库。
- Python wheel 必须与同一发布版本的原生库配套使用。
- Java API jar 必须与同一发布版本的 JNI 和原生 bundle 配套使用。
- `TSP_SOLVER_ABI_VERSION` 是面向维护者的兼容性元数据；它目前不代表不同发布产物可以混用。

## Python 绑定

Python 包位于 `bindings/python/tsp_solver`，并封装了 C API。
本地开发时，请将 `TSP_SOLVER_LIBRARY_PATH` 指向已构建的共享库，并在运行前把 `bindings/python`
加入 `PYTHONPATH`：

```bash
python -m unittest discover -s tests/python -p "test_*.py" -v
```

构建可分发的 wheel：

```bash
cmake --build --preset <your-host-appropriate-preset> --target tsp_solver_python_wheel
```

请使用与其他产物同一发布版本的 wheel；平台 wheel 已经内置了匹配的原生库。

当前发布 wheel 在 GitHub 托管 runner 上构建，并在对应 runner 镜像上验证。
Linux wheel 目前尚未修复到 manylinux 基线。

## Java 绑定

Java 绑定位于 `bindings/java/`，并通过 CMake 构建为位于 C API 之上的轻量 JNI 层。
运行 Java 绑定测试：

```bash
ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_java_tests
```

本地开发时，请将 `TSP_SOLVER_JAVA_LIBRARY_PATH` 指向已构建的 JNI 桥接库，或者传入
`-Dtsp.solver.library.path=/absolute/path/to/tsp_solver_java_jni`。

构建面向发布的 Java 产物：

```bash
cmake --build --preset <your-host-appropriate-preset> --target tsp_solver_java_jar tsp_solver_java_native_bundle
```

请使用同一发布版本的 Java API jar 和 native bundle。

Linux Java native bundle 当前在匹配的 `ubuntu-latest` runner 镜像上验证，
尚未声明更广泛的 glibc 兼容基线。

## 构建与测试

```bash
cmake --list-presets
cmake --preset <your-host-appropriate-preset>
cmake --build --preset <your-host-appropriate-preset>
ctest --preset <your-host-appropriate-preset> --output-on-failure
```

运行单个测试：

```bash
ctest --preset <your-host-appropriate-preset> --output-on-failure -R tsp_solver_tests
```

运行打包后的绑定冒烟测试：

```bash
ctest --preset <your-host-appropriate-preset> --output-on-failure -R "tsp_solver_python_package_smoke|tsp_solver_java_package_smoke"
```

## 工作流说明

- 分支保护建议见 `docs/branch-protection.md`。
- CI 定义在 `.github/workflows/ci.yml`。
- 发布打包定义在 `.github/workflows/release.yml`。
- 格式化规则定义在 `.clang-format`。
- 静态分析默认配置位于 `.clang-tidy`。

## 示例

`examples/README.md` 提供了 C++、Python 和 Java 的同场景比较示例，三种语言都使用同一份 20 城市距离矩阵。

## 扩展路线图

- 扩展局部搜索，增加更多邻域移动和更好的初始种子
- 在共享算法枚举之后加入更多构造型和精确算法
- 为未来的元启发式加入共享求解器抽象
- 与原生发布 bundle 一起发布 Python 和 Java 绑定产物
