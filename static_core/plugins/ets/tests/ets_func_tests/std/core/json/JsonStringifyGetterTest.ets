/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import { jsonx } from "std/core"

// ---------------------------------------------------------------------------
// Test classes
// ---------------------------------------------------------------------------

class Simple {
    name: string = "Alice"
    age: number = 30
}


class SimpleWithGetter {
    name: string = "Alice"
    age: number = 30

    @JSONStringifyGetter
    get fullLabel(): string {
        return this.name + "_" + this.age
    }
}

class WithRegularMethod {
    value: number = 42

    notIncluded(): string {
        return "should not appear"
    }

    get unannotatedGetter(): string {
        return "also should not appear"
    }
}

class BaseWithGetter {
    x: number = 1

    @JSONStringifyGetter
    get baseComputed(): string {
        return "base_" + this.x
    }
}

class DerivedWithGetter extends BaseWithGetter {
    y: number = 2

    @JSONStringifyGetter
    get derivedComputed(): string {
        return "derived_" + this.y
    }
}

class MultipleGetters {
    a: number = 10
    b: number = 20

    @JSONStringifyGetter
    get sum(): number {
        return this.a + this.b
    }

    @JSONStringifyGetter
    get product(): number {
        return this.a * this.b
    }
}

class GetterWithIgnoredField {
    @JSONStringifyIgnore
    secret: string = "hidden"

    @JSONStringifyGetter
    get hint(): string {
        return "hint_value"
    }
}

// ---------------------------------------------------------------------------
// Test: stringifyWithGetters includes get accessor properties
// ---------------------------------------------------------------------------

function testGetterIncluded(): void {
    const obj = new SimpleWithGetter()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getString("name"), "Alice")
    arktest.assertEQ(parsed.getInteger("age"), 30)
    arktest.assertEQ(parsed.getString("fullLabel"), "Alice_30")
}

// ---------------------------------------------------------------------------
// Test: JSON.stringify does NOT include get accessor properties
// ---------------------------------------------------------------------------

function testStringifyDoesNotIncludeGetters(): void {
    const obj = new SimpleWithGetter()
    const json = JSON.stringify(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getString("name"), "Alice")
    arktest.assertEQ(parsed.getInteger("age"), 30)
    arktest.assertTrue(parsed.tryGetElement("fullLabel") == undefined)
}

// ---------------------------------------------------------------------------
// Test: regular (non-get) method is not included by stringifyWithGetters
// ---------------------------------------------------------------------------

function testRegularMethodExcluded(): void {
    const obj = new WithRegularMethod()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getInteger("value"), 42)
    arktest.assertTrue(parsed.tryGetElement("notIncluded") == undefined)
    arktest.assertTrue(parsed.tryGetElement("unannotatedGetter") == undefined)
}

// ---------------------------------------------------------------------------
// Test: inherited get accessor is also included
// ---------------------------------------------------------------------------

function testInheritedGetterIncluded(): void {
    const obj = new DerivedWithGetter()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getInteger("x"), 1)
    arktest.assertEQ(parsed.getInteger("y"), 2)
    arktest.assertEQ(parsed.getString("baseComputed"), "base_1")
    arktest.assertEQ(parsed.getString("derivedComputed"), "derived_2")
}

// ---------------------------------------------------------------------------
// Test: multiple get accessors on a single class
// ---------------------------------------------------------------------------

function testMultipleGetters(): void {
    const obj = new MultipleGetters()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getInteger("a"), 10)
    arktest.assertEQ(parsed.getInteger("b"), 20)
    arktest.assertEQ(parsed.getInteger("sum"), 30)
    arktest.assertEQ(parsed.getInteger("product"), 200)
}

// ---------------------------------------------------------------------------
// Test: @JSONStringifyIgnore on field + get accessor
// ---------------------------------------------------------------------------

function testGetterWithIgnoredField(): void {
    const obj = new GetterWithIgnoredField()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertTrue(parsed.tryGetElement("secret") == undefined)
    arktest.assertEQ(parsed.getString("hint"), "hint_value")
}

// ---------------------------------------------------------------------------
// Test: stringifyWithGetters respects replacer array (fieldsFilter)
// ---------------------------------------------------------------------------

