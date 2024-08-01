# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

if (PANDA_TARGET_ARM32 OR PANDA_QEMU_BUILD)
    return()
endif()

# Build ETSNAPI library
set(SAMPLER_GEN_ETSNAPI_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}")
file(MAKE_DIRECTORY ${SAMPLER_GEN_ETSNAPI_LIB_DIR})

panda_add_library(sampler_etsnapi_lib SHARED
    SamplerNapiTest.cpp
)
set_target_properties(sampler_etsnapi_lib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${SAMPLER_GEN_ETSNAPI_LIB_DIR})
panda_target_link_libraries(sampler_etsnapi_lib arkruntime)
panda_target_include_directories(sampler_etsnapi_lib PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/napi)

add_custom_target(sampler_etsnapi_test_suite COMMENT "Running sampler etsnapi tests")

if(CMAKE_CROSSCOMPILING)
    ExternalProject_Get_Property(panda_host_tools binary_dir)
    set(aspt_converter_target panda_host_tools-build)
    set(aspt_converter_bin "${binary_dir}/tools/sampler/aspt_converter")
else()
    set(aspt_converter_target aspt_converter)
    set(aspt_converter_bin  $<TARGET_FILE:${aspt_converter_target}>)
endif()

set(ETS_COMPILE_ARGUMENTS --gen-stdlib=false --extension=sts --opt-level=0)

set(RUNTIME_ARGUMENTS
    --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
    --load-runtimes=ets
    --sampling-profiler-enable
    --sampling-profiler-interval=200
)

set(COMPILER_ARGUMENTS_LIST
    "false"
    "true"
)

set(INTERPRETER_ARGUMENTS_LIST
    "cpp"
    "irtoc"
)

set(testing_dir "${CMAKE_CURRENT_BINARY_DIR}")

# Compile Ets tests
add_custom_target(sampler_etsnapi_es2panda
    COMMAND ${es2panda_bin} ${ETS_COMPILE_ARGUMENTS} --output ${testing_dir}/SamplerNapiTest.abc ${CMAKE_CURRENT_SOURCE_DIR}/SamplerNapiTest.sts
    WORKING_DIRECTORY "${testing_dir}"
    DEPENDS ${es2panda_target}
)
add_dependencies(sampler_etsnapi_es2panda sampler_etsnapi_lib)

set(ETS_ENTRY_POINT "main")
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "FastVerify")
    set(ETS_ENTRY_POINT "release_main")
endif()

# NOTE(kurnevichstanislav): fix SIGSEGV in sampler tests with TaskManager (#14278)
set(RUNTIME_ARGUMENTS ${RUNTIME_ARGUMENTS} --workers-type=threadpool)

foreach(c_arg ${COMPILER_ARGUMENTS_LIST})
    foreach(i_arg ${INTERPRETER_ARGUMENTS_LIST})
        # Run tests
        add_custom_target(sampler_etsnapi_run_${c_arg}_${i_arg}
            COMMAND LD_LIBRARY_PATH=${SAMPLER_GEN_ETSNAPI_LIB_DIR} ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} --compiler-enable-jit=${c_arg} --interpreter-type=${i_arg} --sampling-profiler-output-file=${testing_dir}/stack_output_${c_arg}_${i_arg}.aspt ${testing_dir}/SamplerNapiTest.abc ETSGLOBAL::${ETS_ENTRY_POINT}
            WORKING_DIRECTORY "${SAMPLER_GEN_ETSNAPI_LIB_DIR}"
            DEPENDS ark etsstdlib sampler_etsnapi_es2panda
        )

        add_custom_target(sampler_etsnapi_run_aspt_convertor_${c_arg}_${i_arg}
            COMMAND ${aspt_converter_bin} --input=${testing_dir}/stack_output_${c_arg}_${i_arg}.aspt --output=${testing_dir}/traceout_${c_arg}_${i_arg}.csv --cold-graph-enable=true
            WORKING_DIRECTORY "${SAMPLER_GEN_ETSNAPI_LIB_DIR}"
            DEPENDS sampler_etsnapi_run_${c_arg}_${i_arg} ${aspt_converter_target}
        )

        add_custom_target(sampler_etsnapi_trace_check_${c_arg}_${i_arg}
            COMMAND python3 ${PANDA_ETS_PLUGIN_SOURCE}/tests/napi/sampler/sampler_napi_traces_check.py --file=${testing_dir}/traceout_${c_arg}_${i_arg}.csv
            WORKING_DIRECTORY "${SAMPLER_GEN_ETSNAPI_LIB_DIR}"
            DEPENDS sampler_etsnapi_run_aspt_convertor_${c_arg}_${i_arg}
        )

        add_dependencies(sampler_etsnapi_test_suite sampler_etsnapi_trace_check_${c_arg}_${i_arg})
    endforeach()
endforeach()

# NOTE: Add tests only if sanitizers are disabled to avoid conflicts with sanitizers' signal stack
if (PANDA_USE_CUSTOM_SIGNAL_STACK)
    add_dependencies(ets_tests sampler_etsnapi_test_suite)
endif()
