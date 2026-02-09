/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! CHECKER       Compare double JIT
//! RUN           force_jit: true, options: "--compiler-regex='.*ETSGLOBAL::compare.*'",
//!               entry: "ets_double_compare.ETSGLOBAL::main"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareMore__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareLess__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareMoreEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareLessEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareNe__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstMore__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstLess__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstMoreEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstLessEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstNe__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"

//! CHECKER       Compare double AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: ""
//! METHOD        "ets_double_compare.ETSGLOBAL::compareMore__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareLess__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareMoreEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareLessEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareNe__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstMore__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstLess__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstMoreEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstLessEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstEq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::compareConstNe__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! RUN           entry: "ets_double_compare.ETSGLOBAL::main"

function compareMore__noinline__(str1 : String, str2 : String): boolean {
    return str1.length < str2.length
}

function compareLess__noinline__(str1 : String, str2 : String): boolean {
    return str1.length > str2.length
}

function compareMoreEq__noinline__(str1 : String, str2 : String): boolean {
    return str1.length <= str2.length
}

function compareLessEq__noinline__(str1 : String, str2 : String): boolean {
    return str1.length >= str2.length
}

function compareEq__noinline__(str1 : String, str2 : String): boolean {
    return str1.length == str2.length
}

function compareNe__noinline__(str1 : String, str2 : String): boolean {
    return str1.length != str2.length
}

function compareConstMore__noinline__(str : String) : boolean {
    if (str.length < 4.0) {
        return str.length < 2147483647.0;
    }
    return false;
}

function compareConstLess__noinline__(str : String) : boolean {
    if (str.length > 4.0) {
        return str.length > -2147483648.0;
    }
    return false;
}

function compareConstMoreEq__noinline__(str : String) : boolean {
    if (str.length <= 4.0) {
        return str.length <= 2147483648.0;
    }
    return false;
}

function compareConstLessEq__noinline__(str : String) : boolean {
    if (str.length >= 4.0) {
        return str.length >= -2147483649.0;
    }
    return false;
}

function compareConstEq__noinline__(str : String) : boolean {
    return str.length == 4.0;
}

function compareConstNe__noinline__(str : String) : boolean {
    return str.length != 4.0;
}

function main(): int {
    if (compareMore__noinline__('abcde', 'abcd')) {
        return 1
    }
    if (!compareMore__noinline__('abcd', 'abcde')) {
        return 2
    }
    if (compareMore__noinline__('abcd', 'abcd')) {
        return 3
    }

    if (!compareLess__noinline__('abcde', 'abcd')) {
        return 4
    }
    if (compareLess__noinline__('abcd', 'abcde')) {
        return 5
    }
    if (compareLess__noinline__('abcd', 'abcd')) {
        return 6
    }

    if (compareMoreEq__noinline__('abcde', 'abcd')) {
        return 7
    }
    if (!compareMoreEq__noinline__('abcd', 'abcde')) {
        return 8
    }
    if (!compareMoreEq__noinline__('abcd', 'abcd')) {
        return 9
    }

    if (!compareLessEq__noinline__('abcde', 'abcd')) {
        return 10
    }
    if (compareLessEq__noinline__('abcd', 'abcde')) {
        return 11
    }
    if (!compareLessEq__noinline__('abcd', 'abcd')) {
        return 12
    }

    if (compareEq__noinline__('abcde', 'abcd')) {
        return 13
    }
    if (compareEq__noinline__('abcd', 'abcde')) {
        return 14
    }
    if (!compareEq__noinline__('abcd', 'abcd')) {
        return 15
    }

    if (!compareNe__noinline__('abcde', 'abcd')) {
        return 16
    }
    if (!compareNe__noinline__('abcd', 'abcde')) {
        return 17
    }
    if (compareNe__noinline__('abcd', 'abcd')) {
        return 18
    }

    if (!compareConstMore__noinline__('abc')) {
        return 19
    }
    if (compareConstMore__noinline__('abcd')) {
        return 20
    }
    if (compareConstMore__noinline__('abcde')) {
        return 21
    }

    if (compareConstLess__noinline__('abc')) {
        return 22
    }
    if (compareConstLess__noinline__('abcd')) {
        return 23
    }
    if (!compareConstLess__noinline__('abcde')) {
        return 24
    }

    if (!compareConstMoreEq__noinline__('abc')) {
        return 25
    }
    if (!compareConstMoreEq__noinline__('abcd')) {
        return 26
    }
    if (compareConstMoreEq__noinline__('abcde')) {
        return 27
    }

    if (compareConstLessEq__noinline__('abc')) {
        return 28
    }
    if (!compareConstLessEq__noinline__('abcd')) {
        return 29
    }
    if (!compareConstLessEq__noinline__('abcde')) {
        return 30
    }

    if (compareConstEq__noinline__('abc')) {
        return 31
    }
    if (!compareConstEq__noinline__('abcd')) {
        return 32
    }
    if (compareConstEq__noinline__('abcde')) {
        return 33
    }

    if (!compareConstNe__noinline__('abc')) {
        return 34
    }
    if (compareConstNe__noinline__('abcd')) {
        return 35
    }
    if (!compareConstNe__noinline__('abcde')) {
        return 36
    }
    return 0
}
