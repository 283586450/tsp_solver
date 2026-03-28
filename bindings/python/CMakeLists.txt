find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(
  NAME tsp_solver_python_tests
  COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_SOURCE_DIR}/tests/python -p
          test_*.py -v)

if(WIN32)
  set(_tsp_solver_python_test_path "PATH=$<TARGET_FILE_DIR:tsp_solver>;$ENV{PATH}")
  set(_tsp_solver_python_env
      "${_tsp_solver_python_test_path};PYTHONPATH=${CMAKE_SOURCE_DIR}/bindings/python;TSP_SOLVER_LIBRARY_PATH=$<TARGET_FILE:tsp_solver>")
else()
  set(_tsp_solver_python_env
      "PYTHONPATH=${CMAKE_SOURCE_DIR}/bindings/python;TSP_SOLVER_LIBRARY_PATH=$<TARGET_FILE:tsp_solver>")
endif()

set_tests_properties(tsp_solver_python_tests PROPERTIES ENVIRONMENT "${_tsp_solver_python_env}")
