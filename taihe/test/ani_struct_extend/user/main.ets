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
import * as StructExtendTest from "struct_extend";

loadLibrary("ani_struct_extend");

function check_struct_A() {
    let ins_a: StructExtendTest.A = {param1: 100};
    StructExtendTest.check_A(ins_a)
    let res_a = StructExtendTest.create_A()
    assertEQ(res_a.param1, 1)
}

function check_struct_B() {
    let ins_b: StructExtendTest.B = {param1: 100, param2: 200};
    StructExtendTest.check_B(ins_b)
    let res_b = StructExtendTest.create_B()
    assertEQ(res_b.param1, 1)
    assertEQ(res_b.param2, 2)
}

class C implements StructExtendTest.C {
    param1: int = 100;
    param2: int = 200;
    param3: int = 300;
}

function check_struct_C() {
    let ins_c: StructExtendTest.C = new C();
    StructExtendTest.check_C(ins_c)
    let res_c = StructExtendTest.create_C()
    assertEQ(res_c.param1, 1)
    assertEQ(res_c.param2, 2)
    assertEQ(res_c.param3, 3)
}

function check_struct_D() {
    let ins_d: StructExtendTest.D = {param4: 400};
    StructExtendTest.check_D(ins_d)
    let res_d = StructExtendTest.create_D()
    assertEQ(res_d.param4, 4)
}

function check_struct_E() {
    let ins_e: StructExtendTest.E = {param4: 400, param5: 500};
    StructExtendTest.check_E(ins_e)
    let res_e = StructExtendTest.create_E()
    assertEQ(res_e.param4, 4)
    assertEQ(res_e.param5, 5)
}

function check_class_Bar() {
    let in_e: StructExtendTest.E = {param4: 4, param5: 5};
    let bar: StructExtendTest.Bar = StructExtendTest.create_Bar(in_e);
    assertEQ(StructExtendTest.check_Bar(bar), true);
    let e: StructExtendTest.E = bar.getE();
    assertEQ(e.param4, 4);
    assertEQ(e.param5, 5);
    let e1: StructExtendTest.E = {param4: 400, param5: 500};
    bar.setE(e1);
    let e2: StructExtendTest.E = bar.getE();
    assertEQ(e2.param4, 400);
    assertEQ(e2.param5, 500);
}

function check_struct_F() {
    let in_e: StructExtendTest.E = {param4: 4, param5: 5};
    let f: StructExtendTest.F = StructExtendTest.create_F(in_e);
    assertEQ(StructExtendTest.check_F(f), true);
    let e: StructExtendTest.E = f.barF.getE();
    assertEQ(e.param4, 4);
    assertEQ(e.param5, 5);
}

function check_struct_G() {
    let in_e: StructExtendTest.E = {param4: 4, param5: 5};
    let g: StructExtendTest.F = StructExtendTest.create_G(in_e);
    assertEQ(StructExtendTest.check_F(g), true);
    let e1: StructExtendTest.E = g.barF.getE();
    assertEQ(e1.param4, 4);
    assertEQ(e1.param5, 5);
    let e2: StructExtendTest.E = g.barG.getE();
    assertEQ(e2.param4, 4);
    assertEQ(e2.param5, 5);
}

function main() {
    const suite = new ArkTestsuite("struct extend ut")
    suite.addTest("check struct A", check_struct_A)
    suite.addTest("check struct B", check_struct_B)
    suite.addTest("check struct C", check_struct_C)
    suite.addTest("check struct D", check_struct_D)
    suite.addTest("check struct E", check_struct_E)
    suite.addTest("check class bar", check_class_Bar)
    suite.addTest("check struct F", check_class_Bar)
    suite.addTest("check struct G", check_class_Bar)
    exit(suite.run())
}
