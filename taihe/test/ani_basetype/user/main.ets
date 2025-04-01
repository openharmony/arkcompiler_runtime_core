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
import * as base_test from "../author_generated/base_test";

loadLibrary("ani_basetype");

function main() {
    console.log("base_test start");
    const suite = new ArkTestsuite("base_test ut")
    suite.addTest("integer_test", integer_test);
    suite.addTest("float_test", float_test);
    suite.addTest("boolean_test", boolean_test);
    suite.addTest("string_test", string_test);
    console.log("base_test end");
}

function integer_test() {
    console.log("run integer_test start ...");
    let a: byte = 2;
    let b: byte = 3;
    assertEQ(base_test.AddI8(a, b), 5);
    console.log(base_test.AddI8(a, b))

    let c: short = 2;
    let d: short = 3;
    assertEQ(base_test.SubI16(c, d), -1);
    console.log(base_test.SubI16(c, d))

    let e: int = 2;
    let f: int = 3;
    assertEQ(base_test.MulI32(e, f), 6);
    console.log(base_test.MulI32(e, f))

    let h: long = 2;
    let i: long = 2;
    assertEQ(base_test.DivI64(h, i), 1);
    console.log(base_test.DivI64(h, i))

    try {
        let x: long = 2;
        let y: long = 0;
        assertEQ(base_test.DivI64(x, y), -1);
    } catch (e) {
        console.error("divide error ...");
    }

    console.log("run integer_test end");
}

function float_test() {
    console.log("run float_test start ...");
    let a: float = 2.5;
    let b: float = 3.5;
    let c: float = base_test.AddF32(a, b);
    assertEQ(c, 6.0);
    console.log(
        "AddF32 result : " +
            `ret = `,
        c);

    let d: float = 4.3;
    let e: float = 3.0;
    let f: float = base_test.SubF32(d, e);
    assertEQ(f >= 1.3, true);
    console.log(
        "SubF32 result : " +
            `ret = `,
        f);

    let h: float = 3.7;
    let i: double = 5.3;
    let j: double = base_test.AddF64(h, i);
    assertEQ(j >= 9.0, true);
    console.log(
        "AddF64 result : " +
            `ret = `,
        j);

    let k: float = 3.5;
    let l: double = 1.0;
    let m: double = base_test.SubF64(k, l);
    assertEQ(m >= 2.5, true);
    console.log(
        "SubF64 result : " +
            `ret = `,
        m);

    let x: float = 2.5;
    let y: float = 4.0;
    let z: double = base_test.MulF64(x, y);
    assertEQ(z, 10.0);
    console.log(
        "MulF64 result : " +
            `ret = `,
        z);
    console.log("run float_test end");
}

function boolean_test() {
    console.log("run boolean_test start ...");
    let a = true;
    let b = true;
    assertEQ(base_test.Check(a, b), true);
    a = true;
    b = false;
    assertEQ(base_test.Check(a, b), false);
    a = false;
    b = true;
    assertEQ(base_test.Check(a, b), false);
    a = false;
    b = false;
    assertEQ(base_test.Check(a, b), false);
    console.log("run boolean_test end");
}

function string_test() {
    console.log("run string_test start ...");
    let a: string = "tai";
    let b: string = "he";
    assertEQ(base_test.Concatx(a, b), "taihe");

    let c: string = "taihe";
    assertEQ(base_test.Splitx(c, 3), "tai");

    let d: string = "10";
    assertEQ(base_test.ToI32(d), 10);

    let e: int = 10;
    assertEQ(base_test.FromI32(e), "10");

    console.log("run string_test end");
}
