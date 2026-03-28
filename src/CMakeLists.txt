target_sources(tsp_solver
  PRIVATE
    algorithms/held_karp.cpp
    algorithms/local_search.cpp
    c_api.cpp)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/ProjectWarnings.cmake)
tsp_solver_enable_warnings(tsp_solver)
