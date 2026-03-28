if(NOT DEFINED binary_dir)
  message(FATAL_ERROR "binary_dir is required")
endif()

if(NOT DEFINED make_program)
  message(FATAL_ERROR "make_program is required")
endif()

if(NOT DEFINED project_version)
  message(FATAL_ERROR "project_version is required")
endif()

if(NOT DEFINED c_compiler OR NOT DEFINED cxx_compiler)
  message(FATAL_ERROR "c_compiler and cxx_compiler are required")
endif()

set(install_dir "${binary_dir}/package_smoke_install")
set(consumer_build_dir "${binary_dir}/package_smoke_consumer")
set(mismatch_build_dir "${binary_dir}/package_smoke_version_mismatch")
set(configure_env "CC=${c_compiler}" "CXX=${cxx_compiler}")
set(configure_args
    -G Ninja
    -DCMAKE_MAKE_PROGRAM=${make_program}
    -DCMAKE_C_COMPILER=${c_compiler}
    -DCMAKE_CXX_COMPILER=${cxx_compiler}
    -S "${CMAKE_CURRENT_LIST_DIR}/package_smoke"
    -B "${consumer_build_dir}"
    -Dtsp_solver_VERSION_REQUIRED=${project_version}
    -Dtsp_solver_DIR=${install_dir}/lib/cmake/tsp_solver)

if(CMAKE_HOST_WIN32)
  if(NOT DEFINED rc_compiler OR NOT DEFINED mt_tool)
    message(FATAL_ERROR "rc_compiler and mt_tool are required on Windows")
  endif()

  get_filename_component(rc_dir "${rc_compiler}" DIRECTORY)
  get_filename_component(mt_dir "${mt_tool}" DIRECTORY)
  set(include_env "$ENV{INCLUDE}")
  set(lib_env "$ENV{LIB}")
  set(libpath_env "$ENV{LIBPATH}")
  string(REPLACE ";" "\\;" include_env "${include_env}")
  string(REPLACE ";" "\\;" lib_env "${lib_env}")
  string(REPLACE ";" "\\;" libpath_env "${libpath_env}")
  set(path_env "${rc_dir}\;${mt_dir}\;$ENV{PATH}")
  list(APPEND configure_env
       "RC=${rc_compiler}"
       "MT=${mt_tool}"
       "INCLUDE=${include_env}"
       "LIB=${lib_env}"
       "LIBPATH=${libpath_env}"
       "PATH=${path_env}")
  list(APPEND configure_args
       -DCMAKE_RC_COMPILER=${rc_compiler}
       -DCMAKE_MT=${mt_tool})
endif()

file(REMOVE_RECURSE "${consumer_build_dir}" "${mismatch_build_dir}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${binary_dir}" --prefix "${install_dir}"
  RESULT_VARIABLE install_result)
if(NOT install_result EQUAL 0)
  message(FATAL_ERROR "install step failed")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env ${configure_env} "${CMAKE_COMMAND}" ${configure_args}
  RESULT_VARIABLE configure_result)
if(NOT configure_result EQUAL 0)
  message(FATAL_ERROR "consumer configure failed")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env ${configure_env} "${CMAKE_COMMAND}" --build "${consumer_build_dir}"
  RESULT_VARIABLE build_result)
if(NOT build_result EQUAL 0)
  message(FATAL_ERROR "consumer build failed")
endif()

set(mismatch_configure_args ${configure_args})
list(REMOVE_ITEM mismatch_configure_args -B "${consumer_build_dir}")
list(APPEND mismatch_configure_args
     -B "${mismatch_build_dir}"
     -Dtsp_solver_VERSION_REQUIRED=9.9.9)

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env ${configure_env} "${CMAKE_COMMAND}" ${mismatch_configure_args}
  RESULT_VARIABLE mismatch_result)
if(mismatch_result EQUAL 0)
  message(FATAL_ERROR "expected exact-version consumer configure to fail")
endif()
