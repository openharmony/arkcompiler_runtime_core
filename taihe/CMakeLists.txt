# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.18)
project(TaiheCompiler)

# 设置 C 编译器为 clang
set(CMAKE_C_COMPILER "clang")

# 设置 C++ 编译器为 clang++
set(CMAKE_CXX_COMPILER "clang++")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 启用 ASan
# if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#     # 添加编译和链接时的 ASan 选项
#     add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
#     add_link_options(-fsanitize=address)
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address ")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address ")
# endif()

# 启用调试信息
set(CMAKE_BUILD_TYPE Debug)

# 为编译器设置 -fPIC
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(TH_STDLIB_DIR "${CMAKE_SOURCE_DIR}/stdlib")

# taihe runtime .c/.cpp path
set(TH_RUNTIME_DIR "${CMAKE_SOURCE_DIR}/runtime")
set(TH_RUNTIME_INCLUDE_DIR "${TH_RUNTIME_DIR}/include")
set(TH_RUNTIME_SOURCE_DIR "${TH_RUNTIME_DIR}/src")

# 设置 Python
set(Python3_EXECUTABLE "/usr/bin/python3")
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# 启用或禁用 coverage
option(ENABLE_COVERAGE "Enable coverage run for the Python command" OFF)

# 编译 taihe runtime 静态库
set(TH_SOURCE
  "${TH_RUNTIME_SOURCE_DIR}/string.cpp"
  "${TH_RUNTIME_SOURCE_DIR}/object.cpp"
  "${TH_RUNTIME_SOURCE_DIR}/runtime.cpp"
)

add_library(th_runtime STATIC ${TH_SOURCE})

set_target_properties(th_runtime PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

target_include_directories(th_runtime PUBLIC ${TH_RUNTIME_INCLUDE_DIR})


function(update_grammar)
  add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/compiler/taihe/parse/antlr/TaiheAST.py
    COMMAND ./generate-grammar
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/compiler # 设置工作目录
    COMMENT "Update grammar"
    DEPENDS ${CMAKE_SOURCE_DIR}/compiler/Taihe.g4
  )

  add_custom_target(UPDATE_GRAMMAR ALL
    DEPENDS ${CMAKE_SOURCE_DIR}/compiler/taihe/parse/antlr/TaiheAST.py
  )
endfunction()


