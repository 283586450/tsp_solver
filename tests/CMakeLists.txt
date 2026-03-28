add_executable(tsp_solver_tests local_search_test.cpp)
target_link_libraries(tsp_solver_tests PRIVATE tsp_solver)

add_executable(tsp_solver_c_api_header_test c_api_header_test.c)
target_link_libraries(tsp_solver_c_api_header_test PRIVATE tsp_solver)

add_executable(tsp_solver_c_api_header_test_cpp c_api_header_test.cpp)
target_link_libraries(tsp_solver_c_api_header_test_cpp PRIVATE tsp_solver)

add_executable(tsp_solver_c_api_runtime_test c_api_runtime_test.cpp)
target_link_libraries(tsp_solver_c_api_runtime_test PRIVATE tsp_solver)

add_executable(tsp_solver_c_api_error_test c_api_error_test.cpp)
target_link_libraries(tsp_solver_c_api_error_test PRIVATE tsp_solver)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/ProjectWarnings.cmake)
tsp_solver_enable_warnings(tsp_solver_tests)
tsp_solver_enable_warnings(tsp_solver_c_api_header_test)
tsp_solver_enable_warnings(tsp_solver_c_api_header_test_cpp)
tsp_solver_enable_warnings(tsp_solver_c_api_runtime_test)
tsp_solver_enable_warnings(tsp_solver_c_api_error_test)

add_test(NAME tsp_solver_tests COMMAND tsp_solver_tests)
add_test(NAME tsp_solver_c_api_header_test COMMAND tsp_solver_c_api_header_test)
add_test(NAME tsp_solver_c_api_header_test_cpp COMMAND tsp_solver_c_api_header_test_cpp)
add_test(NAME tsp_solver_c_api_runtime_test COMMAND tsp_solver_c_api_runtime_test)
add_test(NAME tsp_solver_c_api_error_test COMMAND tsp_solver_c_api_error_test)

set(_tsp_solver_package_smoke_args
    -Dbinary_dir=${CMAKE_BINARY_DIR}
    -Dmake_program=${CMAKE_MAKE_PROGRAM}
    -Dc_compiler=${CMAKE_C_COMPILER}
    -Dcxx_compiler=${CMAKE_CXX_COMPILER})

if(WIN32)
  list(APPEND _tsp_solver_package_smoke_args
       -Drc_compiler=${CMAKE_RC_COMPILER}
       -Dmt_tool=${CMAKE_MT})
endif()

add_test(
  NAME tsp_solver_package_smoke
  COMMAND ${CMAKE_COMMAND} ${_tsp_solver_package_smoke_args} -P ${CMAKE_CURRENT_LIST_DIR}/package_smoke.cmake)

if(WIN32)
  set(_tsp_solver_test_path "PATH=$<TARGET_FILE_DIR:tsp_solver>;$ENV{PATH}")
  foreach(test_name
      tsp_solver_tests
      tsp_solver_c_api_header_test
      tsp_solver_c_api_header_test_cpp
      tsp_solver_c_api_runtime_test
      tsp_solver_c_api_error_test
      tsp_solver_package_smoke)
    set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "${_tsp_solver_test_path}")
  endforeach()
endif()
