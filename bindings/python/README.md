# tsp_solver Python bindings

Platform wheels bundle the native `tsp_solver` shared library so installed wheels
can import and solve directly without extra environment setup.

Linux wheels are currently built and verified on the matching GitHub-hosted
runner image; they are not yet repaired to a manylinux baseline.

For local development, the wheel build consumes the native library path from
`TSP_SOLVER_PYTHON_NATIVE_LIBRARY`.
