cmake_minimum_required(VERSION 3.24)

project(tsp_solver VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(TSP_SOLVER_BUILD_SHARED "Build tsp_solver as a shared library" ON)
set(BUILD_SHARED_LIBS ${TSP_SOLVER_BUILD_SHARED})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(tsp_solver)
add_library(tsp_solver::tsp_solver ALIAS tsp_solver)

target_compile_definitions(tsp_solver
  PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:TSP_SOLVER_BUILDING_DLL>)

target_include_directories(tsp_solver
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_subdirectory(src)

if(BUILD_TESTING)
  add_subdirectory(tests)
  add_subdirectory(bindings/python)
  add_subdirectory(bindings/java)
endif()

install(TARGETS tsp_solver
  EXPORT tsp_solverTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT tsp_solverTargets
  FILE tsp_solverTargets.cmake
  NAMESPACE tsp_solver::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsp_solver)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/tsp_solverConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/tsp_solverConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsp_solver)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/tsp_solverConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/tsp_solverConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/tsp_solverConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tsp_solver)
