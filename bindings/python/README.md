# tsp_solver Python 绑定

平台 wheel 会打包原生 `tsp_solver` 共享库，因此安装后的 wheel 可以直接导入并求解，
无需额外环境配置。

如果你还会使用原生安装归档或 Java 绑定，请使用与其他产物相同发布版本的 wheel。

Linux wheel 当前在匹配的 GitHub 托管 runner 镜像上构建并验证，尚未修复到 manylinux 基线。

本地开发时，wheel 构建会从 `TSP_SOLVER_PYTHON_NATIVE_LIBRARY` 获取原生库路径。
