/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

//! CHECKER      If Conversion with Bitfield Extration JIT
//! SKIP_IF      @architecture != "arm64"
//! RUN          force_jit: true, options: "--compiler-loop-unroll=false", entry: "if_conversion_ubfx.ETSGLOBAL::main"
//! EVENT        /Compilation,if_conversion_ubfx.ETSGLOBAL::main,.*,COMPILED/
//! EVENT        /Compilation,if_conversion_ubfx.ETSGLOBAL::__noinline__foo1,.*,COMPILED/
//! EVENT        /Compilation,if_conversion_ubfx.ETSGLOBAL::__noinline__foo2,.*,COMPILED/
//! EVENT        /Compilation,if_conversion_ubfx.ETSGLOBAL::__noinline__foo3,.*,COMPILED/
//! METHOD       "if_conversion_ubfx.ETSGLOBAL::__noinline__foo1"
//! PASS_AFTER   "IfConversion"
//! INST         "ExtractBitfield"
//! METHOD       "if_conversion_ubfx.ETSGLOBAL::__noinline__foo2"
//! PASS_AFTER   "IfConversion"
//! INST         "ExtractBitfield"
//! METHOD       "if_conversion_ubfx.ETSGLOBAL::__noinline__foo3"
//! PASS_AFTER   "IfConversion"
//! INST         "ExtractBitfield"

function __noinline__foo1(x: int, y: int) {
    if ((y & 1) != 0) {
        x += 1;
    }
    return x;
}

function __noinline__foo2(x: int, y: int) {
    if ((y & 2) != 0) {
        x += 1;
    }
    return x;
}

function __noinline__foo3(x: int, y: int) {
    if ((y & 4) != 0) {
        x += 1;
    }
    return x;
}

function __noinline__foo_without_ubfx(x: int, y: int) {
    if ((y & 3) != 0) {
        x += 1;
    }
    if ((y & 6) != 0) {
        x += 1;
    }
    if ((y & 8) != 0) {
        x += 3;
    }
    return x;
}

function main(): void {
    const x = 42;

    let y = 0;
    arktest.assertEQ(__noinline__foo1(x, y), 42);
    arktest.assertEQ(__noinline__foo2(x, y), 42);
    arktest.assertEQ(__noinline__foo3(x, y), 42);
    arktest.assertEQ(__noinline__foo_without_ubfx(x, y), 42);

    y = 1;
    arktest.assertEQ(__noinline__foo1(x, y), 43);
    arktest.assertEQ(__noinline__foo2(x, y), 42);
    arktest.assertEQ(__noinline__foo3(x, y), 42);
    arktest.assertEQ(__noinline__foo_without_ubfx(x, y), 43);

    y = 2;
    arktest.assertEQ(__noinline__foo1(x, y), 42);
    arktest.assertEQ(__noinline__foo2(x, y), 43);
    arktest.assertEQ(__noinline__foo3(x, y), 42);
    arktest.assertEQ(__noinline__foo_without_ubfx(x, y), 44);

    y = 3;
    arktest.assertEQ(__noinline__foo1(x, y), 43);
    arktest.assertEQ(__noinline__foo2(x, y), 43);
    arktest.assertEQ(__noinline__foo3(x, y), 42);
    arktest.assertEQ(__noinline__foo_without_ubfx(x, y), 44);

    y = 4;
    arktest.assertEQ(__noinline__foo1(x, y), 42);
    arktest.assertEQ(__noinline__foo2(x, y), 42);
    arktest.assertEQ(__noinline__foo3(x, y), 43);
    arktest.assertEQ(__noinline__foo_without_ubfx(x, y), 43);

    y = 5;
    arktest.assertEQ(__noinline__foo1(x, y), 43);
    arktest.assertEQ(__noinline__foo2(x, y), 42);
    arktest.assertEQ(__noinline__foo3(x, y), 43);
    arktest.assertEQ(__noinline__foo_without_ubfx(x, y), 44);

    y = 6;
    arktest.assertEQ(__noinline__foo1(x, y), 42);
    arktest.assertEQ(__noinline__foo2(x, y), 43);
    arktest.assertEQ(__noinline__foo3(x, y), 43);
    arktest.assertEQ(__noinline__foo_without_ubfx(x, y), 44);

    y = 7;
    arktest.assertEQ(__noinline__foo1(x, y), 43);
    arktest.assertEQ(__noinline__foo2(x, y), 43);
    arktest.assertEQ(__noinline__foo3(x, y), 43);
    arktest.assertEQ(__noinline__foo_without_ubfx(x, y), 44);

    y = 8;
    arktest.assertEQ(__noinline__foo1(x, y), 42);
    arktest.assertEQ(__noinline__foo2(x, y), 42);
    arktest.assertEQ(__noinline__foo3(x, y), 42);
    arktest.assertEQ(__noinline__foo_without_ubfx(x, y), 45);
}
