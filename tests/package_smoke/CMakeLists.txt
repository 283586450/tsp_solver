cmake_minimum_required(VERSION 3.24)

project(package_smoke C)

if(DEFINED tsp_solver_VERSION_REQUIRED)
  find_package(tsp_solver ${tsp_solver_VERSION_REQUIRED} EXACT CONFIG REQUIRED NO_DEFAULT_PATH
               PATHS "${tsp_solver_DIR}")
else()
  find_package(tsp_solver CONFIG REQUIRED NO_DEFAULT_PATH PATHS "${tsp_solver_DIR}")
endif()

add_executable(package_smoke main.c)
target_link_libraries(package_smoke PRIVATE tsp_solver::tsp_solver)
