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

function(execute_and_set_variable OUTPUT_VAR_NAME)
  execute_process(
    COMMAND taihec ${ARGN}
    OUTPUT_VARIABLE _output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _result
  )

  if(_result EQUAL 0)
    set(${OUTPUT_VAR_NAME} "${_output}" PARENT_SCOPE)
    message(STATUS "${OUTPUT_VAR_NAME} set to: ${_output}")
  else()
    message(FATAL_ERROR "Failed to execute 'taihec ${COMMAND_ARGS}'. Error code: ${_result}")
  endif()
endfunction()

if(ENABLE_COVERAGE)
  set(TAIHEC coverage run --parallel-mode -m taihe.cli.compiler)
else()
  set(TAIHEC taihec)
endif()

execute_and_set_variable(TAIHE_RUNTIME_INCLUDE_DIR "--print-runtime-header-path")
execute_and_set_variable(TAIHE_RUNTIME_SRC_DIR "--print-runtime-source-path")
set(TAIHE_RUNTIME_BUILD_DIR "${CMAKE_BINARY_DIR}/runtime")

# 设置 Python
set(Python3_EXECUTABLE "/usr/bin/python3")
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# 启用或禁用 coverage
option(ENABLE_COVERAGE "Enable coverage run for the Python command" OFF)

# 编译 taihe runtime 静态库
set(TAIHE_RUNTIME_SOURCES
  "${TAIHE_RUNTIME_SRC_DIR}/string.cpp"
  "${TAIHE_RUNTIME_SRC_DIR}/object.cpp"
  "${TAIHE_RUNTIME_SRC_DIR}/runtime.cpp"
)

add_library(taihe_runtime STATIC ${TAIHE_RUNTIME_SOURCES})

set_target_properties(taihe_runtime PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${TAIHE_RUNTIME_BUILD_DIR}
)

target_include_directories(taihe_runtime PUBLIC ${TAIHE_RUNTIME_INCLUDE_DIR})

execute_and_set_variable(TAIHE_STDLIB_DIR "--print-stdlib-path")
set(TAIHE_STDLIB_GEN_DIR "${CMAKE_BINARY_DIR}/stdlib/generated")
set(TAIHE_STDLIB_GEN_INCLUDE_DIR "${TAIHE_STDLIB_GEN_DIR}/include")
set(TAIHE_STDLIB_GEN_SRC_DIR "${TAIHE_STDLIB_GEN_DIR}/src")
set(TAIHE_STDLIB_BUILD_DIR "${CMAKE_BINARY_DIR}/stdlib/build")

set(TAIHE_STDLIB_IDL_FILES
  "${TAIHE_STDLIB_DIR}/taihe.platform.ani.taihe"
)

set(TAIHE_STDLIB_GEN_SOURCES
  "${TAIHE_STDLIB_GEN_SRC_DIR}/taihe.platform.ani.abi.c"
)

add_custom_command(
  OUTPUT ${TAIHE_STDLIB_GEN_SOURCES} ${TAIHE_STDLIB_GEN_INCLUDE_DIR}
  COMMAND ${TAIHEC}
  ${TAIHE_STDLIB_IDL_FILES}
  -O${TAIHE_STDLIB_GEN_DIR}
  -Gcpp-common -Gabi-source
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS ${TAIHE_STDLIB_IDL_FILES} ${CMAKE_SOURCE_DIR}/compiler/taihe/parse/antlr/TaiheAST.py
  COMMENT "Generating Taihe standard library C++ header and source files..."
  VERBATIM
)

add_library(taihe_stdlib STATIC ${TAIHE_STDLIB_GEN_SOURCES})

set_target_properties(taihe_stdlib PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${TAIHE_STDLIB_BUILD_DIR}
)

target_include_directories(taihe_stdlib PUBLIC ${TAIHE_STDLIB_GEN_INCLUDE_DIR} ${TAIHE_RUNTIME_INCLUDE_DIR})

