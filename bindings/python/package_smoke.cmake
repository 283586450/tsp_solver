if(NOT DEFINED python_executable)
  message(FATAL_ERROR "python_executable is required")
endif()

if(NOT DEFINED wheel_dir)
  message(FATAL_ERROR "wheel_dir is required")
endif()

if(NOT DEFINED source_dir)
  message(FATAL_ERROR "source_dir is required")
endif()

if(NOT DEFINED native_library)
  message(FATAL_ERROR "native_library is required")
endif()

if(NOT DEFINED smoke_script)
  message(FATAL_ERROR "smoke_script is required")
endif()

if(NOT DEFINED venv_dir)
  message(FATAL_ERROR "venv_dir is required")
endif()

file(REMOVE_RECURSE "${wheel_dir}" "${venv_dir}")
file(MAKE_DIRECTORY "${wheel_dir}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
          "TSP_SOLVER_PYTHON_NATIVE_LIBRARY=${native_library}"
          "${python_executable}" -m pip wheel "${source_dir}" --no-deps --wheel-dir "${wheel_dir}"
  RESULT_VARIABLE wheel_result)
if(NOT wheel_result EQUAL 0)
  message(FATAL_ERROR "python wheel build failed")
endif()

file(GLOB wheel_files "${wheel_dir}/*.whl")
list(LENGTH wheel_files wheel_count)
if(NOT wheel_count EQUAL 1)
  message(FATAL_ERROR "expected one wheel in ${wheel_dir}, found ${wheel_count}")
endif()
list(GET wheel_files 0 wheel_file)

execute_process(
  COMMAND "${python_executable}" -m venv "${venv_dir}"
  RESULT_VARIABLE venv_result)
if(NOT venv_result EQUAL 0)
  message(FATAL_ERROR "python venv creation failed")
endif()

if(CMAKE_HOST_WIN32)
  set(venv_python "${venv_dir}/Scripts/python.exe")
else()
  set(venv_python "${venv_dir}/bin/python")
endif()

execute_process(
  COMMAND "${venv_python}" -m pip install --force-reinstall "${wheel_file}"
  RESULT_VARIABLE install_result)
if(NOT install_result EQUAL 0)
  message(FATAL_ERROR "python wheel install failed")
endif()

execute_process(
  COMMAND "${venv_python}" "${smoke_script}"
  RESULT_VARIABLE smoke_result)
if(NOT smoke_result EQUAL 0)
  message(FATAL_ERROR "python package smoke test failed")
endif()
