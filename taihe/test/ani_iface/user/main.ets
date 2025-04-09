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
import * as IfaceReadonlyTest from "@generated/iface_readonly_test";
import * as IfaceTest from "@generated/iface_test";
import * as System from "std/core/System";

loadLibrary("ani_iface");

class SubFoo implements IfaceTest.Foo {
    bar(): void {}

    name: string = "";
}

interface Person {
    get name(): string;
}

class SubPerson implements Person {
    readonly name: string = "";
}

function getIfaceName() {
    let foo = IfaceTest.getFooIface();
    let name = foo.name;
    console.log("foo.name is " + name)
    assertEQ(name, "foo")
}

function getIfaceReadonlyName() {
    let noo = IfaceReadonlyTest.getNooIface();
    let nooName = noo.name;
    console.log("noo.name is " + nooName)
    assertEQ(nooName, "noo")
}

function setIfaceName() {
    let foo = IfaceTest.getFooIface();
    foo.name = "alice"
    let name = foo.name;
    console.log("foo.name is " + name)
    assertEQ(name, "alice")
}

function printFooName() {
    let foo = IfaceTest.getFooIface();
    foo.name = "bob"
    let name = IfaceTest.printFooName(foo)
    console.log("foo.name is " + name)
    assertEQ(name, "bob")
}

function printNooName() {
    let noo = IfaceReadonlyTest.getNooIface();
    let name = IfaceReadonlyTest.printNooName(noo)
    console.log("noo.name is " + name)
    assertEQ(name, "noo")
}

function setSubFooName() {
    let subFoo = new SubFoo()
    subFoo.name = "aaa"
    let name = subFoo.name
    console.log("subFoo.name is " + name)
    assertEQ(name, "aaa")
}

function main() {
    console.log("run main ... IfacePropTest ...")

    const suite = new ArkTestsuite("IfacePropTest")
    suite.addTest("getIfaceName", getIfaceName)
    suite.addTest("getIfaceReadonlyName", getIfaceReadonlyName)
    suite.addTest("setIfaceName", setIfaceName)
    suite.addTest("printFooName", printFooName)
    suite.addTest("printNooName", printNooName)
    suite.addTest("setSubFooName", setSubFooName)
    System.exit(suite.run())
}