function(generate_code_from_idl demo_name idl_files gen_ets_names taihe_configs gen_include_dir gen_abi_c_files gen_ani_cpp_files gen_ets_files)
  set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

  set(GEN_INCLUDE_DIR "${GEN_DIR}/include")

  set(GEN_ABI_C_FILES)
  set(GEN_ANI_CPP_FILES)
  foreach(TAIHE_FILE ${idl_files})
    # 替换扩展名
    get_filename_component(TAIHE_FILE_NAME ${TAIHE_FILE} NAME)
    # 将修改后的文件名添加到新的列表中
    string(REGEX REPLACE "\\.taihe$" ".abi.c" GEN_ABI_C_FILE ${TAIHE_FILE_NAME})
    list(APPEND GEN_ABI_C_FILES "${GEN_DIR}/src/${GEN_ABI_C_FILE}")
    string(REGEX REPLACE "\\.taihe$" ".ani.cpp" GEN_ANI_CPP_FILE ${TAIHE_FILE_NAME})
    list(APPEND GEN_ANI_CPP_FILES "${GEN_DIR}/src/${GEN_ANI_CPP_FILE}")
  endforeach()

  set(GEN_ETS_FILES)
  foreach(ETS_NAME ${gen_ets_names})
    set(ETS_FILE "${GEN_DIR}/${ETS_NAME}")
    list(APPEND GEN_ETS_FILES "${ETS_FILE}")
  endforeach()

  add_custom_command(
    OUTPUT ${GEN_INCLUDE_DIR} ${GEN_ABI_C_FILES} ${GEN_ANI_CPP_FILES} ${GEN_ETS_FILES}
    COMMAND ${TAIHEC}
    ${idl_files}
    -O${GEN_DIR}
    -Gani-bridge -Gcpp-author -Gpretty-print
    ${taihe_configs}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${idl_files} ${CMAKE_SOURCE_DIR}/compiler/taihe/parse/antlr/TaiheAST.py
    COMMENT "Generating Taihe C++ header and source files... ${GEN_DIR}"
    VERBATIM
  )

  set(${gen_ets_files} ${GEN_ETS_FILES} PARENT_SCOPE)
  set(${gen_abi_c_files} ${GEN_ABI_C_FILES} PARENT_SCOPE)
  set(${gen_ani_cpp_files} ${GEN_ANI_CPP_FILES} PARENT_SCOPE)
  set(${gen_include_dir} ${GEN_INCLUDE_DIR} PARENT_SCOPE)
endfunction()

# 准备 panda sdk
function(setup_panda_sdk)
  # 用户显式提供了 PANDA_HOME，不执行下载逻辑
  if(DEFINED PANDA_HOME AND IS_DIRECTORY "${PANDA_HOME}")
    message(STATUS "Using user-provided PANDA_HOME: ${PANDA_HOME}")
    set(ENV{PANDA_HOME} "${PANDA_HOME}")
    # 自动下载模式
  else()
    execute_and_set_variable(PANDA_EXTRACT_DIR "--print-panda-vm-path")
    set(PANDA_HOME "${PANDA_EXTRACT_DIR}/linux_host_tools")
    set(ENV{PANDA_HOME} "${PANDA_HOME}")
    set(PANDA_HOME "${PANDA_HOME}" PARENT_SCOPE)
    message(STATUS "PANDA_HOME set to: ${PANDA_HOME}")
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
function(compile_dylib demo_name user_include_dir user_cpp_files gen_include_dir gen_abi_c_files gen_ani_cpp_files)
  add_library(${demo_name} SHARED ${gen_abi_c_files} ${gen_ani_cpp_files} ${user_cpp_files})

  target_compile_options(${demo_name} PRIVATE "-Wno-attributes")
  set_target_properties(${demo_name} PROPERTIES LINKER_LANGUAGE CXX)
  target_link_libraries(${demo_name} PRIVATE taihe_runtime taihe_stdlib)
  target_link_options(${demo_name} PRIVATE "-Wl,--no-undefined")
  target_include_directories(${demo_name} PUBLIC ${user_include_dir} ${gen_include_dir} ${TAIHE_RUNTIME_INCLUDE_DIR} ${TAIHE_STDLIB_GEN_INCLUDE_DIR})
endfunction()


