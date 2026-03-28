if(NOT DEFINED cmake_command)
  message(FATAL_ERROR "cmake_command is required")
endif()

if(NOT DEFINED build_dir)
  message(FATAL_ERROR "build_dir is required")
endif()

if(NOT DEFINED java_executable)
  message(FATAL_ERROR "java_executable is required")
endif()

if(NOT DEFINED javac_executable)
  message(FATAL_ERROR "javac_executable is required")
endif()

if(NOT DEFINED jar_file)
  message(FATAL_ERROR "jar_file is required")
endif()

if(NOT DEFINED bundle_dir)
  message(FATAL_ERROR "bundle_dir is required")
endif()

if(NOT DEFINED smoke_source)
  message(FATAL_ERROR "smoke_source is required")
endif()

if(NOT DEFINED smoke_build_dir)
  message(FATAL_ERROR "smoke_build_dir is required")
endif()

if(NOT DEFINED jni_library_name)
  message(FATAL_ERROR "jni_library_name is required")
endif()

execute_process(
  COMMAND "${cmake_command}" --build "${build_dir}" --target tsp_solver_java_jar tsp_solver_java_native_bundle
  RESULT_VARIABLE build_result)
if(NOT build_result EQUAL 0)
  message(FATAL_ERROR "java package build targets failed")
endif()

file(REMOVE_RECURSE "${smoke_build_dir}")
file(MAKE_DIRECTORY "${smoke_build_dir}")

execute_process(
  COMMAND "${javac_executable}" -cp "${jar_file}" -d "${smoke_build_dir}" "${smoke_source}"
  RESULT_VARIABLE compile_result)
if(NOT compile_result EQUAL 0)
  message(FATAL_ERROR "java package smoke compile failed")
endif()

set(jni_library_path "${bundle_dir}/${jni_library_name}")

if(CMAKE_HOST_WIN32)
  set(classpath_separator ";")
  set(loader_env "PATH=${bundle_dir};$ENV{PATH}")
elseif(APPLE)
  set(classpath_separator ":")
  set(loader_env "DYLD_LIBRARY_PATH=${bundle_dir}:$ENV{DYLD_LIBRARY_PATH}")
else()
  set(classpath_separator ":")
  set(loader_env "LD_LIBRARY_PATH=${bundle_dir}:$ENV{LD_LIBRARY_PATH}")
endif()

execute_process(
  COMMAND "${cmake_command}" -E env
          "${loader_env}"
          "${java_executable}"
          -Dtsp.solver.library.path=${jni_library_path}
          -cp "${smoke_build_dir}${classpath_separator}${jar_file}"
          PackageSmokeMain
  RESULT_VARIABLE smoke_result)
if(NOT smoke_result EQUAL 0)
  message(FATAL_ERROR "java package smoke run failed")
endif()
