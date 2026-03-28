find_package(Java REQUIRED)
find_package(JNI REQUIRED)

set(TSP_SOLVER_JAVA_MAIN_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Algorithm.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Model.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/NativeLibrary.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/NativeResource.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Options.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Result.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Solver.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/SolverNativeException.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Status.java)

set(TSP_SOLVER_JAVA_TEST_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test/java/tsp/solver/BindingTestMain.java)

set(TSP_SOLVER_JAVA_CLASSES_DIR ${CMAKE_CURRENT_BINARY_DIR}/classes)
set(TSP_SOLVER_JAVA_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(TSP_SOLVER_JAVA_COMPILE_STAMP ${TSP_SOLVER_JAVA_CLASSES_DIR}/.compiled)

add_custom_command(
  OUTPUT ${TSP_SOLVER_JAVA_COMPILE_STAMP}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TSP_SOLVER_JAVA_CLASSES_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TSP_SOLVER_JAVA_HEADERS_DIR}
  COMMAND ${Java_JAVAC_EXECUTABLE} -h ${TSP_SOLVER_JAVA_HEADERS_DIR} -d ${TSP_SOLVER_JAVA_CLASSES_DIR}
          ${TSP_SOLVER_JAVA_MAIN_SOURCES} ${TSP_SOLVER_JAVA_TEST_SOURCES}
  COMMAND ${CMAKE_COMMAND} -E touch ${TSP_SOLVER_JAVA_COMPILE_STAMP}
  DEPENDS ${TSP_SOLVER_JAVA_MAIN_SOURCES} ${TSP_SOLVER_JAVA_TEST_SOURCES}
  VERBATIM)

add_custom_target(tsp_solver_java_classes ALL DEPENDS ${TSP_SOLVER_JAVA_COMPILE_STAMP})

add_library(tsp_solver_java_jni SHARED src/main/cpp/tsp_solver_java_jni.cpp)
add_dependencies(tsp_solver_java_jni tsp_solver_java_classes)
target_include_directories(tsp_solver_java_jni
  PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${TSP_SOLVER_JAVA_HEADERS_DIR})
target_link_libraries(tsp_solver_java_jni PRIVATE tsp_solver)

if(WIN32)
  set(_tsp_solver_java_test_path
      "PATH=$<TARGET_FILE_DIR:tsp_solver_java_jni>\;$<TARGET_FILE_DIR:tsp_solver>\;$ENV{PATH}")
  set(_tsp_solver_java_env_test_path "PATH=$<TARGET_FILE_DIR:tsp_solver>\;$ENV{PATH}")
  add_test(
    NAME tsp_solver_java_tests
    COMMAND ${CMAKE_COMMAND} -E env
            "${_tsp_solver_java_test_path}"
            ${Java_JAVA_EXECUTABLE}
            -Dtsp.solver.library.path=$<TARGET_FILE:tsp_solver_java_jni>
            -cp ${TSP_SOLVER_JAVA_CLASSES_DIR}
            tsp.solver.BindingTestMain)
  add_test(
    NAME tsp_solver_java_tests_env
    COMMAND ${CMAKE_COMMAND} -E env
            "${_tsp_solver_java_env_test_path}"
            "TSP_SOLVER_LIBRARY_PATH=$<TARGET_FILE:tsp_solver>"
            "TSP_SOLVER_JAVA_LIBRARY_PATH=$<TARGET_FILE:tsp_solver_java_jni>"
            ${Java_JAVA_EXECUTABLE}
            -cp ${TSP_SOLVER_JAVA_CLASSES_DIR}
            tsp.solver.BindingTestMain)
else()
  add_test(
    NAME tsp_solver_java_tests
    COMMAND ${Java_JAVA_EXECUTABLE}
            -Dtsp.solver.library.path=$<TARGET_FILE:tsp_solver_java_jni>
            -cp ${TSP_SOLVER_JAVA_CLASSES_DIR}
            tsp.solver.BindingTestMain)
  add_test(
    NAME tsp_solver_java_tests_env
    COMMAND ${CMAKE_COMMAND} -E env
            TSP_SOLVER_LIBRARY_PATH=$<TARGET_FILE:tsp_solver>
            TSP_SOLVER_JAVA_LIBRARY_PATH=$<TARGET_FILE:tsp_solver_java_jni>
            ${Java_JAVA_EXECUTABLE}
            -cp ${TSP_SOLVER_JAVA_CLASSES_DIR}
            tsp.solver.BindingTestMain)
endif()
