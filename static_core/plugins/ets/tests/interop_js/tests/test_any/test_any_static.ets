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

import { Foo, fooInstance } from "testDecl";
import { NumberMap, numberMapInstance } from "testDecl";
import { functionArg0, functionArg1, functionArg2 } from "testDecl";
import { Qux, quxInstance } from "testDecl";
import { TestNew0, TestNew1, TestNew2 } from "testDecl";

function testIsInstance() {
    let res = fooInstance instanceof Foo;
    arktest.assertEQ(res, true);
}

function testLdByName() {
    let bar = fooInstance.bar;
    arktest.assertEQ(bar, 0xcafe);

    let baz = fooInstance.baz;
    arktest.assertEQ(baz, "hello world");
}

function testLdByVal() {
    let bar = fooInstance["bar"];
    arktest.assertEQ(bar, 0xcafe);

    let baz = fooInstance["baz"];
    arktest.assertEQ(baz, "hello world");
}

function testLdByIdx() {
    let babe = numberMapInstance[1];
    arktest.assertEQ(babe, 0xbabe);
}

function testStByIdx() {
    numberMapInstance[1] = 0xabcd;

    let babe = numberMapInstance[1];
    arktest.assertEQ(babe, 0xabcd);
}

function testStByName() {
    fooInstance.baz = "updated";
    let baz = fooInstance.baz;
    arktest.assertEQ(baz, "updated");
}

function testStByVal() {
    fooInstance["bar"] = 0xf00d;
    let bar = fooInstance["bar"];
    arktest.assertEQ(bar, 0xf00d);

    fooInstance["baz"] = "testStByVal";
    let baz = fooInstance["baz"];
    arktest.assertEQ(baz, "testStByVal");
}

function testCall0() {
    let cafe = functionArg0();
    arktest.assertEQ(cafe, 0xcafe);
}

function testCallShort() {
    let caff = functionArg1(0xcafe);
    arktest.assertEQ(caff, 0xcaff);
}

function testCallRange() {
    let caff = functionArg2(0xcafe, 1);
    arktest.assertEQ(caff, 0xcaff);
}

function testCallThis0() {
    let cafe = quxInstance.methodArg0();
    arktest.assertEQ(cafe, 0xcafe);
}

function testCallThis1() {
    let caff = quxInstance.methodArg1(0xcafe);
    arktest.assertEQ(caff, 0xcaff);
}

function testCallThis2() {
    let caff = quxInstance.methodArg2(0xcafe, 1);
    arktest.assertEQ(caff, 0xcaff);
}

function testCallNew0() {
    let testNew0Instance = new TestNew0();
    let cafe = testNew0Instance.bar;
    arktest.assertEQ(cafe, 0xcafe);
}

function testCallNew1() {
    let testNew1Instance = new TestNew1(0xcafe);
    let cafe = testNew1Instance.bar;
    arktest.assertEQ(cafe, 0xcafe);
}

function testCallNew2() {
    let testNew2Instance = new TestNew2(0xcafe, 1);
    let caff = testNew2Instance.bar;
    arktest.assertEQ(caff, 0xcaff);
}

function testAll() {
    testIsInstance();
    testLdByName();
    testLdByVal();
    testLdByIdx();
    testStByName();
    testStByVal();
    testStByIdx();
    testCall0();
    testCallShort();
    testCallRange();
    testCallThis0();
    testCallThis1();
    testCallThis2();
    testCallNew0();
    testCallNew1();
    testCallNew2();
}