function testGetterWithFieldsFilter(): void {
    const obj = new SimpleWithGetter()

    const filterWith: Array<string> = ["name", "fullLabel"]
    const jsonWith = JSON.stringifyWithGetters(obj, filterWith)
    const parsedWith = JSON.parseJsonElement(jsonWith)
    arktest.assertEQ(parsedWith.getString("name"), "Alice")
    arktest.assertEQ(parsedWith.getString("fullLabel"), "Alice_30")
    arktest.assertTrue(parsedWith.tryGetElement("age") == undefined)

    const filterWithout: Array<string> = ["name", "age"]
    const jsonWithout = JSON.stringifyWithGetters(obj, filterWithout)
    const parsedWithout = JSON.parseJsonElement(jsonWithout)
    arktest.assertEQ(parsedWithout.getString("name"), "Alice")
    arktest.assertEQ(parsedWithout.getInteger("age"), 30)
    arktest.assertTrue(parsedWithout.tryGetElement("fullLabel") == undefined)
}

// ---------------------------------------------------------------------------
// Test classes for interface getters
// ---------------------------------------------------------------------------

interface SimpleGetterIface {
    @JSONStringifyGetter
    get computed(): string
}

class SimpleIfaceImpl implements SimpleGetterIface {
    value: int = 7

    @JSONStringifyGetter
    override get computed(): string {
        return "val_" + this.value
    }
}

interface MultiGetterIface {
    @JSONStringifyGetter
    get alpha(): int

    @JSONStringifyGetter
    get beta(): string
}

class MultiGetterIfaceImpl implements MultiGetterIface {
    @JSONStringifyGetter
    override get alpha(): int {
        return 100
    }

    @JSONStringifyGetter
    override get beta(): string {
        return "hello"
    }
}

interface BaseIface {
    @JSONStringifyGetter
    get baseVal(): string
}

interface DerivedIface extends BaseIface {
    @JSONStringifyGetter
    get derivedVal(): int
}

class DerivedIfaceImpl implements DerivedIface {
    @JSONStringifyGetter
    override get baseVal(): string {
        return "from_base"
    }

    @JSONStringifyGetter
    override get derivedVal(): int {
        return 42
    }
}

class ParentWithIface implements SimpleGetterIface {
    @JSONStringifyGetter
    override get computed(): string {
        return "parent"
    }
}

class ChildOfParentWithIface extends ParentWithIface {
    extra: int = 99
}

interface IfaceWithoutAnnotation {
    get plain(): string
}

class ImplWithoutAnnotation implements IfaceWithoutAnnotation {
    override get plain(): string {
        return "no_annotation"
    }
}

// ---------------------------------------------------------------------------
// Test: interface getter included in stringifyWithGetters
// ---------------------------------------------------------------------------

function testInterfaceGetterIncluded(): void {
    const obj = new SimpleIfaceImpl()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getInteger("value"), 7)
    arktest.assertEQ(parsed.getString("computed"), "val_7")
}

// ---------------------------------------------------------------------------
// Test: interface getter is also included in plain stringify
// ---------------------------------------------------------------------------

function testInterfaceGetterInStringify(): void {
    const obj = new SimpleIfaceImpl()
    const json = JSON.stringify(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getInteger("value"), 7)
    arktest.assertEQ(parsed.getString("computed"), "val_7")
}

// ---------------------------------------------------------------------------
// Test: multiple getters from interface
// ---------------------------------------------------------------------------

function testMultipleInterfaceGetters(): void {
    const obj = new MultiGetterIfaceImpl()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getInteger("alpha"), 100)
    arktest.assertEQ(parsed.getString("beta"), "hello")
}

// ---------------------------------------------------------------------------
// Test: inherited interface getters (interface extends interface)
// ---------------------------------------------------------------------------

function testDerivedInterfaceGetters(): void {
    const obj = new DerivedIfaceImpl()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getString("baseVal"), "from_base")
    arktest.assertEQ(parsed.getInteger("derivedVal"), 42)
}

// ---------------------------------------------------------------------------
// Test: child class inherits interface getter from parent
// ---------------------------------------------------------------------------

function testChildInheritsInterfaceGetter(): void {
    const obj = new ChildOfParentWithIface()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getInteger("extra"), 99)
    arktest.assertEQ(parsed.getString("computed"), "parent")
}

// ---------------------------------------------------------------------------
// Test: interface getter without @JSONStringifyGetter is still included
// ---------------------------------------------------------------------------

function testUnannotatedInterfaceGetterIncluded(): void {
    const obj = new ImplWithoutAnnotation()
    const json = JSON.stringifyWithGetters(obj)
    const parsed = JSON.parseJsonElement(json)
    arktest.assertEQ(parsed.getString("plain"), "no_annotation")
}

