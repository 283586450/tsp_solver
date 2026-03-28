if(NOT DEFINED binary_dir)
  message(FATAL_ERROR "binary_dir is required")
endif()

if(NOT DEFINED make_program)
  message(FATAL_ERROR "make_program is required")
endif()

if(NOT DEFINED c_compiler OR NOT DEFINED cxx_compiler OR NOT DEFINED rc_compiler OR NOT DEFINED mt_tool)
  message(FATAL_ERROR "c_compiler, cxx_compiler, rc_compiler, and mt_tool are required")
endif()

get_filename_component(rc_dir "${rc_compiler}" DIRECTORY)
get_filename_component(mt_dir "${mt_tool}" DIRECTORY)

set(install_dir "${binary_dir}/package_smoke_install")
set(consumer_build_dir "${binary_dir}/package_smoke_consumer")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${binary_dir}" --prefix "${install_dir}"
  RESULT_VARIABLE install_result)
if(NOT install_result EQUAL 0)
  message(FATAL_ERROR "install step failed")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "CC=${c_compiler}"
          "CXX=${cxx_compiler}"
          "RC=${rc_compiler}"
          "MT=${mt_tool}"
          "PATH=${rc_dir};${mt_dir};$ENV{PATH}"
          "${CMAKE_COMMAND}" -G Ninja
          -DCMAKE_MAKE_PROGRAM=${make_program}
          -DCMAKE_C_COMPILER=${c_compiler}
          -DCMAKE_CXX_COMPILER=${cxx_compiler}
          -DCMAKE_RC_COMPILER=${rc_compiler}
          -DCMAKE_MT=${mt_tool}
          -S "${CMAKE_CURRENT_LIST_DIR}/package_smoke"
          -B "${consumer_build_dir}"
          -Dtsp_solver_DIR=${install_dir}/lib/cmake/tsp_solver
  RESULT_VARIABLE configure_result)
if(NOT configure_result EQUAL 0)
  message(FATAL_ERROR "consumer configure failed")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "CC=${c_compiler}"
          "CXX=${cxx_compiler}"
          "RC=${rc_compiler}"
          "MT=${mt_tool}"
          "PATH=${rc_dir};${mt_dir};$ENV{PATH}"
          "${CMAKE_COMMAND}" --build "${consumer_build_dir}"
  RESULT_VARIABLE build_result)
if(NOT build_result EQUAL 0)
  message(FATAL_ERROR "consumer build failed")
endif()
