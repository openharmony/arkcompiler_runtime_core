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
import * as IfaceTest from "@generated/keep_name_test";
import * as System from "std/core/System";

loadLibrary("ani_keep_name");

class SubFoo implements IfaceTest.Foo {
    Bar(): void {}

    name: string = "";
}

interface Person {
    get name(): string;
}

class SubPerson implements Person {
    readonly name: string = "";
}

function getIfaceName() {
    let foo = IfaceTest.GetFooIface();
    let name = foo.name;
    console.log("foo.name is " + name)
    assertEQ(name, "foo")
}

function setIfaceName() {
    let foo = IfaceTest.GetFooIface();
    foo.name = "alice"
    let name = foo.name;
    console.log("foo.name is " + name)
    assertEQ(name, "alice")
}

function printFooName() {
    let foo = IfaceTest.GetFooIface();
    foo.name = "bob"
    let name = IfaceTest.PrintFooName(foo)
    foo.Bar();
    console.log("foo.name is " + name)
    assertEQ(name, "bob")
}

function setSubFooName() {
    let subFoo = new SubFoo()
    subFoo.name = "aaa"
    let name = subFoo.name
    subFoo.Bar()
    console.log("subFoo.name is " + name)
    assertEQ(name, "aaa")
}

function ibase_on() {
    let a = IfaceTest.GetIBase("hello", "world")
    try {
        a.on("set", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function ibase_off() {
    let a = IfaceTest.GetIBase("hello", "world")
    try {
        a.off("set", () => {
            console.log("void input void output callback!");
        })
    } catch (error) {
        console.error("main finsih test ERROR ", error)
    }
}

function main() {
    console.log("run main ... IfacePropTest ...")

    const suite = new ArkTestsuite("IfacePropTest")
    suite.addTest("ibase_on", ibase_on)
    suite.addTest("ibase_off", ibase_off)
    suite.addTest("getIfaceName", getIfaceName)
    suite.addTest("setIfaceName", setIfaceName)
    suite.addTest("printFooName", printFooName)
    suite.addTest("setSubFooName", setSubFooName)
    System.exit(suite.run())
}