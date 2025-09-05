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
import * as IfaceReadonlyTest from "iface_readonly_test";
import * as IfaceTest from "iface_test";

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

class BaseImpl implements IfaceTest.IBase {
    id: string;
    constructor(id: string) {
        this.id = id;
    }
    getId(): string {
        return this.id;
    }
    setId(id: string): void {
        this.id = id;
        return;
    }
}

function getIfaceName() {
    let foo = IfaceTest.getFooIface();
    let name = foo.name;
    console.log("foo.name is " + name)
    arktest.assertEQ(name, "foo")
}

function getIfaceReadonlyName() {
    let noo = IfaceReadonlyTest.getNooIface();
    let nooName = noo.name;
    console.log("noo.name is " + nooName)
    arktest.assertEQ(nooName, "noo")
}

function setIfaceName() {
    let foo = IfaceTest.getFooIface();
    foo.name = "alice"
    let name = foo.name;
    console.log("foo.name is " + name)
    arktest.assertEQ(name, "alice")
}

function printFooName() {
    let foo = IfaceTest.getFooIface();
    foo.name = "bob"
    let name = IfaceTest.printFooName(foo)
    console.log("foo.name is " + name)
    arktest.assertEQ(name, "bob")
}

class Noo1 implements IfaceReadonlyTest.Noo {
    name: string = "hello";
    age?: int;
    bar(): void {
        console.log("Noo1");
    }
    constructor(name: string, age?: int) {
        this.name = name;
        this.age = age;
    }
}

function printNooName() {
    let noo = IfaceReadonlyTest.getNooIface();
    let name = IfaceReadonlyTest.printNooName(noo);
    console.log("noo name: ", noo.name);
    console.log("noo age: ", noo.age);
    console.log("print noo name: ", name);

    let a1 = new Noo1("test1", 100);
    console.log(a1);
    let a2 = new Noo1("test2");
    console.log(a2);
    console.log("noo.name is " + name)
    arktest.assertEQ(name, "noo")
}

function setSubFooName() {
    let subFoo = new SubFoo()
    subFoo.name = "aaa"
    let name = subFoo.name
    console.log("subFoo.name is " + name)
    arktest.assertEQ(name, "aaa")
}

function test_iface_subBaseFun_base() {
    let sub = IfaceTest.getSubBaseFun();
    let info = sub.base();

    console.log("sub.base is " + info)
    arktest.assertEQ(info, "SubBaseFun.base")
}

function makeIBase() {
    let sub = IfaceTest.getSubBaseElem();
    sub.base = "SubBaseElem.base";
    let info = sub.base;

    console.log("SubBaseElem.base is " + info)
    arktest.assertEQ(info, "SubBaseElem.base")
}

function test_iface_inherit() {
    let ibase_1 = IfaceTest.makeIBase("abc");
    console.log("ibase_1 getId:", ibase_1.getId());
    let ibase_1_id = ibase_1.getId();
    arktest.assertEQ(ibase_1_id, "abc");

    ibase_1.setId("xyz");
    console.log("ibase_1 setId:", ibase_1.getId());
    ibase_1_id = ibase_1.getId();
    arktest.assertEQ(ibase_1_id, "xyz");

    let ibase_2 = IfaceTest.makeIBase("test");
    IfaceTest.copyIBase(ibase_1, ibase_2);
    ibase_1_id = ibase_1.getId();
    let ibase_2_id = ibase_2.getId();
    console.log("copyIBase:", ibase_1_id, ibase_2_id);
    arktest.assertEQ(ibase_1_id, "test");
    arktest.assertEQ(ibase_2_id, "test");

    let ishape_1 = IfaceTest.makeIShape("shape", 3.14, 2.5);
    console.log("makeIShape:", ishape_1.calculateArea());
    ibase_2_id = ibase_2.getId();
    let ishape_1_id = ishape_1.getId();
    arktest.assertEQ(ibase_2_id, "test");
    console.log("interface extends:", ishape_1_id);
    arktest.assertEQ(ishape_1_id, "shape");

    ishape_1.setId("aaaaa");
    ishape_1_id = ishape_1.getId();
    console.log("interface extends set:", ishape_1_id);
    arktest.assertEQ(ishape_1_id, "aaaaa");

    let a: BaseImpl = new BaseImpl("A");
    let b: BaseImpl = new BaseImpl("B");
    IfaceTest.copyIBase(b, a);
    let a_id = a.getId();
    let b_id = b.getId();
    console.log("impl interface: ", a_id, b_id);
    arktest.assertEQ(a_id, "A");
    arktest.assertEQ(b_id, "A");

    IfaceTest.copyIBase(ibase_1, ishape_1);
    ibase_1_id = ibase_1.getId();
    ishape_1_id = ishape_1.getId();
    console.log("interface extends: ", ibase_1_id, ishape_1_id);
    arktest.assertEQ(ibase_1_id, "aaaaa");
    arktest.assertEQ(ishape_1_id, "aaaaa");

    let d = new IfaceTest.IDerived();
    d.call();
    arktest.assertEQ(d.getId(), "d");
}

function test_iface_subBaseElem_base() {
    let sub = IfaceTest.getSubBaseElem();
    sub.base = "SubBaseElem.base";
    let info = sub.base;

    console.log("SubBaseElem.base is " + info)
    arktest.assertEQ(info, "SubBaseElem.base")
}

function main() {
    console.log("run main ... IfacePropTest ...")
    const suite = new arktest.ArkTestsuite("IfacePropTest")

    suite.addTest("getIfaceName", getIfaceName)
    suite.addTest("getIfaceReadonlyName", getIfaceReadonlyName)
    suite.addTest("setIfaceName", setIfaceName)
    suite.addTest("printFooName", printFooName)
    suite.addTest("printNooName", printNooName)
    suite.addTest("setSubFooName", setSubFooName)

    suite.addTest("test_iface_subBaseFun_base", test_iface_subBaseFun_base)
    suite.addTest("test_iface_subBaseElem_base", test_iface_subBaseElem_base)
    suite.addTest("test_iface_inherit", test_iface_inherit)

    exit(suite.run())
}