find_package(Python3 COMPONENTS Interpreter REQUIRED)

file(GLOB TSP_SOLVER_PYTHON_PACKAGE_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/tsp_solver/*.py
     ${CMAKE_CURRENT_SOURCE_DIR}/*.md
     ${CMAKE_CURRENT_SOURCE_DIR}/*.py
     ${CMAKE_CURRENT_SOURCE_DIR}/*.toml)

set(TSP_SOLVER_PYTHON_WHEEL_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist)
set(TSP_SOLVER_PYTHON_WHEEL_STAMP ${TSP_SOLVER_PYTHON_WHEEL_DIR}/wheel.stamp)

add_custom_command(
  OUTPUT ${TSP_SOLVER_PYTHON_WHEEL_STAMP}
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${TSP_SOLVER_PYTHON_WHEEL_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TSP_SOLVER_PYTHON_WHEEL_DIR}
  COMMAND ${CMAKE_COMMAND} -E env
          TSP_SOLVER_PYTHON_NATIVE_LIBRARY=$<TARGET_FILE:tsp_solver>
          ${Python3_EXECUTABLE}
          -m pip wheel ${CMAKE_CURRENT_SOURCE_DIR} --no-deps --wheel-dir ${TSP_SOLVER_PYTHON_WHEEL_DIR}
  COMMAND ${CMAKE_COMMAND} -E touch ${TSP_SOLVER_PYTHON_WHEEL_STAMP}
  DEPENDS tsp_solver ${TSP_SOLVER_PYTHON_PACKAGE_SOURCES}
  VERBATIM)

add_custom_target(tsp_solver_python_wheel DEPENDS ${TSP_SOLVER_PYTHON_WHEEL_STAMP})

add_test(
  NAME tsp_solver_python_tests
  COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_SOURCE_DIR}/tests/python -p
          test_*.py -v)

add_test(
  NAME tsp_solver_python_package_smoke
  COMMAND ${CMAKE_COMMAND}
          -Dpython_executable=${Python3_EXECUTABLE}
          -Dwheel_dir=${TSP_SOLVER_PYTHON_WHEEL_DIR}
          -Dsource_dir=${CMAKE_CURRENT_SOURCE_DIR}
          -Dnative_library=$<TARGET_FILE:tsp_solver>
          -Dsmoke_script=${CMAKE_SOURCE_DIR}/tests/python/package_smoke.py
          -Dvenv_dir=${CMAKE_CURRENT_BINARY_DIR}/package-smoke-venv
          -P ${CMAKE_CURRENT_SOURCE_DIR}/package_smoke.cmake)

if(WIN32)
  set(_tsp_solver_python_test_path "PATH=$<TARGET_FILE_DIR:tsp_solver>;$ENV{PATH}")
  set(_tsp_solver_python_env
      "${_tsp_solver_python_test_path};PYTHONPATH=${CMAKE_SOURCE_DIR}/bindings/python;TSP_SOLVER_LIBRARY_PATH=$<TARGET_FILE:tsp_solver>")
else()
  set(_tsp_solver_python_env
      "PYTHONPATH=${CMAKE_SOURCE_DIR}/bindings/python;TSP_SOLVER_LIBRARY_PATH=$<TARGET_FILE:tsp_solver>")
endif()

set_tests_properties(tsp_solver_python_tests PROPERTIES ENVIRONMENT "${_tsp_solver_python_env}")
