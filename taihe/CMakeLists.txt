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
#     # 添加编译和链接时的ASan选项
#     add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
#     add_link_options(-fsanitize=address)

#     add_link_options(
#         -fsanitize=address
#     )
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address ")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address ")
# endif()


# 启用调试信息
set(CMAKE_BUILD_TYPE Debug)

# 为 C 编译器设置 -fPIC
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# 为 C++ 编译器设置 -fPIC
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# taihe runtime .c/.cpp path
set(TH_RUNTIME_DIR "${CMAKE_SOURCE_DIR}/runtime")
set(TH_RUNTIME_INCLUDE_DIR "${TH_RUNTIME_DIR}/include")
set(TH_RUNTIME_SOURCE_DIR "${TH_RUNTIME_DIR}/src")


# 设置 PANDA 路径
set(PANDA_SDK_FILENAME "sdk-1.5.0-dev.26258.tgz")
set(PANDA_SDK_URL "https://nexus.bz-openlab.ru:10443/repository/koala-npm/%40panda/sdk/-/${PANDA_SDK_FILENAME}")
set(PANDA_EXTRACT_DIR "${CMAKE_SOURCE_DIR}/.panda_vm")
set(PANDA_HOME "${PANDA_EXTRACT_DIR}/package/linux_host_tools")
set(PANDA_SDK_PATH "${PANDA_EXTRACT_DIR}/${PANDA_SDK_FILENAME}")

# 设置 Python
set(Python3_EXECUTABLE "/usr/bin/python3")
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# 启用或禁用coverage
option(ENABLE_COVERAGE "Enable coverage run for the Python command" OFF)

# 编译 taihe runtime 静态库
set(TH_SOURCE
  "${TH_RUNTIME_SOURCE_DIR}/string.c"
  "${TH_RUNTIME_SOURCE_DIR}/object.c"
  "${TH_RUNTIME_SOURCE_DIR}/runtime.cpp"
)

add_library(th_runtime STATIC ${TH_SOURCE})

