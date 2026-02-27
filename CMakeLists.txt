#
# This file is part of Extensive Pore Modelling (xpm).
#   | https://github.com/dp-69/xpm
#
# Copyright (c) 2024
#   | Dmytro Petrovskyy, PhD
#   | dmytro.petrovsky@gmail.com
#   | https://www.linkedin.com/in/dmytro-petrovskyy/
#
# xpm is free software: you can redistribute it and/or modify              
# it under the terms of the GNU General Public License as published by     
# the Free Software Foundation, either version 3 of the License, or        
# (at your option) any later version.                                      
#                                                                         
# xpm is distributed in the hope that it will be useful,                   
# but WITHOUT ANY WARRANTY; without even the implied warranty of           
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            
# GNU General Public License for more details.                             
#                                                                         
# You should have received a copy of the GNU General Public License        
# along with xpm. If not, see <http://www.gnu.org/licenses/>.
#
# 

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

project(xpm_project)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_AUTOMOC ON)

if(MSVC)  
  string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

  #set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
  #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
endif()



add_executable(xpm # WIN32
  "src/main.cpp"
)

target_include_directories(xpm PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_compile_features(xpm PUBLIC cxx_std_20)

target_compile_definitions(xpm PRIVATE XPM_DEBUG_OUTPUT)


add_subdirectory(libs/dpl)
target_link_libraries(xpm PRIVATE dpl)

add_subdirectory(libs/pnextract)

if(MSVC)  
  target_compile_options(pnextract PRIVATE -w)
  target_compile_options(xpm PRIVATE -W4)

  #if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  #  target_compile_options(xpm PRIVATE -bigobj)
  #endif()

  target_compile_options(xpm PRIVATE -bigobj)
else()
  #add_compile_options("-w")
endif()


find_package(Boost COMPONENTS iostreams graph REQUIRED)
target_link_libraries(xpm PRIVATE Boost::iostreams Boost::graph)

target_include_directories(xpm PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/argh")

find_package(fmt CONFIG REQUIRED)
target_link_libraries(xpm PRIVATE fmt::fmt)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(xpm PRIVATE nlohmann_json::nlohmann_json)


find_package(MPI)
if(MPI_FOUND)
  target_link_libraries(xpm PRIVATE MPI::MPI_CXX)
endif()


if(TRUE)
  set(HYPRE_ENABLE_HYPRE_BLAS OFF)
  set(HYPRE_ENABLE_HYPRE_LAPACK OFF)
  set(HYPRE_ENABLE_MIXEDINT OFF CACHE BOOL "" FORCE)
  set(HYPRE_ENABLE_BIGINT OFF CACHE BOOL "" FORCE)
  set(HYPRE_ENABLE_SINGLE OFF CACHE BOOL "" FORCE)
  set(HYPRE_PRINT_ERRORS OFF CACHE BOOL "" FORCE)
  set(HYPRE_ENABLE_SHARED OFF CACHE BOOL "" FORCE)
  
  include(FetchContent)
  FetchContent_Declare(
    hypre
    GIT_REPOSITORY https://github.com/hypre-space/hypre.git
    GIT_TAG        v2.31.0
    SOURCE_SUBDIR  src
  )
  FetchContent_MakeAvailable(hypre)

  target_include_directories(xpm PRIVATE "${hypre_SOURCE_DIR}/src")
  target_link_libraries(xpm PRIVATE HYPRE)
else()
  find_package(HYPRE REQUIRED)
  target_link_libraries(xpm PRIVATE HYPRE::HYPRE)
endif()


if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(NOT EXISTS ${OUTPUT_DIR}/config.json)
  file(MAKE_DIRECTORY ${OUTPUT_DIR})
  file(COPY_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/files/config.json"
    "${OUTPUT_DIR}/config.json")
endif()