cmake_minimum_required(VERSION 3.24)

project(tsp_solver VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(tsp_solver)
add_library(tsp_solver::tsp_solver ALIAS tsp_solver)

target_include_directories(tsp_solver
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_subdirectory(src)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(TARGETS tsp_solver
  EXPORT tsp_solverTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
