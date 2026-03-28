add_executable(tsp_solver_examples_cpp compare_algorithms.cpp)
target_link_libraries(tsp_solver_examples_cpp PRIVATE tsp_solver)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/ProjectWarnings.cmake)
tsp_solver_enable_warnings(tsp_solver_examples_cpp)
