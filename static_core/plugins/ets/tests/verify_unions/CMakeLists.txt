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

function(add_ets_verifier_test)
    set(prefix ARG)
    set(noValues VERIFIER_FAIL_TEST)
    set(singleValues FILE)
    cmake_parse_arguments(${prefix}
                          "${noValues}"
                          "${singleValues}"
                          ""
                          ${ARGN})
    cmake_parse_arguments(PARSE_ARGV 3
                          ${prefix}
                          ""
                          "SEARCH_STDERR"
                          "")
    if (ARG_VERIFIER_FAIL_TEST)
        set(VERIFIER_FAIL_TEST VERIFIER_FAIL_TEST)
    else()
        set(VERIFIER_FAIL_TEST)
    endif()

    set(error_file)

    verifier_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/verify_unions/${ARG_FILE}.pa
        TARGET verify_unions_${ARG_FILE}-verify
        ${VERIFIER_FAIL_TEST}
        SEARCH_STDERR "${ARG_SEARCH_STDERR}"
        ERROR_FILE_VARIABLE error_file
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
    )
    add_dependencies(ets_union_asm_verify verify_unions_${ARG_FILE}-verify)

    if (DEFINED ARG_SEARCH_STDERR AND NOT (CMAKE_BUILD_TYPE MATCHES Release))
        add_custom_target(verify_unions_${ARG_FILE}-check-logmsg
                          COMMENT "Check verify_unions_${ARG_FILE} log message"
                          COMMAND grep -zo \"${ARG_SEARCH_STDERR}\" ${error_file} >/dev/null
                          DEPENDS verify_unions_${ARG_FILE}-verify)

        add_dependencies(ets_union_asm_verify verify_unions_${ARG_FILE}-check-logmsg)
    endif()
endfunction()

add_custom_target(ets_union_asm_verify)
add_dependencies(ets_union_asm_verify verifier)
add_dependencies(ets_tests ets_union_asm_verify)

add_ets_verifier_test(FILE "correct_union_arg")
add_ets_verifier_test(FILE "correct_union_arg_2")
add_ets_verifier_test(FILE "correct_union_override")
add_ets_verifier_test(FILE "correct_union_override_2")
add_ets_verifier_test(FILE "neg_union_arg" VERIFIER_FAIL_TEST SEARCH_STDERR "Bad call incompatible parameter")
add_ets_verifier_test(FILE "neg_union_override_multi" VERIFIER_FAIL_TEST SEARCH_STDERR "Cannot link class: Multiple override LE\;foo LD\;foo")
add_ets_verifier_test(FILE "neg_union_arg_redecl" VERIFIER_FAIL_TEST SEARCH_STDERR "Cannot link class: Method is redeclarated LD\;foo LD\;foo")