function(generate_code_from_idl target_name idl_files gen_ets_files generated_dir taihe_configs)
  set(IDL_FILES ${idl_files})
  set(GEN_ETS_FILES ${gen_ets_files})

  set(PROJ_HPP_FILES)
  set(IMPL_HPP_FILES)
  set(ABI_H_FILES)
  set(ABI_C_FILES)
  set(ANI_HPP_FILES)
  set(ANI_CPP_FILES)

  foreach(TAIHE_FILE ${IDL_FILES})
    # 替换扩展名
    get_filename_component(TAIHE_FILE_NAME ${TAIHE_FILE} NAME)

    string(REGEX REPLACE "\\.taihe$" ".proj.hpp" PROJ_HPP_FILE ${TAIHE_FILE_NAME})
    string(REGEX REPLACE "\\.taihe$" ".impl.hpp" IMPL_HPP_FILE ${TAIHE_FILE_NAME})
    string(REGEX REPLACE "\\.taihe$" ".abi.h" ABI_H_FILE ${TAIHE_FILE_NAME})
    string(REGEX REPLACE "\\.taihe$" ".abi.c" ABI_C_FILE ${TAIHE_FILE_NAME})
    string(REGEX REPLACE "\\.taihe$" ".ani.hpp" ANI_HPP_FILE ${TAIHE_FILE_NAME})
    string(REGEX REPLACE "\\.taihe$" ".ani.cpp" ANI_CPP_FILE ${TAIHE_FILE_NAME})

    # 将修改后的文件名添加到新的列表中
    list(APPEND PROJ_HPP_FILES "${generated_dir}/include/${PROJ_HPP_FILE}")
    list(APPEND IMPL_HPP_FILES "${generated_dir}/include/${IMPL_HPP_FILE}")
    list(APPEND ABI_H_FILES "${generated_dir}/include/${ABI_H_FILE}")
    list(APPEND ABI_C_FILES "${generated_dir}/src/${ABI_C_FILE}")
    list(APPEND ANI_HPP_FILES "${generated_dir}/include/${ANI_HPP_FILE}")
    list(APPEND ANI_CPP_FILES "${generated_dir}/src/${ANI_CPP_FILE}")
  endforeach()

  if(ENABLE_COVERAGE)
    list(APPEND COMMAND_TO_RUN "coverage" "run" "--parallel-mode" "-m")
  else()
    list(APPEND COMMAND_TO_RUN "python3" "-m")
  endif()

  list(APPEND COMMAND_TO_RUN
    "taihe.cli.compiler"
    "${idl_files}"
    "-O" "${generated_dir}"
    "--generate" "ani-bridge" "cpp-author" "pretty-print"
    "${taihe_configs}"
  )

  add_custom_command(
    OUTPUT ${PROJ_HPP_FILES} ${IMPL_HPP_FILES} ${ABI_H_FILES} ${ABI_C_FILES} ${ANI_HPP_FILES} ${ANI_CPP_FILES} ${GEN_ETS_FILES}
    COMMAND ${COMMAND_TO_RUN}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/compiler
    DEPENDS ${IDL_FILES} ${CMAKE_SOURCE_DIR}/compiler/taihe/parse/antlr/TaiheAST.py
    COMMENT "Generating Taihe C++ header and source files..."
    VERBATIM
  )

  add_custom_target(${target_name}_gen
    DEPENDS ${PROJ_HPP_FILES}
    ${IMPL_HPP_FILES}
    ${ABI_H_FILES}
    ${ABI_C_FILES}
    ${ANI_HPP_FILES}
    ${ANI_CPP_FILES}
    ${GEN_ETS_FILES}
  )

  set(${target_name}_PROJ_HPP_FILES ${PROJ_HPP_FILES} PARENT_SCOPE)
  set(${target_name}_IMPL_HPP_FILES ${IMPL_HPP_FILES} PARENT_SCOPE)
  set(${target_name}_ABI_H_FILES ${ABI_H_FILES} PARENT_SCOPE)
  set(${target_name}_ABI_C_FILES ${ABI_C_FILES} PARENT_SCOPE)
  set(${target_name}_ANI_HPP_FILES ${ANI_HPP_FILES} PARENT_SCOPE)
  set(${target_name}_ANI_CPP_FILES ${ANI_CPP_FILES} PARENT_SCOPE)
endfunction()

