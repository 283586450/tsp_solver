add_executable(tsp_solver_tests local_search_test.cpp)
target_link_libraries(tsp_solver_tests PRIVATE tsp_solver)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/ProjectWarnings.cmake)
tsp_solver_enable_warnings(tsp_solver_tests)

add_test(NAME tsp_solver_tests COMMAND tsp_solver_tests)
