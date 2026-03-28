cmake_minimum_required(VERSION 3.24)

project(package_smoke C)

find_package(tsp_solver CONFIG REQUIRED NO_DEFAULT_PATH PATHS "${tsp_solver_DIR}")

add_executable(package_smoke main.c)
target_link_libraries(package_smoke PRIVATE tsp_solver::tsp_solver)
