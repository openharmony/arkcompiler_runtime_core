/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

//! CHECKER         No virtual call to Box.set,get
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*main.*'"
//! METHOD          "lambda_final_box.ETSGLOBAL::main"
//! PASS_AFTER      "IrBuilder"
//! INST_NOT        /CallVirtual.*std.core.Box/
//! INST_NOT        /CallVirtual.*std.core.BooleanBox/
//! INST_NOT        /CallVirtual.*std.core.ByteBox/
//! INST_NOT        /CallVirtual.*std.core.ShortBox/
//! INST_NOT        /CallVirtual.*std.core.CharBox/
//! INST_NOT        /CallVirtual.*std.core.IntBox/
//! INST_NOT        /CallVirtual.*std.core.LongBox/
//! INST_NOT        /CallVirtual.*std.core.FloatBox/
//! INST_NOT        /CallVirtual.*std.core.DoubleBox/
//! INST            /CallStatic.*std.core.Box/
//! INST            /CallStatic.*std.core.BooleanBox/
//! INST            /CallStatic.*std.core.ByteBox/
//! INST            /CallStatic.*std.core.ShortBox/
//! INST            /CallStatic.*std.core.CharBox/
//! INST            /CallStatic.*std.core.IntBox/
//! INST            /CallStatic.*std.core.LongBox/
//! INST            /CallStatic.*std.core.FloatBox/
//! INST            /CallStatic.*std.core.DoubleBox/

function main(): int {
    let cBoolean: boolean = true;
    let cByte: byte = 1 as byte;
    let cChar: char = c'a';
    let cShort: short = 123;
    let cInt: int = 123000;
    let cLong: long = 123000000000;
    let cFloat: float = 1.0f;
    let cDouble: double = 1.0;
    let cObject = "abc";
    cBoolean = false;
    cByte = 2;
    cChar = c'b';
    cShort++;
    cInt++;
    cLong++;
    cFloat++;
    cDouble++;
    cObject = "def";
    (() => {
        cBoolean = false;
        cByte = 2;
        cChar = c'b';
        cShort++;
        cInt++;
        cLong++;
        cFloat++;
        cDouble++;
        cObject = "def";
    })();
    return 0;
}