set_target_properties(th_runtime PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

target_include_directories(th_runtime PUBLIC
  ${TH_RUNTIME_INCLUDE_DIR})

# 准备 panda sdk
function(setup_panda_sdk)

  file(MAKE_DIRECTORY "${PANDA_EXTRACT_DIR}")

  # Download Panda SDK
  if(NOT EXISTS "${PANDA_SDK_PATH}")
    message(STATUS "Downloading Panda SDK...")
    execute_process(
      COMMAND curl -u koala-pub:y3t!n0therP -L -o "${PANDA_SDK_PATH}" "${PANDA_SDK_URL}"
      RESULT_VARIABLE DOWNLOAD_RESULT
    )
    if(NOT DOWNLOAD_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to download Panda SDK! Please check the URL or credentials.")
    endif()
  endif()

  # Extract Panda SDK
  if(NOT EXISTS "${PANDA_HOME}")
    message(STATUS "Extracting Panda SDK...")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xzf "${PANDA_SDK_PATH}"
      WORKING_DIRECTORY "${PANDA_EXTRACT_DIR}"
      RESULT_VARIABLE EXTRACT_RESULT
    )
    if(NOT EXTRACT_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to extract Panda SDK!")
    endif()
  endif()

  # Set environment variable
  set(ENV{PANDA_HOME} "${PANDA_HOME}")

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


# 生成 arktsconfig.json
function(write_arkts_config config_path gen_dir sys_dir)
  file(WRITE "${config_path}"
    "{\n"
    "  \"compilerOptions\": {\n"
    "    \"baseUrl\": \"${PANDA_HOME}\",\n"
    "    \"paths\": {\n"
    "      \"std\": [\"${PANDA_HOME}/../ets/stdlib/std\"],\n"
    "      \"escompat\": [\"${PANDA_HOME}/../ets/stdlib/escompat\"],\n"
    "      \"@generated\": [\"${gen_dir}\"],\n"
    "      \"@system\": [\"${sys_dir}\"]\n"
    "    }\n"
    "  }\n"
    "}\n"
  )
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


function(generate_code_from_idl this_target_name idl_files gen_ets_files idl_dir generated_dir)
  set(IDL_FILES ${idl_files})
  set(GEN_ETS_FILES ${gen_ets_files})

  set(PROJ_HPP_FILES)
  set(IMPL_HPP_FILES)
  set(ABI_H_FILES)
  set(ABI_C_FILES)
  set(ANI_HPP_FILES)
  set(ANI_CPP_FILES)
  set(ANI_CTOR_FILE "${generated_dir}/src/ani_constructor.cpp")

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

  # 如果启用覆盖率，使用 coverage run
  if(ENABLE_COVERAGE)
    list(APPEND COMMAND_TO_RUN "coverage" "run" "--parallel-mode" "-m" "taihe.cli.compiler" "-I" "${idl_dir}/idl" "-O" "${generated_dir}" "--ani" "--author" "--sts-keep-name")
  else()
    list(APPEND COMMAND_TO_RUN "python" "-m" "taihe.cli.compiler" "-I" "${idl_dir}/idl" "-O" "${generated_dir}" "--ani" "--author" "--sts-keep-name")
  endif()

  add_custom_command(
    OUTPUT  ${PROJ_HPP_FILES} ${IMPL_HPP_FILES} ${ABI_H_FILES} ${ABI_C_FILES} ${ANI_HPP_FILES} ${ANI_CPP_FILES} ${GEN_ETS_FILES} ${ANI_CTOR_FILE}
    COMMAND ${COMMAND_TO_RUN}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/compiler
    DEPENDS ${IDL_FILES} ${CMAKE_SOURCE_DIR}/compiler/taihe/parse/antlr/TaiheAST.py
    COMMENT "Generating Taihe C++ header and source files..."
    VERBATIM
  )

  add_custom_target(${this_target_name}_gen
    DEPENDS ${PROJ_HPP_FILES}
    ${IMPL_HPP_FILES}
    ${ABI_H_FILES}
    ${ABI_C_FILES}
    ${ANI_HPP_FILES}
    ${ANI_CPP_FILES}
    ${GEN_ETS_FILES}
    ${ANI_CTOR_FILE}
  )

  set(${this_target_name}_PROJ_HPP_FILES ${PROJ_HPP_FILES} PARENT_SCOPE)
  set(${this_target_name}_IMPL_HPP_FILES ${IMPL_HPP_FILES} PARENT_SCOPE)
  set(${this_target_name}_ABI_H_FILES ${ABI_H_FILES} PARENT_SCOPE)
  set(${this_target_name}_ABI_C_FILES ${ABI_C_FILES} PARENT_SCOPE)
  set(${this_target_name}_ANI_HPP_FILES ${ANI_HPP_FILES} PARENT_SCOPE)
  set(${this_target_name}_ANI_CPP_FILES ${ANI_CPP_FILES} PARENT_SCOPE)
  set(${this_target_name}_ANI_CTOR_FILE ${ANI_CTOR_FILE} PARENT_SCOPE)
endfunction()


function(copy_dir this_target_name user_ets_files build_demo_dir)
  set(USER_ETS_FILES ${user_ets_files})
  file(MAKE_DIRECTORY "${build_demo_dir}/user/")
  set(copy_user_files "")
  foreach(user_ets_file ${USER_ETS_FILES})
    get_filename_component(filename ${user_ets_file} NAME)
    add_custom_command(
      OUTPUT ${build_demo_dir}/user/${filename}
      COMMAND ${CMAKE_COMMAND} -E copy
      ${user_ets_file}
      ${build_demo_dir}/user/${filename}
      DEPENDS ${user_ets_file}
    )
    list(APPEND copy_user_files ${build_demo_dir}/user/${filename})
  endforeach()
  add_custom_target(copy_${this_target_name}_user_target ALL
    DEPENDS ${copy_user_files}
  )
  set(${this_target_name}_copy_user_files ${copy_user_files} PARENT_SCOPE)
endfunction()


# 编译动态库
function(compile_dylib target_name target_path user_cpp_files build_demo_dir)
  # 创建输出目录（如果不存在）
  file(MAKE_DIRECTORY ${target_path}/build)
  set(LIBRARY_OUTPUT_PATH ${target_path}/build)

  set(ALL_SRC_FILES ${${this_target_name}_ABI_C_FILES} ${${this_target_name}_ANI_CPP_FILES} ${${this_target_name}_ANI_CTOR_FILE} ${user_cpp_files})

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
  target_include_directories(${target_name} PUBLIC "${build_demo_dir}/author_generated/include/" TH_RUNTIME_INCLUDE_DIR)
endfunction()


# 自定义 ETS 构建规则
function(build_ets_target this_target_name ets_files output_name dep_ets build_demo_dir)
  get_filename_component(prefix_name ${this_target_name} NAME)

  # 设置输出目录
  set(OUTPUT_DIR "${build_demo_dir}/build")

  # 创建输出目录（如果不存在）
  file(MAKE_DIRECTORY ${OUTPUT_DIR})

  add_custom_command(
    OUTPUT ${OUTPUT_DIR}/${output_name}.abc  # 输出文件
    COMMAND ${ETS_COMPILER} ${ets_files}
    --output ${OUTPUT_DIR}/${output_name}.abc
    --extension ets
    --arktsconfig ${build_demo_dir}/arktsconfig.json  # 生成命令
    && ${ETS_DISASM} ${OUTPUT_DIR}/${output_name}.abc  ${OUTPUT_DIR}/${output_name}.abc.dump
    DEPENDS ${ets_files} ${dep_ets} ${build_demo_dir}/arktsconfig.json # 输入文件依赖
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}  # 工作目录
    COMMENT "Building ${this_target_name} ETS target"  # 注释
  )

  # add_custom_command(
  #     OUTPUT  ${OUTPUT_DIR}/${output_name}.abc.dump   # 输出文件
  #     COMMAND ${ETS_DISASM} ${OUTPUT_DIR}/${output_name}.abc  ${OUTPUT_DIR}/${output_name}.abc.dump
  #     DEPENDS ${OUTPUT_DIR}/${output_name}.abc   # 输入文件依赖
  #     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}  # 工作目录
  #     COMMENT "Building ${this_target_name} ETS target DISASM"  # 注释
  # )

endfunction()


# ets 生成 abc
function(ets_to_abc this_target_name ets_list output_abc_list_var demo_dir build_demo_dir dep_ets)
  set(ABC_FILES "")

  # write_arkts_config("${build_demo_dir}/arktsconfig.json" "${build_demo_dir}/author_generated" "${demo_dir}/system")

  add_custom_target(${this_target_name}_${output_abc_list_var}_compile_ets ALL DEPENDS ${ets_list})

  # 为每个 ETS 文件创建编译目标
  foreach(ETS_FILE ${ets_list})
    get_filename_component(ETS_NAME_P ${ETS_FILE} NAME)
    string(REGEX REPLACE "\\.[^.]*$" "" ETS_NAME "${ETS_NAME_P}")

    set(ABC_FILE "${build_demo_dir}/build/${ETS_NAME}.ets.abc")

    build_ets_target(${this_target_name} "${ETS_FILE}" "${ETS_NAME}.ets" "${dep_ets}" ${build_demo_dir})
    list(APPEND ABC_FILES "${ABC_FILE}")

    add_dependencies(${this_target_name}_${output_abc_list_var}_compile_ets ${this_target_name})
  endforeach()

  # 设置输出变量
  set(${output_abc_list_var} ${ABC_FILES} PARENT_SCOPE)
endfunction()


function(abc_link this_target_name all_abc build_demo_dir)
  set(OUTPUT_DIR "${build_demo_dir}")
  file(MAKE_DIRECTORY ${OUTPUT_DIR})

  # 创建输出目标文件
  set(MAIN_ABC "${OUTPUT_DIR}/main.abc")

  set(ETS_ABC_FILES "${all_abc}")

  # 创建链接命令
  add_custom_command(
    OUTPUT ${MAIN_ABC}
    COMMAND ${ETS_LINK} --output ${MAIN_ABC} -- ${ETS_ABC_FILES}
    DEPENDS ${ETS_ABC_FILES}
    COMMENT "Linking all ABC files to main.abc"
  )

  # 创建链接目标
  add_custom_target(${this_target_name}_link_abc ALL DEPENDS ${MAIN_ABC})

  # 依赖关系
  add_dependencies(${this_target_name}_link_abc ${this_target_name}_SYS_ABC_compile_ets ${this_target_name}_USER_ABC_compile_ets ${this_target_name}_GENERATED_ABC_compile_ets ${this_target_name})

  # 导出主 ABC 文件路径
  set(${this_target_name}_MAIN_ABC ${MAIN_ABC} PARENT_SCOPE)
endfunction()

function(run_abc this_target_name build_demo_dir)
  # 设置
  set(OUTPUT_DIR "${build_demo_dir}")
  file(MAKE_DIRECTORY ${OUTPUT_DIR})
  get_filename_component(target_name ${this_target_name} NAME_WE)

  # 创建运行目标，并明确依赖于链接目标
  add_custom_target(${this_target_name}_run ALL
    WORKING_DIRECTORY ${OUTPUT_DIR}
    COMMAND # LD_PRELOAD=/usr/lib/llvm-14/lib/clang/14.0.0/lib/linux/libclang_rt.asan-x86_64.so
    LD_LIBRARY_PATH=${OUTPUT_DIR} ${ETS_RUNTIME}
    --boot-panda-files=$ENV{PANDA_HOME}/../ets/etsstdlib.abc
    --load-runtimes=ets ${OUTPUT_DIR}/main.abc main.ETSGLOBAL::main
    && echo "Run successful" || (echo "Run failed" && exit 1)
    COMMENT "Running ${target_name}"
  )

  # 确保运行在链接之后
  add_dependencies(${this_target_name}_run ${this_target_name}_link_abc)
endfunction()


function(update_grammar)
  add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/compiler/taihe/parse/antlr/TaiheAST.py
    COMMAND ./generate-grammar
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/compiler  # 设置工作目录
    COMMENT "Update grammar"
    DEPENDS ${CMAKE_SOURCE_DIR}/compiler/Taihe.g4
  )

  add_custom_target(UPDATE_GRAMMAR ALL
    DEPENDS ${CMAKE_SOURCE_DIR}/compiler/taihe/parse/antlr/TaiheAST.py
  )
