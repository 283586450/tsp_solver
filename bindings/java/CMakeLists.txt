find_package(Java REQUIRED)
find_package(JNI REQUIRED)

set(TSP_SOLVER_JAVA_GENERATED_SOURCES_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated-java)
set(TSP_SOLVER_JAVA_VERSION_SOURCE
    ${TSP_SOLVER_JAVA_GENERATED_SOURCES_DIR}/tsp/solver/Version.java)
file(MAKE_DIRECTORY ${TSP_SOLVER_JAVA_GENERATED_SOURCES_DIR}/tsp/solver)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Version.java.in
               ${TSP_SOLVER_JAVA_VERSION_SOURCE} @ONLY)

set(TSP_SOLVER_JAVA_MAIN_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Algorithm.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Model.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/NativeLibrary.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/NativeResource.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Options.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Result.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Solver.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/SolverNativeException.java
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main/java/tsp/solver/Status.java
  ${TSP_SOLVER_JAVA_VERSION_SOURCE})

set(TSP_SOLVER_JAVA_TEST_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/test/java/tsp/solver/BindingTestMain.java)

set(TSP_SOLVER_JAVA_EXAMPLE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/../../examples/java/src/main/java/tsp/solver/examples/CompareAlgorithmsMain.java)

set(TSP_SOLVER_JAVA_MAIN_CLASSES_DIR ${CMAKE_CURRENT_BINARY_DIR}/classes/main)
set(TSP_SOLVER_JAVA_TEST_CLASSES_DIR ${CMAKE_CURRENT_BINARY_DIR}/classes/test)
set(TSP_SOLVER_JAVA_EXAMPLE_CLASSES_DIR ${CMAKE_CURRENT_BINARY_DIR}/classes/examples)
set(TSP_SOLVER_JAVA_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(TSP_SOLVER_JAVA_MAIN_COMPILE_STAMP ${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR}/.compiled)
set(TSP_SOLVER_JAVA_TEST_COMPILE_STAMP ${TSP_SOLVER_JAVA_TEST_CLASSES_DIR}/.compiled)
set(TSP_SOLVER_JAVA_EXAMPLE_COMPILE_STAMP ${TSP_SOLVER_JAVA_EXAMPLE_CLASSES_DIR}/.compiled)
set(TSP_SOLVER_JAVA_DIST_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" TSP_SOLVER_JAVA_SYSTEM_PROCESSOR)
if(TSP_SOLVER_JAVA_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$")
  set(TSP_SOLVER_JAVA_ARCH x64)
elseif(TSP_SOLVER_JAVA_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
  set(TSP_SOLVER_JAVA_ARCH arm64)
else()
  set(TSP_SOLVER_JAVA_ARCH ${TSP_SOLVER_JAVA_SYSTEM_PROCESSOR})
endif()

if(WIN32)
  set(TSP_SOLVER_JAVA_PLATFORM windows-${TSP_SOLVER_JAVA_ARCH})
  set(TSP_SOLVER_JAVA_TEST_CLASSPATH
      "${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR}\;${TSP_SOLVER_JAVA_TEST_CLASSES_DIR}")
elseif(APPLE)
  set(TSP_SOLVER_JAVA_PLATFORM macos-${TSP_SOLVER_JAVA_ARCH})
  set(TSP_SOLVER_JAVA_TEST_CLASSPATH
      "${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR}:${TSP_SOLVER_JAVA_TEST_CLASSES_DIR}")
else()
  set(TSP_SOLVER_JAVA_PLATFORM linux-${TSP_SOLVER_JAVA_ARCH})
  set(TSP_SOLVER_JAVA_TEST_CLASSPATH
      "${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR}:${TSP_SOLVER_JAVA_TEST_CLASSES_DIR}")
endif()

set(TSP_SOLVER_JAVA_JAR ${TSP_SOLVER_JAVA_DIST_DIR}/tsp-solver-java-${PROJECT_VERSION}.jar)
set(TSP_SOLVER_JAVA_NATIVE_BUNDLE_DIR
    ${TSP_SOLVER_JAVA_DIST_DIR}/tsp-solver-java-native-${TSP_SOLVER_JAVA_PLATFORM}-${PROJECT_VERSION})
set(TSP_SOLVER_JAVA_NATIVE_BUNDLE_STAMP ${TSP_SOLVER_JAVA_NATIVE_BUNDLE_DIR}/.bundle-ready)

add_custom_command(
  OUTPUT ${TSP_SOLVER_JAVA_MAIN_COMPILE_STAMP}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TSP_SOLVER_JAVA_HEADERS_DIR}
  COMMAND ${Java_JAVAC_EXECUTABLE} -h ${TSP_SOLVER_JAVA_HEADERS_DIR} -d ${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR}
          ${TSP_SOLVER_JAVA_MAIN_SOURCES}
  COMMAND ${CMAKE_COMMAND} -E touch ${TSP_SOLVER_JAVA_MAIN_COMPILE_STAMP}
  DEPENDS ${TSP_SOLVER_JAVA_MAIN_SOURCES}
  VERBATIM)

add_custom_target(tsp_solver_java_main_classes ALL DEPENDS ${TSP_SOLVER_JAVA_MAIN_COMPILE_STAMP})

add_custom_command(
  OUTPUT ${TSP_SOLVER_JAVA_TEST_COMPILE_STAMP}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TSP_SOLVER_JAVA_TEST_CLASSES_DIR}
  COMMAND ${Java_JAVAC_EXECUTABLE} -cp ${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR} -d ${TSP_SOLVER_JAVA_TEST_CLASSES_DIR}
          ${TSP_SOLVER_JAVA_TEST_SOURCES}
  COMMAND ${CMAKE_COMMAND} -E touch ${TSP_SOLVER_JAVA_TEST_COMPILE_STAMP}
  DEPENDS tsp_solver_java_main_classes ${TSP_SOLVER_JAVA_TEST_SOURCES}
  VERBATIM)

add_custom_target(tsp_solver_java_test_classes ALL DEPENDS ${TSP_SOLVER_JAVA_TEST_COMPILE_STAMP})

add_custom_command(
  OUTPUT ${TSP_SOLVER_JAVA_EXAMPLE_COMPILE_STAMP}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TSP_SOLVER_JAVA_EXAMPLE_CLASSES_DIR}
  COMMAND ${Java_JAVAC_EXECUTABLE} -cp ${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR} -d ${TSP_SOLVER_JAVA_EXAMPLE_CLASSES_DIR}
          ${TSP_SOLVER_JAVA_EXAMPLE_SOURCES}
  COMMAND ${CMAKE_COMMAND} -E touch ${TSP_SOLVER_JAVA_EXAMPLE_COMPILE_STAMP}
  DEPENDS tsp_solver_java_main_classes ${TSP_SOLVER_JAVA_EXAMPLE_SOURCES}
  VERBATIM)

add_custom_target(tsp_solver_java_example_classes ALL DEPENDS ${TSP_SOLVER_JAVA_EXAMPLE_COMPILE_STAMP})

add_custom_command(
  OUTPUT ${TSP_SOLVER_JAVA_JAR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TSP_SOLVER_JAVA_DIST_DIR}
  COMMAND ${CMAKE_COMMAND} -E rm -f ${TSP_SOLVER_JAVA_JAR}
  COMMAND ${Java_JAR_EXECUTABLE} cf ${TSP_SOLVER_JAVA_JAR} -C ${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR} .
  DEPENDS tsp_solver_java_main_classes ${TSP_SOLVER_JAVA_MAIN_SOURCES}
  VERBATIM)

add_custom_target(tsp_solver_java_jar DEPENDS ${TSP_SOLVER_JAVA_JAR})

add_library(tsp_solver_java_jni SHARED src/main/cpp/tsp_solver_java_jni.cpp)
add_dependencies(tsp_solver_java_jni tsp_solver_java_main_classes)
target_include_directories(tsp_solver_java_jni
  PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${TSP_SOLVER_JAVA_HEADERS_DIR})
target_link_libraries(tsp_solver_java_jni PRIVATE tsp_solver)

if(APPLE)
  set_target_properties(tsp_solver_java_jni PROPERTIES BUILD_RPATH "@loader_path" INSTALL_RPATH "@loader_path")
elseif(UNIX)
  set_target_properties(tsp_solver_java_jni PROPERTIES BUILD_RPATH "$ORIGIN" INSTALL_RPATH "$ORIGIN")
endif()

add_custom_command(
  OUTPUT ${TSP_SOLVER_JAVA_NATIVE_BUNDLE_STAMP}
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${TSP_SOLVER_JAVA_NATIVE_BUNDLE_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${TSP_SOLVER_JAVA_NATIVE_BUNDLE_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${TSP_SOLVER_JAVA_NATIVE_BUNDLE_DIR}/README.md
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tsp_solver_java_jni> ${TSP_SOLVER_JAVA_NATIVE_BUNDLE_DIR}/$<TARGET_FILE_NAME:tsp_solver_java_jni>
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tsp_solver> ${TSP_SOLVER_JAVA_NATIVE_BUNDLE_DIR}/$<TARGET_FILE_NAME:tsp_solver>
  COMMAND ${CMAKE_COMMAND} -E touch ${TSP_SOLVER_JAVA_NATIVE_BUNDLE_STAMP}
  DEPENDS tsp_solver_java_jni tsp_solver ${CMAKE_CURRENT_SOURCE_DIR}/README.md
  VERBATIM)

add_custom_target(tsp_solver_java_native_bundle DEPENDS ${TSP_SOLVER_JAVA_NATIVE_BUNDLE_STAMP})

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
            -cp ${TSP_SOLVER_JAVA_TEST_CLASSPATH}
            tsp.solver.BindingTestMain)
  add_test(
    NAME tsp_solver_java_tests_env
    COMMAND ${CMAKE_COMMAND} -E env
            "${_tsp_solver_java_env_test_path}"
            "TSP_SOLVER_LIBRARY_PATH=$<TARGET_FILE:tsp_solver>"
            "TSP_SOLVER_JAVA_LIBRARY_PATH=$<TARGET_FILE:tsp_solver_java_jni>"
            ${Java_JAVA_EXECUTABLE}
            -cp ${TSP_SOLVER_JAVA_TEST_CLASSPATH}
            tsp.solver.BindingTestMain)
else()
  add_test(
    NAME tsp_solver_java_tests
    COMMAND ${Java_JAVA_EXECUTABLE}
            -Dtsp.solver.library.path=$<TARGET_FILE:tsp_solver_java_jni>
            -cp ${TSP_SOLVER_JAVA_TEST_CLASSPATH}
            tsp.solver.BindingTestMain)
  add_test(
    NAME tsp_solver_java_tests_env
    COMMAND ${CMAKE_COMMAND} -E env
            TSP_SOLVER_LIBRARY_PATH=$<TARGET_FILE:tsp_solver>
            TSP_SOLVER_JAVA_LIBRARY_PATH=$<TARGET_FILE:tsp_solver_java_jni>
            ${Java_JAVA_EXECUTABLE}
            -cp ${TSP_SOLVER_JAVA_TEST_CLASSPATH}
            tsp.solver.BindingTestMain)
endif()

add_test(
  NAME tsp_solver_java_package_smoke
  COMMAND ${CMAKE_COMMAND}
          -Dcmake_command=${CMAKE_COMMAND}
          -Dbuild_dir=${CMAKE_BINARY_DIR}
          -Djava_executable=${Java_JAVA_EXECUTABLE}
          -Djavac_executable=${Java_JAVAC_EXECUTABLE}
          -Djar_file=${TSP_SOLVER_JAVA_JAR}
          -Dbundle_dir=${TSP_SOLVER_JAVA_NATIVE_BUNDLE_DIR}
          -Dsmoke_source=${CMAKE_SOURCE_DIR}/tests/java/package_smoke/PackageSmokeMain.java
          -Dsmoke_build_dir=${CMAKE_CURRENT_BINARY_DIR}/package-smoke-classes
          -Djni_library_name=$<TARGET_FILE_NAME:tsp_solver_java_jni>
          -P ${CMAKE_CURRENT_SOURCE_DIR}/package_smoke.cmake)

add_test(
  NAME tsp_solver_examples_java_smoke
  COMMAND ${CMAKE_COMMAND}
          -Djava_executable=${Java_JAVA_EXECUTABLE}
          -Djava_main_classes_dir=${TSP_SOLVER_JAVA_MAIN_CLASSES_DIR}
          -Djava_example_classes_dir=${TSP_SOLVER_JAVA_EXAMPLE_CLASSES_DIR}
          -Djni_library_path=$<TARGET_FILE:tsp_solver_java_jni>
          -P ${CMAKE_SOURCE_DIR}/tests/examples_java_smoke.cmake)

if(WIN32)
  set_tests_properties(tsp_solver_examples_java_smoke PROPERTIES
                       ENVIRONMENT "PATH=$<TARGET_FILE_DIR:tsp_solver_java_jni>;$<TARGET_FILE_DIR:tsp_solver>;$ENV{PATH}")
elseif(APPLE)
  set_tests_properties(tsp_solver_examples_java_smoke PROPERTIES
                       ENVIRONMENT "DYLD_LIBRARY_PATH=$<TARGET_FILE_DIR:tsp_solver_java_jni>:$<TARGET_FILE_DIR:tsp_solver>:$ENV{DYLD_LIBRARY_PATH}")
else()
  set_tests_properties(tsp_solver_examples_java_smoke PROPERTIES
                       ENVIRONMENT "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:tsp_solver_java_jni>:$<TARGET_FILE_DIR:tsp_solver>:$ENV{LD_LIBRARY_PATH}")
endif()