# 准备 panda sdk
function(setup_panda_sdk)
  # 用户显式提供了 PANDA_HOME，不执行下载逻辑
  if(DEFINED PANDA_HOME AND IS_DIRECTORY "${PANDA_HOME}")
    message(STATUS "Using user-provided PANDA_HOME: ${PANDA_HOME}")
    set(ENV{PANDA_HOME} "${PANDA_HOME}")
    # 自动下载模式
  else()
    # 设置 PANDA 路径
    set(PANDA_SDK_FILENAME "sdk-1.5.0-dev.36922.tgz")
    set(PANDA_SDK_URL "https://nexus.bz-openlab.ru:10443/repository/koala-npm/%40panda/sdk/-/${PANDA_SDK_FILENAME}")
    set(PANDA_EXTRACT_DIR "${CMAKE_SOURCE_DIR}/.panda_vm")
    set(PANDA_SDK_PATH "${PANDA_EXTRACT_DIR}/${PANDA_SDK_FILENAME}")
    set(PANDA_HOME "${PANDA_EXTRACT_DIR}/package/linux_host_tools")
    set(PANDA_SDK_VERSION_FILE "${PANDA_EXTRACT_DIR}/package/version.txt")

    file(MAKE_DIRECTORY "${PANDA_EXTRACT_DIR}")
    string(REPLACE ".tgz" "" PANDA_SDK_VERSION "${PANDA_SDK_FILENAME}")

    # 检查当前已提取版本
    if(EXISTS "${PANDA_SDK_VERSION_FILE}")
      file(READ "${PANDA_SDK_VERSION_FILE}" EXISTING_PANDA_SDK_VERSION)
      string(STRIP "${EXISTING_PANDA_SDK_VERSION}" EXISTING_PANDA_SDK_VERSION)
    else()
      set(EXISTING_PANDA_SDK_VERSION "")
    endif()

    # 检查版本是否匹配
    if(NOT "${EXISTING_PANDA_SDK_VERSION}" STREQUAL "${PANDA_SDK_VERSION}")
      message(STATUS "Panda SDK version mismatch or not found. Expected: ${PANDA_SDK_VERSION}, Found: ${EXISTING_PANDA_SDK_VERSION}")
      if(NOT "${PANDA_HOME}" STREQUAL "")
        file(REMOVE_RECURSE "${PANDA_HOME}")
      endif()
      set(NEED_DOWNLOAD TRUE)
    else()
      set(NEED_DOWNLOAD FALSE)
    endif()

    # 如果需要重新准备 SDK
    if(NEED_DOWNLOAD)
      # 先检查本地是否已有对应的 SDK 包
      set(LOCAL_SDK_PATH "${PANDA_EXTRACT_DIR}/${PANDA_SDK_VERSION}.tgz")
      if(EXISTS "${LOCAL_SDK_PATH}")
        message(STATUS "Found local Panda SDK package: ${LOCAL_SDK_PATH}")
        set(PANDA_SDK_PATH "${LOCAL_SDK_PATH}")
        set(NEED_ACTUAL_DOWNLOAD FALSE)
      else()
        message(STATUS "No local Panda SDK package found, will download: ${PANDA_SDK_FILENAME}")
        set(NEED_ACTUAL_DOWNLOAD TRUE)
      endif()

      # 如果确实找不到，就去下载
      if(NEED_ACTUAL_DOWNLOAD)
        message(STATUS "Downloading Panda SDK: ${PANDA_SDK_FILENAME}")
        execute_process(
          COMMAND curl -u koala-pub:y3t!n0therP -L -o "${PANDA_EXTRACT_DIR}/${PANDA_SDK_FILENAME}" "${PANDA_SDK_URL}"
          RESULT_VARIABLE DOWNLOAD_RESULT
        )
        if(NOT DOWNLOAD_RESULT EQUAL 0)
          message(FATAL_ERROR "Failed to download Panda SDK!")
        endif()
        set(PANDA_SDK_PATH "${PANDA_EXTRACT_DIR}/${PANDA_SDK_FILENAME}")
      endif()

      # 删除原来的解压目录
      file(REMOVE_RECURSE "${PANDA_HOME}")
    endif()

    # Extract Panda SDK if needed
    if(NOT EXISTS "${PANDA_HOME}")
      message(STATUS "Extracting Panda SDK from ${PANDA_SDK_PATH} ...")
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${PANDA_SDK_PATH}"
        WORKING_DIRECTORY "${PANDA_EXTRACT_DIR}"
        RESULT_VARIABLE EXTRACT_RESULT
      )
      if(NOT EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract Panda SDK!")
      endif()

      # 保存当前版本信息
      file(WRITE "${PANDA_SDK_VERSION_FILE}" "${PANDA_SDK_VERSION}")
    endif()

    # Set environment variable
    set(ENV{PANDA_HOME} "${PANDA_HOME}")
    set(PANDA_HOME "${PANDA_HOME}" PARENT_SCOPE)
  endif()

  # Set ETS compiler path
  list(APPEND CMAKE_PROGRAM_PATH "$ENV{PANDA_HOME}/bin")

  find_program(ETS_COMPILER es2panda)
  find_program(ETS_RUNTIME ark)
  find_program(ETS_DISASM ark_disasm)
  find_program(ETS_LINK ark_link)

  if(NOT ETS_COMPILER)
    message(FATAL_ERROR "ets_compiler not found! Please set ETS_COMPILER_PATH or ensure ets_compiler is in your PATH.")
  else()
    message(STATUS "Found ets_compiler: ${ETS_COMPILER}")
  endif()
endfunction()


# 编译动态库
function(compile_dylib target_name include_dirs user_cpp_files)
  set(ALL_SRC_FILES ${${target_name}_ABI_C_FILES} ${${target_name}_ANI_CPP_FILES} ${user_cpp_files})

  foreach(author_source_file ${user_cpp_files})
    foreach(impl_hpp_file ${${target_name}_IMPL_HPP_FILES})
      set_source_files_properties(${author_source_file} PROPERTIES OBJECT_DEPENDS ${impl_hpp_file})
    endforeach()
  endforeach()

  add_library(${target_name} SHARED ${ALL_SRC_FILES})

  add_dependencies(${target_name} th_runtime ${target_name}_gen)
  target_compile_options(${target_name} PRIVATE "-Wno-attributes")
  set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)
  target_link_libraries(${target_name} PRIVATE th_runtime)
  target_link_options(${target_name} PRIVATE "-Wl,--no-undefined")
  target_include_directories(${target_name} PUBLIC ${include_dirs} TH_RUNTIME_INCLUDE_DIR)
