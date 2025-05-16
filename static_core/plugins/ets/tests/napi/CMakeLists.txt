# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

add_subdirectory(bridges)
add_subdirectory(etsnative)

if (PANDA_TARGET_ARM32)
    return()
endif()

# Testin pipeline:
# 1. Build ETSNAPI library (one for all tests)
# 2. Compile arkts tests
# 3. Run tests

# Save Ets tests to list
set(ets_tests_list
    #"InnerNativeTest"
    "FakeNativeTest"
    #"InterfaceNativeTest"
    "EtsnapiNameTest"
    "EtsnapiVersionHookTest"
)

# Build ETSNAPI library
set(GEN_ETSNAPI_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")
file(MAKE_DIRECTORY ${GEN_ETSNAPI_LIB_DIR})

panda_add_library(etsnapi_lib SHARED
    #lib/InnerNativeTest.cpp
    #lib/InterfaceNativeTest.cpp
    lib/EtsnapiNameTest.cpp
)
set_target_properties(etsnapi_lib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${GEN_ETSNAPI_LIB_DIR})
panda_target_link_libraries(etsnapi_lib arkruntime)
panda_add_sanitizers(TARGET etsnapi_lib SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_target_include_directories(etsnapi_lib PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/napi ${PANDA_ROOT})

panda_add_library(etsnapi_lib_new SHARED
    lib/EtsnapiVersionHookTest.cpp
)

set_target_properties(etsnapi_lib_new PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${GEN_ETSNAPI_LIB_DIR})
panda_target_include_directories(etsnapi_lib_new PRIVATE ${GEN_ETSNAPI_HEADERS_DIR})
panda_target_link_libraries(etsnapi_lib_new arkruntime)
panda_add_sanitizers(TARGET etsnapi_lib_new SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_target_include_directories(etsnapi_lib PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/napi)

add_custom_target(etsnapi_test_suite COMMENT "Running etsnapi tests")

set(ETS_COMPILE_ARGUMENTS --gen-stdlib=false --extension=ets --opt-level=0)

set(BOOT_PANDA_FILES ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc)

set(RUNTIME_ARGUMENTS
    --boot-panda-files=${BOOT_PANDA_FILES}
    --load-runtimes=ets
    --compiler-enable-jit=false
)

foreach(t ${ets_tests_list})
    set(testing_dir "${CMAKE_CURRENT_BINARY_DIR}/${t}")
    file (MAKE_DIRECTORY ${testing_dir})

    # Compile Ets tests
    add_custom_target(es2panda_${t}
        COMMAND ${es2panda_bin} ${ETS_COMPILE_ARGUMENTS} --output ${testing_dir}/${t}.abc ${CMAKE_CURRENT_SOURCE_DIR}/ets/${t}.ets
        WORKING_DIRECTORY "${testing_dir}"
        DEPENDS ${es2panda_target}
    )
    if (${t} STREQUAL "EtsnapiVersionHookTest")
      add_dependencies(es2panda_${t} etsnapi_lib_new)
    else()
      add_dependencies(es2panda_${t} etsnapi_lib)
    endif()

    # Run tests
    add_custom_target(etsnapi_run_${t}
        COMMAND LD_LIBRARY_PATH=${GEN_ETSNAPI_LIB_DIR} ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} ${testing_dir}/${t}.abc ${t}.ETSGLOBAL::main
        COMMAND LD_LIBRARY_PATH=${GEN_ETSNAPI_LIB_DIR} ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} --gc-type=stw ${testing_dir}/${t}.abc ${t}.ETSGLOBAL::main
        WORKING_DIRECTORY "${GEN_ETSNAPI_LIB_DIR}"
        DEPENDS ark etsstdlib es2panda_${t}
    )
    add_dependencies(etsnapi_run_${t} es2panda_${t})
    add_dependencies(etsnapi_test_suite etsnapi_run_${t})
endforeach()

add_dependencies(ets_tests etsnapi_test_suite)
