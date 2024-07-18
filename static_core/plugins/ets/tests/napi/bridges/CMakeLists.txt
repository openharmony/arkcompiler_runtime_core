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

set (ETS_NAPI_LIB_TESTS "${CMAKE_CURRENT_BINARY_DIR}/lib")
file(MAKE_DIRECTORY ${ETS_NAPI_LIB_TESTS})

panda_add_library(EtsNapiTests SHARED ${CMAKE_CURRENT_SOURCE_DIR}/lib/EtsNapiTests.cpp)

panda_target_include_directories(EtsNapiTests PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/napi)
panda_target_compile_options(EtsNapiTests PRIVATE -Wno-attributes)

set_target_properties(EtsNapiTests PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${ETS_NAPI_LIB_TESTS})

add_custom_target(ets_napi_tests COMMENT "Ets Napi tests")
add_custom_target(ets_napi_tests_pre 
    COMMAND "rm" "-f" "${CMAKE_CURRENT_BINARY_DIR}/*.pa.abc")

set (ETS_NAPI_TEST_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/test.sh")

add_custom_target(ets_napi_tests_booleans
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-01.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-02.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-03.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-04.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-05.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-06.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-07.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-08.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-09.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-10.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-boolean-11.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}  
)

add_custom_target(ets_napi_tests_bytes 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-01.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-02.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-03.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-04.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-05.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-06.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-07.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-08.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-09.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-10.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-byte-11.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}  
)

add_custom_target(ets_napi_tests_chars 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-01.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-02.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-03.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-04.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-05.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-06.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-07.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-08.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-09.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-10.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-char-11.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}  
)

add_custom_target(ets_napi_tests_doubles 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-01.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-02.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-03.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-04.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-05.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-06.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-07.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-08.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-09.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-10.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-double-11.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}  
)

add_custom_target(ets_napi_tests_floats 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-01.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-02.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-03.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-04.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-05.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-06.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-07.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-08.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-09.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-10.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-float-11.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}  
)

add_custom_target(ets_napi_tests_ints
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-01.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-02.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-03.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-04.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-05.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-06.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-07.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-08.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-09.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-10.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-int-11.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}  
)

add_custom_target(ets_napi_tests_longs
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-01.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-02.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-03.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-04.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-05.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-06.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-07.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-08.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-09.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-10.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-long-11.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}  
)

add_custom_target(ets_napi_tests_shorts
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-01.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-02.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-03.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-04.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-05.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-06.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-07.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-08.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-09.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-10.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-short-11.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}  
)

add_custom_target(ets_napi_tests_returns
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "return-boolean.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "return-byte.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "return-char.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "return-double.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "return-float.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "return-int.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "return-short.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "return-string.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "return-long.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
)

# TODO(a.kropacheva) enable tests when SEGV catch will be implemented
#add_custom_target(ets_napi_tests_segv)

add_custom_target(ets_napi_tests_strings
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-01.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-02.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-03.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-04.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-05.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-06.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-07.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-08.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-09.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-10.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "types-string-11.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}  
)

add_custom_target(ets_napi_tests_arrays
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "arrays-boolean.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "arrays-byte.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "arrays-char.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "arrays-double.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "arrays-float.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "arrays-int.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "arrays-short.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "arrays-string.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX}
    COMMAND "${ETS_NAPI_TEST_SCRIPT}" "arrays-long.pa" "${PANDA_BINARY_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}" ${PANDA_RUN_PREFIX} 
)

add_dependencies(ets_napi_tests_booleans ets_napi_tests_pre)
add_dependencies(ets_napi_tests_bytes ets_napi_tests_pre)
add_dependencies(ets_napi_tests_chars ets_napi_tests_pre)
add_dependencies(ets_napi_tests_doubles ets_napi_tests_pre)
add_dependencies(ets_napi_tests_floats ets_napi_tests_pre)
add_dependencies(ets_napi_tests_ints ets_napi_tests_pre)
add_dependencies(ets_napi_tests_longs ets_napi_tests_pre)
add_dependencies(ets_napi_tests_shorts ets_napi_tests_pre)
add_dependencies(ets_napi_tests_strings ets_napi_tests_pre)
add_dependencies(ets_napi_tests_returns ets_napi_tests_pre)
add_dependencies(ets_napi_tests_arrays ets_napi_tests_pre)
add_dependencies(ets_napi_tests
                ets_napi_tests_booleans
                ets_napi_tests_bytes
                ets_napi_tests_chars
                ets_napi_tests_doubles
                ets_napi_tests_floats
                ets_napi_tests_ints
                ets_napi_tests_longs
                ets_napi_tests_shorts
                ets_napi_tests_returns
                ets_napi_tests_strings
                ets_napi_tests_arrays
)

add_dependencies(ets_napi_tests_pre
    ark
    ark_asm
    EtsNapiTests
    etsstdlib
)

add_dependencies(ets_tests ets_napi_tests)