# 生成 arktsconfig.json
function(write_arkts_config arktsconfig ets_files)
  foreach(file IN LISTS ets_files)
    get_filename_component(file_name ${file} NAME)
    string(REGEX REPLACE "\\.ets$" "" stripped_file "${file_name}")
    set(entry "      \"${stripped_file}\": [\"${file}\"]")
    list(APPEND ETS_PATAIHE_ENTRIES "${entry}")
  endforeach()

  string(REPLACE ";" ",\n" ETS_PATAIHE_ENTRIES_JOINED "${ETS_PATAIHE_ENTRIES}")

  file(WRITE "${arktsconfig}"
    "{\n"
    "  \"compilerOptions\": {\n"
    "    \"baseUrl\": \"${PANDA_HOME}\",\n"
    "    \"paths\": {\n"
    "      \"std\": [\"${PANDA_HOME}/../ets/stdlib/std\"],\n"
    "      \"escompat\": [\"${PANDA_HOME}/../ets/stdlib/escompat\"],\n"
    "      \"@ohos\": [\"${PANDA_HOME}/../ets/sdk/sdk/api/@ohos\"],\n"
    "${ETS_PATAIHE_ENTRIES_JOINED}\n"
    "    }\n"
    "  }\n"
    "}\n"
  )
endfunction()


# 自定义 ETS 构建规则
function(build_ets_target ets_file output_dir abc_file dump_file ets_files arktsconfig)
  # 创建输出目录（如果不存在）
  file(MAKE_DIRECTORY ${output_dir})

  add_custom_command(
    OUTPUT ${abc_file} # 输出文件
    COMMAND ${ETS_COMPILER} ${ets_file}
    --output ${abc_file}
    --extension ets
    --arktsconfig ${arktsconfig} # 生成命令
    && ${ETS_DISASM} ${abc_file} ${dump_file}
    DEPENDS ${ets_file} ${ets_files} ${arktsconfig} # 输入文件依赖
    COMMENT "Building ${ets_file} ETS target" # 注释
  )
endfunction()


function(abc_link demo_name main_abc ets_files)
  set(ABC_FILES)
  set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/build")
  set(ARKTSCONFIG "${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json")

  write_arkts_config(${ARKTSCONFIG} "${ets_files}")

  # 为每个 ETS 文件创建编译目标
  foreach(ETS_FILE ${ets_files})
    get_filename_component(ETS_NAME_P ${ETS_FILE} NAME)
    string(REGEX REPLACE "\\.[^.]*$" "" ETS_NAME "${ETS_NAME_P}")

    set(ABC_FILE "${BUILD_DIR}/${ETS_NAME}.ets.abc")
    set(DUMP_FILE "${BUILD_DIR}/${ETS_NAME}.ets.abc.dump")

    build_ets_target(${ETS_FILE} ${BUILD_DIR} ${ABC_FILE} ${DUMP_FILE} "${ets_files}" ${ARKTSCONFIG})
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


function(run_abc demo_name main_abc)
  # 设置
  get_filename_component(demo_name ${demo_name} NAME_WE)

  # 创建运行目标，并明确依赖于链接目标
  add_custom_target(${demo_name}_run ALL
    COMMAND # LD_PRELOAD=/usr/lib/llvm-14/lib/clang/14.0.0/lib/linux/libclang_rt.asan-x86_64.so
    LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR} ${ETS_RUNTIME}
    --boot-panda-files=$ENV{PANDA_HOME}/../ets/etsstdlib.abc
    --boot-panda-files=$ENV{PANDA_HOME}/../ets/etssdk.abc
    --load-runtimes=ets ${main_abc} main.ETSGLOBAL::main
    && echo "Run successful" || (echo "Run failed" && exit 1)
    COMMENT "Running ${demo_name}"
    DEPENDS ${main_abc} ${demo_name}
  )
endfunction()


function(add_ani_demo demo_name idl_files taihe_configs gen_ets_names user_ets_files user_include_dir user_cpp_files)
  set(MAIN_ABC "${CMAKE_CURRENT_BINARY_DIR}/main.abc")

  # 生成代码
  generate_code_from_idl(${demo_name} "${idl_files}" "${gen_ets_names}" "${taihe_configs}" GEN_INCLUDE_DIR GEN_ABI_C_FILES GEN_ANI_CPP_FILES GEN_ETS_FILES)

  # 动态库编译
  compile_dylib(${demo_name} "${user_include_dir}" "${user_cpp_files}" "${GEN_INCLUDE_DIR}" "${GEN_ABI_C_FILES}" "${GEN_ANI_CPP_FILES}")

  # 链接为 main.abc
  set(ALL_ETS_FILES ${user_ets_files} ${GEN_ETS_FILES})
  abc_link(${demo_name} ${MAIN_ABC} "${ALL_ETS_FILES}")

  # 运行
  run_abc(${demo_name} ${MAIN_ABC})
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

add_subdirectory(test)
#add_subdirectory(cookbook)
