# Based on https://jonathanhamberg.com/post/cmake-embedding-git-hash/
#
# Example:
#  CMakeLists.txt
#     cmake_minimum_required(VERSION 3.8.0)
#     project(test_gitv)
#     set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
#     include(CheckGit)
#     CheckGitSetup(${PROJECT_NAME})
#     
#     add_executable(${PROJECT_NAME} main.cc)
#     target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_version)
#
#  main.cc
#    #include <iostream>
#    #include "test_gitv_version.h"
#    
#    int main(void) {
#      std::cout << test_gitv::kGitHash << std::endl;
#      return 0;
#    }


set(CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})
if (NOT DEFINED pre_configure_dir)
  set(pre_configure_dir ${CMAKE_CURRENT_LIST_DIR})
endif ()

if (NOT DEFINED post_configure_dir)
  set(post_configure_dir ${CMAKE_BINARY_DIR}/generated)
endif ()

if (NOT DEFINED GIT_MODULE)
  set(GIT_MODULE ${PROJECT_NAME})
endif()

set(pre_configure_header ${pre_configure_dir}/git_version.h.in)
set(pre_configure_cpp ${pre_configure_dir}/git_version.cpp.in)
set(post_configure_header ${post_configure_dir}/${GIT_MODULE}_version.h)
set(post_configure_cpp ${post_configure_dir}/${GIT_MODULE}_version.cpp)

function(CheckGitWrite git_hash)
  file(WRITE ${post_configure_dir}/git-state-${GIT_MODULE}.txt ${git_hash})
endfunction()

function(CheckGitRead git_hash)
  if (EXISTS ${post_configure_dir}/git-state-${GIT_MODULE}.txt)
    file(STRINGS ${post_configure_dir}/git-state-${GIT_MODULE}.txt CONTENT)
    LIST(GET CONTENT 0 var)
    set(${git_hash} ${var} PARENT_SCOPE)
  endif ()
endfunction()

function(CheckGitVersion)
  # Get the latest abbreviated commit hash of the working branch
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  CheckGitRead(GIT_HASH_CACHE)
  if (NOT EXISTS ${post_configure_dir})
    file(MAKE_DIRECTORY ${post_configure_dir})
  endif ()

  if (NOT EXISTS ${post_configure_header})
    configure_file(${pre_configure_header} ${post_configure_header} @ONLY)
  endif()

  if (NOT DEFINED GIT_HASH_CACHE)
    set(GIT_HASH_CACHE "INVALID")
  endif ()

  # Only update the git_version.cpp if the hash has changed. This will
  # prevent us from rebuilding the project more than we need to.
  if (NOT ${GIT_HASH} STREQUAL ${GIT_HASH_CACHE} OR NOT EXISTS ${post_configure_cpp})
    # Set che GIT_HASH_CACHE variable the next build won't have
    # to regenerate the source file.
    CheckGitWrite(${GIT_HASH})

    configure_file(${pre_configure_cpp} ${post_configure_cpp} @ONLY)
  endif ()
endfunction()

function(CheckGitSetup module_name)
  add_custom_target(AlwaysCheckGit_${module_name} COMMAND ${CMAKE_COMMAND}
    -DGIT_MODULE=${module_name}
    -DRUN_CHECK_GIT_VERSION=1
    -Dpre_configure_dir=${pre_configure_dir}
    -Dpost_configure_dir=${post_configure_dir}
    -DGIT_HASH_CACHE=${GIT_HASH_CACHE}
    -P ${CURRENT_LIST_DIR}/CheckGit.cmake
    BYPRODUCTS ${post_configure_cpp} ${post_configure_header})

  add_library(${module_name}_version ${post_configure_cpp})
  target_include_directories(${module_name}_version PUBLIC ${post_configure_dir})
  add_dependencies(${module_name}_version AlwaysCheckGit_${module_name})

  CheckGitVersion()

  unset(GIT_MODULE PARENT_SCOPE)
  unset(pre_configure_dir PARENT_SCOPE)
endfunction()

# This is used to run this function from an external cmake process.
if (RUN_CHECK_GIT_VERSION)
  CheckGitVersion()
endif ()