// ---------------------------------------------------------------------------
// Test: interface getter with fieldsFilter
// ---------------------------------------------------------------------------

function testInterfaceGetterWithFieldsFilter(): void {
    const obj = new MultiGetterIfaceImpl()

    const filterWith: Array<string> = ["alpha"]
    const jsonWith = JSON.stringifyWithGetters(obj, filterWith)
    const parsedWith = JSON.parseJsonElement(jsonWith)
    arktest.assertEQ(parsedWith.getInteger("alpha"), 100)
    arktest.assertTrue(parsedWith.tryGetElement("beta") == undefined)

    const filterWithout: Array<string> = ["beta"]
    const jsonWithout = JSON.stringifyWithGetters(obj, filterWithout)
    const parsedWithout = JSON.parseJsonElement(jsonWithout)
    arktest.assertTrue(parsedWithout.tryGetElement("alpha") == undefined)
    arktest.assertEQ(parsedWithout.getString("beta"), "hello")
}

// ---------------------------------------------------------------------------
// Test: stringifyWithGetters then parseUpdate — getter keys ignored, fields updated
// ---------------------------------------------------------------------------

function testStringifyWithGettersThenParseUpdate(): void {
    const src = new Simple()
    src.name = "Bob"
    src.age = 25

    const json = JSON.stringifyWithGetters(src)

    const target = new SimpleWithGetter()
    JSON.parseUpdate<SimpleWithGetter>(json, target)

    arktest.assertEQ(target.name, "Bob")
    arktest.assertEQ(target.age, 25)
}

// ---------------------------------------------------------------------------
// Test: stringifyWithGetters then parseUpdate — inherited getters
// ---------------------------------------------------------------------------

function testStringifyWithGettersThenParseUpdateInherited(): void {
    const src = new DerivedWithGetter()
    src.x = 100
    src.y = 200

    const json = JSON.stringifyWithGetters(src)

    const target = new DerivedWithGetter()
    JSON.parseUpdate<DerivedWithGetter>(json, target)

    arktest.assertEQ(target.x, 100)
    arktest.assertEQ(target.y, 200)
}

// ---------------------------------------------------------------------------
// Test: stringifyWithGetters then parseUpdate — multiple getters, fields preserved
// ---------------------------------------------------------------------------

function testStringifyWithGettersThenParseUpdateMultiple(): void {
    const src = new MultipleGetters()
    src.a = 5
    src.b = 8

    const json = JSON.stringifyWithGetters(src)

    const target = new MultipleGetters()
    target.a = 99
    target.b = 99
    JSON.parseUpdate<MultipleGetters>(json, target)

    arktest.assertEQ(target.a, 5)
    arktest.assertEQ(target.b, 8)
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringifyWithGetters')
    suite.addTest('get accessor included in output', testGetterIncluded)
    suite.addTest('JSON.stringify does not include get accessors', testStringifyDoesNotIncludeGetters)
    suite.addTest('regular method excluded', testRegularMethodExcluded)
    suite.addTest('inherited get accessor included', testInheritedGetterIncluded)
    suite.addTest('multiple get accessors', testMultipleGetters)
    suite.addTest('ignored field with get accessor', testGetterWithIgnoredField)
    suite.addTest('get accessor with fieldsFilter', testGetterWithFieldsFilter)
    suite.addTest('interface getter included', testInterfaceGetterIncluded)
    suite.addTest('interface getter in plain stringify', testInterfaceGetterInStringify)
    suite.addTest('multiple interface getters', testMultipleInterfaceGetters)
    suite.addTest('derived interface getters', testDerivedInterfaceGetters)
    suite.addTest('child inherits interface getter', testChildInheritsInterfaceGetter)
    suite.addTest('unannotated interface getter included', testUnannotatedInterfaceGetterIncluded)
    suite.addTest('interface getter with fieldsFilter', testInterfaceGetterWithFieldsFilter)
    suite.addTest('stringifyWithGetters then parseUpdate', testStringifyWithGettersThenParseUpdate)
    suite.addTest('stringifyWithGetters then parseUpdate inherited', testStringifyWithGettersThenParseUpdateInherited)
    suite.addTest('stringifyWithGetters then parseUpdate multiple', testStringifyWithGettersThenParseUpdateMultiple)
    return suite.run()
}