endfunction()


# 生成 arktsconfig.json
function(write_arkts_config config_path ets_files)
  foreach(file IN LISTS ets_files)
    get_filename_component(file_name ${file} NAME)
    string(REGEX REPLACE "\\.ets$" "" stripped_file "${file_name}")
    set(entry "      \"${stripped_file}\": [\"${file}\"]")
    list(APPEND ETS_PATH_ENTRIES "${entry}")
  endforeach()

  string(REPLACE ";" ",\n" ETS_PATH_ENTRIES_JOINED "${ETS_PATH_ENTRIES}")

  file(WRITE "${config_path}"
    "{\n"
    "  \"compilerOptions\": {\n"
    "    \"baseUrl\": \"${PANDA_HOME}\",\n"
    "    \"paths\": {\n"
    "      \"std\": [\"${PANDA_HOME}/../ets/stdlib/std\"],\n"
    "      \"escompat\": [\"${PANDA_HOME}/../ets/stdlib/escompat\"],\n"
    "${ETS_PATH_ENTRIES_JOINED}\n"
    "    }\n"
    "  }\n"
    "}\n"
  )
endfunction()


# 自定义 ETS 构建规则
function(build_ets_target ets_file output_dir abc_file dump_file dep_ets_files dep_so_file build_demo_dir arktsconfig)
  # 创建输出目录（如果不存在）
  file(MAKE_DIRECTORY ${output_dir})

  add_custom_command(
    OUTPUT ${abc_file} # 输出文件
    COMMAND ${ETS_COMPILER} ${ets_file}
    --output ${abc_file}
    --extension ets
    --arktsconfig ${arktsconfig} # 生成命令
    && ${ETS_DISASM} ${abc_file} ${dump_file}
    DEPENDS ${ets_file} ${dep_ets_files} ${dep_so_file} ${arktsconfig} # 输入文件依赖
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} # 工作目录
    COMMENT "Building ${ets_file} ETS target" # 注释
  )
endfunction()


function(abc_link target_name ets_files build_demo_dir arktsconfig main_abc)
  write_arkts_config(${arktsconfig} "${ets_files}")

  set(ABC_FILES "")

  set(BUILD_DIR "${build_demo_dir}/build")

  # 为每个 ETS 文件创建编译目标
  foreach(ETS_FILE ${ets_files})
    get_filename_component(ETS_NAME_P ${ETS_FILE} NAME)
    string(REGEX REPLACE "\\.[^.]*$" "" ETS_NAME "${ETS_NAME_P}")

    set(ABC_FILE "${BUILD_DIR}/${ETS_NAME}.ets.abc")
    set(DUMP_FILE "${BUILD_DIR}/${ETS_NAME}.ets.abc.dump")

    build_ets_target(${ETS_FILE} ${BUILD_DIR} ${ABC_FILE} ${DUMP_FILE} "${ets_files}" ${target_name} ${build_demo_dir} ${arktsconfig})
    list(APPEND ABC_FILES "${ABC_FILE}")
  endforeach()

  # 创建链接命令
  add_custom_command(
    OUTPUT ${main_abc}
    COMMAND ${ETS_LINK} --output ${main_abc} -- ${ABC_FILES}
    DEPENDS ${ABC_FILES}
    COMMENT "Linking all ABC files to main.abc"
  )
