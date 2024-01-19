/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
//! RUN           force_jit: true, options: "", entry: "ETSGLOBAL::main"
//! METHOD        "ETSGLOBAL::Compare_more__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_less__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_more_eq__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_less_eq__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_eq__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_ne__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"

//! CHECKER       Compare double AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: ""
//! METHOD        "ETSGLOBAL::Compare_more__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_less__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_more_eq__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_less_eq__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_eq__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! METHOD        "ETSGLOBAL::Compare_ne__noinline__"
//! PASS_BEFORE   "Peephole"
//! INST          "Fcmp"
//! PASS_AFTER    "Lowering"
//! INST_NOT      "Fcmp"
//! RUN           entry: "ETSGLOBAL::main"

function Compare_more__noinline__(str1 : String, str2 : String): boolean {
    return str1.length < str2.length
}

function Compare_less__noinline__(str1 : String, str2 : String): boolean {
    return str1.length > str2.length
}

function Compare_more_eq__noinline__(str1 : String, str2 : String): boolean {
    return str1.length <= str2.length
}

function Compare_less_eq__noinline__(str1 : String, str2 : String): boolean {
    return str1.length >= str2.length
}

function Compare_eq__noinline__(str1 : String, str2 : String): boolean {
    return str1.length == str2.length
}

function Compare_ne__noinline__(str1 : String, str2 : String): boolean {
    return str1.length != str2.length
}

function main(): int {
    if (Compare_more__noinline__("abcde", "abcd")) {
        return 1
    }
    if (!Compare_more__noinline__("abcd", "abcde")) {
        return 2
    }
    if (Compare_more__noinline__("abcd", "abcd")) {
        return 3
    }

    if (!Compare_less__noinline__("abcde", "abcd")) {
        return 4
    }
    if (Compare_less__noinline__("abcd", "abcde")) {
        return 5
    }
    if (Compare_less__noinline__("abcd", "abcd")) {
        return 6
    }

    if (Compare_more_eq__noinline__("abcde", "abcd")) {
        return 7
    }
    if (!Compare_more_eq__noinline__("abcd", "abcde")) {
        return 8
    }
    if (!Compare_more_eq__noinline__("abcd", "abcd")) {
        return 9
    }

    if (!Compare_less_eq__noinline__("abcde", "abcd")) {
        return 10
    }
    if (Compare_less_eq__noinline__("abcd", "abcde")) {
        return 11
    }
    if (!Compare_less_eq__noinline__("abcd", "abcd")) {
        return 12
    }

    if (Compare_eq__noinline__("abcde", "abcd")) {
        return 13
    }
    if (Compare_eq__noinline__("abcd", "abcde")) {
        return 14
    }
    if (!Compare_eq__noinline__("abcd", "abcd")) {
        return 15
    }

    if (!Compare_ne__noinline__("abcde", "abcd")) {
        return 16
    }
    if (!Compare_ne__noinline__("abcd", "abcde")) {
        return 17
    }
    if (Compare_ne__noinline__("abcd", "abcd")) {
        return 18
    }
    return 0
}