endfunction()


update_grammar()


function(add_ani_demo this_target_name idl_files gen_ets_files sys_ets_files user_ets_files user_cpp_files run_dir)
  set(DEMO_DIR "${CMAKE_SOURCE_DIR}/${run_dir}/${this_target_name}")
  set(BUILD_DEMO_DIR "${CMAKE_BINARY_DIR}/${run_dir}/${this_target_name}")
  set(GEN_DIR "${BUILD_DEMO_DIR}/author_generated")
  set(GEN_ETS_PATHS "")
  foreach(ETS_FILE ${gen_ets_files})
    set(FULL_PATH "${GEN_DIR}/${ETS_FILE}")
    list(APPEND GEN_ETS_PATHS "${FULL_PATH}")
  endforeach()
  set(GEN_SYS_ETS_PATHS "")
  foreach(SYS_ETS_FILE ${sys_ets_files})
    set(SYS_FULL_PATH "${DEMO_DIR}/system/${SYS_ETS_FILE}")
    list(APPEND GEN_SYS_ETS_PATHS "${SYS_FULL_PATH}")
  endforeach()

  # 复制 ets
  # copy_dir(${this_target_name} "${user_ets_files}" ${BUILD_DEMO_DIR})
  # 生成代码
  generate_code_from_idl(${this_target_name} "${idl_files}" "${GEN_ETS_PATHS}" ${DEMO_DIR} ${GEN_DIR})
  # 动态库编译
  compile_dylib(${this_target_name} ${DEMO_DIR} "${user_cpp_files}" ${BUILD_DEMO_DIR})
  # gen arktsconfig.json
  write_arkts_config("${BUILD_DEMO_DIR}/arktsconfig.json" "${BUILD_DEMO_DIR}/author_generated" "${DEMO_DIR}/system")
  # ets -> ets.abc
  ets_to_abc(${this_target_name} "${GEN_SYS_ETS_PATHS}" SYS_ABC ${DEMO_DIR} ${BUILD_DEMO_DIR} "")
  ets_to_abc(${this_target_name} "${GEN_ETS_PATHS}" GENERATED_ABC ${DEMO_DIR} ${BUILD_DEMO_DIR} "${GEN_SYS_ETS_PATHS}")
  ets_to_abc(${this_target_name} "${user_ets_files}" USER_ABC ${DEMO_DIR} ${BUILD_DEMO_DIR} "${GEN_ETS_PATHS}")
  # 收集所有 ets.abc
  set(ALL_ABC_FILES ${SYS_ABC} ${GENERATED_ABC} ${USER_ABC})
  # 链接为 main.abc
  abc_link(${this_target_name} "${ALL_ABC_FILES}" ${BUILD_DEMO_DIR})
  # 运行
  run_abc(${this_target_name} ${BUILD_DEMO_DIR})
endfunction()

add_subdirectory(test)
#add_subdirectory(cookbook)
