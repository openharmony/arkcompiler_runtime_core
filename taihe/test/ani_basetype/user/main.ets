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
import {BusinessError} from "@ohos.base";
import * as base_test from "base_test";

loadLibrary("ani_basetype");

function main() {
    const suite = new ArkTestsuite("base_test ut");
    suite.addTest("integer_test", integer_test);
    suite.addTest("float_test", float_test);
    suite.addTest("boolean_test", boolean_test);
    suite.addTest("string_test", string_test);
    exit(suite.run());
}

function integer_test() {
    let a: byte = 2;
    let b: byte = 3;
    assertEQ(base_test.AddI8(a, b), 5);

    let c: short = 2;
    let d: short = 3;
    assertEQ(base_test.SubI16(c, d), -1);

    let e: int = 2;
    let f: int = 3;
    assertEQ(base_test.MulI32(e, f), 6);

    let h: long = 2;
    let i: long = 2;
    assertEQ(base_test.DivI64(h, i), 1);

    expectError(() => {
        let x: long = 2;
        let y: long = 0;
        base_test.DivI64(x, y);
    });
}

function float_test() {
    const eps = 1e-5;

    let a: float = 2.5f;
    let b: float = 3.5f;
    let c: float = base_test.AddF32(a, b);
    assertLE(Math.abs(c - 6.0), eps);

    let d: float = 4.3f;
    let e: float = 3.0f;
    let f: float = base_test.SubF32(d, e);
    assertLE(Math.abs(f - 1.3), eps);

    let h: float = 3.7f;
    let i: double = 5.3;
    let j: double = base_test.AddF64(h, i);
    assertLE(Math.abs(j - 9.0), eps);

    let k: float = 3.5f;
    let l: double = 1.0;
    let m: double = base_test.SubF64(k, l);
    assertLE(Math.abs(m - 2.5), eps);

    let x: float = 2.5f;
    let y: float = 4.0f;
    let z: double = base_test.MulF64(x, y);
    assertLE(Math.abs(z - 10.0), eps);
}

function boolean_test() {
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
}

function string_test() {
    let a: string = "tai";
    let b: string = "he";
    assertEQ(base_test.Concatx(a, b), "taihe");

    let c: string = "taihe";
    assertEQ(base_test.Splitx(c, 3), "tai");

    let d: string = "10";
    assertEQ(base_test.ToI32(d), 10);

    let e: int = 10;
    assertEQ(base_test.FromI32(e), "10");
}
