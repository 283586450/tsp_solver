# 示例

## 这组示例展示什么

这组示例展示如何用同一份 20 城市 TSP 数据，从 C++、Python 和 Java 调用求解库，并对比三种算法的输出：

- `TSP_SOLVER_ALGORITHM_DEFAULT`
- `TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR`
- `TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT`

## 共享数据集

三种语言都读取 `examples/data/20_city_distance_matrix.txt`。

该文件格式如下：

- 第一行是城市数量 `20`
- 后面 20 行，每行 20 个以空格分隔的整数
- 对角线为 `0`

## C++ 示例

```bash
cmake --build --preset <your-host-appropriate-preset> --target tsp_solver_examples_cpp
./build/<your-host-appropriate-preset>/examples/cpp/tsp_solver_examples_cpp examples/data/20_city_distance_matrix.txt
```

## Python 示例

运行前请把 `PYTHONPATH` 指向 `bindings/python`，并让 `TSP_SOLVER_LIBRARY_PATH` 指向已构建的原生库。

```bash
python examples/python/compare_algorithms.py examples/data/20_city_distance_matrix.txt
```

## Java 示例

运行前请把 classpath 指向 `build/<your-host-appropriate-preset>/bindings/java/classes/main` 和
`build/<your-host-appropriate-preset>/bindings/java/classes/examples`，并通过
`-Dtsp.solver.library.path=<jni bridge path>` 或 `TSP_SOLVER_JAVA_LIBRARY_PATH` 指向 JNI 桥接库。

```bash
cmake --build --preset <your-host-appropriate-preset> --target tsp_solver_java_example_classes
java -cp <java classpath> tsp.solver.examples.CompareAlgorithmsMain examples/data/20_city_distance_matrix.txt
```

## 期望输出

每个示例都会输出一张小表，至少包含：

- 算法名
- 目标值
- tour

## 运行前提

- C++：使用当前仓库的 CMake 构建结果
- Python：需要能导入 `bindings/python`，并能加载原生库
- Java：需要能加载 Java jar 和 native bundle
