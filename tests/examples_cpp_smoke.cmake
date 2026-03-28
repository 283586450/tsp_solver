if(NOT DEFINED examples_cpp_binary)
  message(FATAL_ERROR "examples_cpp_binary is required")
endif()

execute_process(
  COMMAND "${examples_cpp_binary}" "${CMAKE_CURRENT_LIST_DIR}/../examples/data/20_city_distance_matrix.txt"
  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/.."
  OUTPUT_VARIABLE output
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "C++ example failed")
endif()

string(FIND "${output}" "TSP_SOLVER_ALGORITHM_DEFAULT" default_pos)
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR" greedy_pos)
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT" two_opt_pos)

if(default_pos EQUAL -1 OR greedy_pos EQUAL -1 OR two_opt_pos EQUAL -1)
  message(FATAL_ERROR "C++ example output missing expected algorithm labels")
endif()