endfunction()


function(run_abc target_name build_demo_dir main_abc)
  # 设置
  file(MAKE_DIRECTORY ${build_demo_dir})
  get_filename_component(target_name ${target_name} NAME_WE)

  # 创建运行目标，并明确依赖于链接目标
  add_custom_target(${target_name}_run ALL
    WORKING_DIRECTORY ${build_demo_dir}
    COMMAND # LD_PRELOAD=/usr/lib/llvm-14/lib/clang/14.0.0/lib/linux/libclang_rt.asan-x86_64.so
    LD_LIBRARY_PATH=${build_demo_dir} ${ETS_RUNTIME}
    --boot-panda-files=$ENV{PANDA_HOME}/../ets/etsstdlib.abc
    --load-runtimes=ets ${build_demo_dir}/main.abc main.ETSGLOBAL::main
    && echo "Run successful" || (echo "Run failed" && exit 1)
    COMMENT "Running ${target_name}"
    DEPENDS ${main_abc}
  )
endfunction()


function(add_ani_demo target_name idl_files taihe_configs gen_ets_names user_ets_files user_include_dir user_cpp_files run_dir)
  set(DEMO_DIR "${CMAKE_SOURCE_DIR}/${run_dir}/${target_name}")
  set(BUILD_DEMO_DIR "${CMAKE_BINARY_DIR}/${run_dir}/${target_name}")
  set(GEN_DIR "${BUILD_DEMO_DIR}/generated")
  set(GEN_INCLUDE_DIR "${GEN_DIR}/include")
  set(AUTHOR_INCLUDE_DIRS ${GEN_INCLUDE_DIR} ${user_include_dir})
  set(BUILD_DIR "${BUILD_DEMO_DIR}/build")
  set(ARKTSCONFIG "${BUILD_DEMO_DIR}/arktsconfig.json")
  set(IDL_FILES ${idl_files} "${TH_STDLIB_DIR}/taihe.platform.ani.taihe")
  set(GEN_ETS_NAMES ${gen_ets_names} "taihe.platform.ani.ets")
  set(GEN_ETS_FILES "")
  foreach(ETS_NAME ${GEN_ETS_NAMES})
    set(ETS_FILE "${GEN_DIR}/${ETS_NAME}")
    list(APPEND GEN_ETS_FILES "${ETS_FILE}")
  endforeach()
  set(ALL_ETS_FILES ${user_ets_files} ${GEN_ETS_FILES})
  set(MAIN_ABC "${BUILD_DEMO_DIR}/main.abc")

  # 生成代码
  generate_code_from_idl(${target_name} "${IDL_FILES}" "${GEN_ETS_FILES}" ${GEN_DIR} "${taihe_configs}")
  # 动态库编译
  compile_dylib(${target_name} "${AUTHOR_INCLUDE_DIRS}" "${user_cpp_files}")
  # 链接为 main.abc
  abc_link(${target_name} "${ALL_ETS_FILES}" ${BUILD_DEMO_DIR} ${ARKTSCONFIG} ${MAIN_ABC})
  # 运行
  run_abc(${target_name} ${BUILD_DEMO_DIR} ${MAIN_ABC})
endfunction()


setup_panda_sdk()

# INCLUDE_ANI_HEADER
set(ANI_HEADER $ENV{PANDA_HOME}/../ohos_arm64/include/plugins/ets/runtime/ani)
if(EXISTS "${ANI_HEADER}")
  message(STATUS "Adding Panda include directory: ${ANI_HEADER}")
  include_directories("${ANI_HEADER}")
else()
  message(FATAL_ERROR "Cannot find the path: ${ANI_HEADER}")
endif()

update_grammar()

add_subdirectory(test)
#add_subdirectory(cookbook)