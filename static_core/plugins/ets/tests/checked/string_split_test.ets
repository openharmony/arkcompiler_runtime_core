/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

//! CHECKER      AOT test check no deoptimization occurs in String.split
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-inlining=true"
//! EVENT_NOT    /Deoptimization,string_split_test.ETSGLOBAL::main,BOUNDS_CHECK/
//! METHOD       "string_split_test.ETSGLOBAL::main"

//! CHECKER      JIT test check no deoptimization occurs in String.split
//! RUN          force_jit: true, options: "--compiler-inlining=false", entry: "string_split_test.ETSGLOBAL::main"
//! EVENT_NOT    /Deoptimization,string_split_test.ETSGLOBAL::main,BOUNDS_CHECK/
//! METHOD       "string_split_test.ETSGLOBAL::main"

function main(): void
{
    let str = "It can't be - there must be a gap in between!";
    let arr: String[] = str.split(" ", -5);
    assertEQ(arr.length, 11, "Length must be 11");
    assertTrue(arr[0].equals("It"), "First substring must be `It`");
    assertTrue(arr[1].equals("can't"), "Second substring must be `can't`");
    assertTrue(arr[2].equals("be"), "Third substring must be `be`");
    assertTrue(arr[3].equals("-"), "Third substring must be `-`");
    assertTrue(arr[4].equals("there"), "Third substring must be `there`");
    assertTrue(arr[5].equals("must"), "Third substring must be `must`");
    assertTrue(arr[6].equals("be"), "Third substring must be `be`");
    assertTrue(arr[7].equals("a"), "Third substring must be `a`");
    assertTrue(arr[8].equals("gap"), "Third substring must be `gap`");
    arr = str.split(" ", 3);
    assertEQ(arr.length, 3, "Length must be 3");
}
