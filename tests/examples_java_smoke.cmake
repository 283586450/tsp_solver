if(NOT DEFINED java_executable)
  message(FATAL_ERROR "java_executable is required")
endif()

if(NOT DEFINED java_main_classes_dir OR NOT DEFINED java_example_classes_dir)
  message(FATAL_ERROR "java_main_classes_dir and java_example_classes_dir are required")
endif()

if(NOT DEFINED jni_library_path)
  message(FATAL_ERROR "jni_library_path is required")
endif()

if(CMAKE_HOST_WIN32)
  set(classpath_separator ";")
else()
  set(classpath_separator ":")
endif()

set(java_example_classpath "${java_main_classes_dir}${classpath_separator}${java_example_classes_dir}")

execute_process(
  COMMAND "${java_executable}" -Dtsp.solver.library.path=${jni_library_path} -cp "${java_example_classpath}"
          tsp.solver.examples.CompareAlgorithmsMain
          "${CMAKE_CURRENT_LIST_DIR}/../examples/data/20_city_distance_matrix.txt"
  OUTPUT_VARIABLE output
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Java example failed")
endif()

string(FIND "${output}" "TSP_SOLVER_ALGORITHM_DEFAULT" default_pos)
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR" greedy_pos)
string(FIND "${output}" "TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT" two_opt_pos)

if(default_pos EQUAL -1 OR greedy_pos EQUAL -1 OR two_opt_pos EQUAL -1)
  message(FATAL_ERROR "Java example output missing expected algorithm labels")
endif()
