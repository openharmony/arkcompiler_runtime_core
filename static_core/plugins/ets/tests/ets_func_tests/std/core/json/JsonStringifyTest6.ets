/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringify indentation and toJSON tests')
    suite.addTest('stringify lambda returns string', strSelfCheck)
    suite.addTest('indent=1', indent1)
    suite.addTest('indent=2', indent2)
    suite.addTest('indent=4', indent4)
    suite.addTest('indent=0 compact', indent0)
    suite.addTest('indent=-1 compact', indentNeg1)
    suite.addTest('indent=20 max-10', indent20)
    suite.addTest('nested indent=2', indentNested)
    suite.addTest('array indent=2', indentArray)
    suite.addTest('tab indent', indentTab)
    suite.addTest('two-space indent', indentTwoSpaces)
    suite.addTest('three-dash indent', indentThreeDash)
    suite.addTest('13-char indent truncated', indent13Chars)
    suite.addTest('10-char indent', indent10Chars)
    suite.addTest('undefined indent', indentUndef)
    suite.addTest('undefined indent 2', indentUndef2)
    suite.addTest('nested toJSON result', toJSONNested)
    suite.addTest('toJSON in array', toJSONInArray)
    suite.addTest('multiple toJSON objs', toJSONMultiple)
    suite.addTest('toJSON with replacer', toJSONWithReplacer)
    suite.addTest('replacer removes prop2', replacerFilterProp)
    suite.addTest('prop list selects prop1,prop3', propListSelect)
    suite.addTest('pretty-printed single prop', prettyProp)
    suite.addTest('pretty-printed nested', prettyNested)
    suite.addTest('replacer filters strings', replacerFilterStr)
    suite.addTest('replacer removes password', replacerFilterPwd)
    suite.addTest('replacer doubles numbers', replacerDoubleNum)
    suite.addTest('replacer called on root key', replacerCallsRoot)
    suite.addTest('replacer returns undefined root', replacerReturnUndef)
    suite.addTest('replacer returns null root', replacerReturnNull)
    suite.addTest('replacer returns function root', replacerReturnFunc)
    suite.addTest('array replacer undef', replacerArrUndef)
    suite.addTest('array replacer func', replacerArrFunc)
    suite.addTest('prop list selects week,month', propListSelectWeek)
    suite.addTest('prop list single key b', propListSelectB)
    suite.addTest('prop list extra keys', propListExtraKeys)
    suite.addTest('prop list nested outer', propListNested)
    suite.addTest('prop list deep nested', propListDeepNested)
    suite.addTest('undefined replacer arg', noReplacer1)
    suite.addTest('undefined replacer 2', noReplacer2)
    suite.addTest('no replacer arg', noReplacer3)
    return suite.run()
}

class EmptyClass { }

interface NestedToJSONHolder {
    inner: ObjectLiteralTextToJSONClass;
}

interface SingleNumberField {
    a: number;
}

interface NestedToJSONStringResultHolder {
    x: ObjectLiteralTextToJSONClass;
    y: ExtendedObjectLiteralTextToJSONClass;
}

class StringOverrideToJSONClass implements JsonSerializable {
    foo: string = 'foo';
    toJSON(): string {
        return 'bar';
    }
}

class ObjectLiteralTextToJSONClass implements JsonSerializable {
    a: number = 1;
    toJSON(): string {
        return '{ b: 2 }';
    }
}

class NumericTextToJSONClass implements JsonSerializable {
    a: number = 1;
    toJSON(): string {
        return '42';
    }
}

class ExtendedObjectLiteralTextToJSONClass implements JsonSerializable {
    a: number = 1;
    toJSON(): string {
        return '{ b: 2, c: 3 }';
    }
}

class UndefinedTextToJSONClass implements JsonSerializable {
    a: number = 1;
    toJSON(): string {
        return 'undefined';
    }
}

class FunctionTextToJSONClass implements JsonSerializable {
    a: number = 1;
    toJSON(): string {
        return '() => {}';
    }
}

class ArrayTextToJSONClass implements JsonSerializable {
    data: number[] = [1, 2, 3];
    toJSON(): string {
        return JSON.stringify(this.data);
    }
}

class SinglePropClass {
    name: string = 'test';
}

class ConstructorValueClass {
    value: number;
    constructor(value: number) {
        this.value = value;
    }
}

class PrimitiveFieldsClass {
    a: number = 1;
    b: string = 'test';
    c: boolean = true;
}

class TwoLevelParent {
    parentProp: string = 'parent';
}

class TwoLevelChild extends TwoLevelParent {
    childProp: string = 'child';
}

class ThreeLevelGrandparent {
    grandProp: string = 'grand';
}

class ThreeLevelParent extends ThreeLevelGrandparent {
    parentProp: string = 'parent';
}

class ThreeLevelChild extends ThreeLevelParent {
    childProp: string = 'child';
}

class OverridePropParent {
    prop: string = 'parent';
}

class OverridePropChild extends OverridePropParent {
    prop: string = 'child';
}

class PublicPropParent {
    publicProp: string = 'public';
}

class PublicPropChild extends PublicPropParent { }

class PrivatePropParent {
    private privateProp: string = 'private';
}

class PrivatePropChild extends PrivatePropParent { }

class ProtectedPropParent {
    protected protectedProp: string = 'protected';
}

class ProtectedPropChild extends ProtectedPropParent { }

interface InterfaceWithExtends {
    interfaceProp: string;
}

class InterfaceExtendsParent {
    parentProp: string = 'parent';
}

class InterfaceExtendsChild extends InterfaceExtendsParent implements InterfaceWithExtends {
    interfaceProp: string = 'interface';
}

interface ImplThenExtendInterface {
    interfaceProp: string;
}

class ImplThenExtendParent implements ImplThenExtendInterface {
    interfaceProp: string = 'interface';
}

class ImplThenExtendChild extends ImplThenExtendParent {
    childProp: string = 'child';
}

interface SingleFieldInterface {
    prop: string;
}

class SingleFieldImpl implements SingleFieldInterface {
    prop: string = 'value';
}

interface OptionalFieldInterface {
    required: string;
    optional?: number;
}

class OptionalFieldImpl implements OptionalFieldInterface {
    required: string = 'value';
    optional?: number = 42;
}

interface ReadonlyFieldInterface {
    readonly readonlyProp: string;
}

class ReadonlyFieldImpl implements ReadonlyFieldInterface {
    readonlyProp: string = 'value';
}

interface MultiInterfaceA {
    prop1: string;
}

interface MultiInterfaceB {
    prop2: number;
}

class MultiInterfaceImpl implements MultiInterfaceA, MultiInterfaceB {
    prop1: string = 'value1';
    prop2: number = 42;
}

interface OverlapInterfaceA {
    prop: string;
}

interface OverlapInterfaceB {
    prop: string;
}

class OverlapInterfaceImpl implements OverlapInterfaceA, OverlapInterfaceB {
    prop: string = 'value';
}

class InstanceMethodClass {
    instanceMethod(): string {
        return 'result';
    }
}

class PropAndMethodClass {
    prop: string = 'value';
    method(): string {
        return 'result';
    }
}

class StaticMethodClass {
    static staticMethod(): string {
        return 'result';
    }
}

class GetterClass {
    private _value: string = 'value';
    get value(): string {
        return this._value;
    }
}

class SetterClass {
    private _value: string = 'value';
    set value(val: string) {
        this._value = val;
    }
}

class GetterSetterClass {
    private _value: string = 'value';
    get value(): string {
        return this._value;
    }
    set value(val: string) {
        this._value = val;
    }
}

class PublicPropClass {
    publicProp: string = 'public';
}

class PublicMethodClass {
    publicMethod(): string {
        return 'method';
    }
}

class NumberStringClass {
    a: number = 1;
    b: string = 'test';
}

class PrivatePropClass {
    private privateProp: string = 'private';
}

class PrivateMethodClass {
    private privateMethod(): string {
        return 'private';
    }
}

class PublicPrivatePropClass {
    publicProp: string = 'public';
    private privateProp: string = 'private';
}

class ProtectedPropClass {
    protected protectedProp: string = 'protected';
}

class ProtectedMethodClass {
    protected protectedMethod(): string {
        return 'protected';
    }
}

class PublicProtectedPropClass {
    publicProp: string = 'public';
    protected protectedProp: string = 'protected';
}

class ReadonlyPropClass {
    readonly readonlyProp: string = 'readonly';
}

class ReadonlyValueClass {
    readonly prop: string = 'value';
}

class PrivateReadonlyClass {
    private readonly prop: string = 'value';
}

class StaticPropClass {
    static staticProp: string = 'static';
}

class StaticMethodOnlyClass {
    static staticMethod(): string {
        return 'static';
    }
}

class StaticAndInstanceClass {
    static staticProp: string = 'static';
    instanceProp: string = 'instance';
}

abstract class AbstractBase {
    abstractProp: string = 'abstract';
}

class ConcreteSub extends AbstractBase {
    concreteProp: string = 'concrete';
}

class GenericValueHolder<T> {
    value: T;
    constructor(value: T) {
        this.value = value;
    }
}

class CustomToJSONClass implements JsonSerializable {
    prop: string = 'value';
    toJSON(): string {
        return 'custom';
    }
}

class InheritedToJSONParent implements JsonSerializable {
    toJSON(): string {
        return 'parent';
    }
}

class InheritedToJSONChild extends InheritedToJSONParent { }

class OverriddenToJSONParent implements JsonSerializable {
    toJSON(): string {
        return 'parent';
    }
}

class OverriddenToJSONChild extends OverriddenToJSONParent {
    toJSON(): string {
        return 'child';
    }
}

class OptionalPropClass {
    prop?: string;
}

class NestedInner {
    innerProp: string = 'inner';
}

class NestedOuter {
    outerProp: string = 'outer';
    inner: NestedInner = new NestedInner();
}

class ArrayItem {
    value: number = 0;
}

class ArrayContainer {
    items: ArrayItem[] = [new ArrayItem(), new ArrayItem()];
}

class MapItem {
    value: number = 0;
}

class MapContainer {
    itemMap: Record<string, MapItem> = { 'a': new MapItem(), 'b': new MapItem() };
}

class TwoStringFieldsClass {
    prop1: string = 'value1';
    prop2: string = 'value2';
}

class NumberValueClass {
    value: number = 42;
}

class ThreeStringFieldsClass {
    prop1: string = 'value1';
    prop2: string = 'value2';
    prop3: string = 'value3';
}

class PrettyPrintedPropClass {
    prop: string = 'value';
}

class PrettyPrintedNestedInner {
    innerProp: string = 'inner';
}

class PrettyPrintedNestedOuter {
    outerProp: string = 'outer';
    inner: PrettyPrintedNestedInner = new PrettyPrintedNestedInner();
}

interface SingleNumberIndentFields {
    a?: number;
}

interface NestedNumberIndentOuter {
    a?: NestedNumberIndentInner;
}

interface NestedNumberIndentInner {
    b?: number;
}

interface NumberArrayIndentFields {
    a?: number[];
}

interface ReplacerFilteredFields {
    a?: string;
    b?: number;
    username?: string;
    password?: string;
}

interface ReplacerDoubledNumberFields {
    a?: number;
    b?: string;
}

interface ReplacerRootValueFields {
    a?: number;
}

interface ReplacerPropertyListFields {
    a?: number;
}

interface DatePartFields {
    week?: number;
    month?: number;
    year?: number;
}

interface ThreeNumberFieldsForPropertyList {
    a?: number;
    b?: number;
    c?: number;
}

interface NestedReplacerOuter {
    outer?: NestedReplacerInner;
}

interface NestedReplacerInner {
    inner?: string;
}

interface DeepNestedOuter {
    a?: DeepNestedMiddle;
}

interface DeepNestedMiddle {
    b?: DeepNestedInner;
}

interface DeepNestedInner {
    c?: number;
}

interface TwoOptionalNumberFields {
    a?: number;
    b?: number;
}

function strSelfCheck(): void {
    let str: string = JSON.stringify(() => { })
    arktest.assertEQ(str, str)
}

function indent1(): void {
    let obj: SingleNumberIndentFields = { a: 2 }
    let str: string = JSON.stringify(obj, null, 1)
    arktest.assertEQ(str, '{\n "a": 2\n}')
}

function indent2(): void {
    let obj: SingleNumberIndentFields = { a: 2 }
    let str: string = JSON.stringify(obj, null, 2)
    arktest.assertEQ(str, '{\n  "a": 2\n}')
}

function indent4(): void {
    let obj: SingleNumberIndentFields = { a: 2 }
    let str: string = JSON.stringify(obj, null, 4)
    arktest.assertEQ(str, '{\n    "a": 2\n}')
}

function indent0(): void {
    let obj: SingleNumberIndentFields = { a: 2 }
    let str: string = JSON.stringify(obj, null, 0)
    arktest.assertEQ(str, '{"a":2}')
}

function indentNeg1(): void {
    let obj: SingleNumberIndentFields = { a: 2 }
    let str: string = JSON.stringify(obj, undefined, -1)
    arktest.assertEQ(str, '{"a":2}')
}

function indent20(): void {
    let obj: SingleNumberIndentFields = { a: 2 }
    let str: string = JSON.stringify(obj, undefined, 20)
    let expected = '{\n          "a": 2\n}'
    arktest.assertEQ(str, '{\n          "a": 2\n}')
}

function indentNested(): void {
    let obj: NestedNumberIndentOuter = { a: { b: 1 } }
    let str: string = JSON.stringify(obj, null, 2)
    arktest.assertEQ(str, '{\n  "a": {\n    "b": 1\n  }\n}')
}

function indentArray(): void {
    let obj: NumberArrayIndentFields = { a: [1, 2] }
    let str: string = JSON.stringify(obj, undefined, 2)
    arktest.assertEQ(str, '{\n  "a": [\n    1,\n    2\n  ]\n}')
}

function indentTab(): void {
    let obj: SingleNumberIndentFields = { a: 2 }
    let str: string = JSON.stringify(obj, undefined, '\t')
    arktest.assertEQ(str, '{\n\t"a": 2\n}')
}

function indentTwoSpaces(): void {
    let obj: SingleNumberIndentFields = { a: 2 }
    let str: string = JSON.stringify(obj, undefined, '  ')
    arktest.assertEQ(str, '{\n  "a": 2\n}')
}

function indentThreeDash(): void {
    let obj: SingleNumberField = { a: 2 }
    let str: string = JSON.stringify(obj, null, '---')
    arktest.assertEQ(str, '{\n---"a": 2\n}')
}

function indent13Chars(): void {
    let obj: SingleNumberField = { a: 2 }
    let str: string = JSON.stringify(obj, null, '0123456789ABC')
    arktest.assertEQ(str, '{\n0123456789"a": 2\n}')
}

function indent10Chars(): void {
    let obj: SingleNumberField = { a: 2 }
    let str: string = JSON.stringify(obj, null, '0123456789')
    arktest.assertEQ(str, '{\n0123456789"a": 2\n}')
}

function indentUndef(): void {
    let obj: SingleNumberField = { a: 2 }
    let str: string = JSON.stringify(obj, null, undefined)
    arktest.assertEQ(str, '{"a":2}')
}

function indentUndef2(): void {
    let obj: SingleNumberField = { a: 2 }
    let str: string = JSON.stringify(obj, null, undefined)
    arktest.assertEQ(str, '{"a":2}')
}

function toJSONNested(): void {
    let inner = new ObjectLiteralTextToJSONClass()
    let outer: NestedToJSONHolder = { inner: inner }
    let str: string = JSON.stringify(outer)
    arktest.assertEQ(str, '{"inner":"{ b: 2 }"}')
}

function toJSONInArray(): void {
    let obj = new NumericTextToJSONClass()
    let str: string = JSON.stringify([obj])
    arktest.assertEQ(str, '["42"]')
}

function toJSONMultiple(): void {
    let obj1 = new ObjectLiteralTextToJSONClass()
    let obj2 = new ExtendedObjectLiteralTextToJSONClass()
    let wrapper: NestedToJSONStringResultHolder = { x: obj1, y: obj2 }
    let str: string = JSON.stringify(wrapper)
    arktest.assertEQ(str, '{"x":"{ b: 2 }","y":"{ b: 2, c: 3 }"}')
}

function toJSONWithReplacer(): void {
    let obj = new ExtendedObjectLiteralTextToJSONClass()
    let replacer = (key: string, value: Any): Any => {
        if (typeof value === 'number') {
            return (value as number) * 2
        }
        return value as ExtendedObjectLiteralTextToJSONClass
    }
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '"{ b: 2, c: 3 }"')
}

function replacerFilterProp(): void {
    const replacer = (key: string, value: Any): Any => {
        if (key === 'prop2') {
            return undefined
        }
        return value
    }
    const obj = new TwoStringFieldsClass()
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{"prop1":"value1"}')
}

function propListSelect(): void {
    const obj = new ThreeStringFieldsClass()
    let str: string = JSON.stringify(obj, ['prop1', 'prop3'])
    arktest.assertEQ(str, '{"prop1":"value1","prop3":"value3"}')
}

function prettyProp(): void {
    const obj = new PrettyPrintedPropClass()
    let str: string = JSON.stringify(obj, null, 2)
    arktest.assertEQ(str, '{\n  "prop": "value"\n}')
}

function prettyNested(): void {
    const obj = new PrettyPrintedNestedOuter()
    let str: string = JSON.stringify(obj, null, 2)
    arktest.assertEQ(str, '{\n  "outerProp": "outer",\n  "inner": {\n    "innerProp": "inner"\n  }\n}')
}

function replacerFilterStr(): void {
    let replacer = (key: string, value: Any): Any => typeof value === 'string' ? undefined : value
    let obj: ReplacerFilteredFields = { a: 'foo', b: 123 }
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{"b":123}')
}

function replacerFilterPwd(): void {
    let replacer = (key: string, value: Any): Any => key === 'password' ? undefined : value
    let obj: ReplacerFilteredFields = { username: 'user', password: 'secret' }
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{"username":"user"}')
}

function replacerDoubleNum(): void {
    let replacer = (key: string, value: Any): Any => {
        if (typeof value === 'number') {
            return (value as number) * 2
        }
        return value
    }
    let obj: ReplacerDoubledNumberFields = { a: 5, b: 'text' }
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{"a":10,"b":"text"}')
}

function replacerCallsRoot(): void {
    let called = false
    let replacer = (key: string, value: Any): Any => {
        if (key === '') {
            called = true
        }
        return value
    }
    let obj: ReplacerRootValueFields = { a: 1 }
    JSON.stringify(obj, replacer)
    arktest.assertTrue(called)
}

function replacerReturnUndef(): void {
    let replacer = (key: string, value: Any): Any => undefined
    let obj: ReplacerRootValueFields = { a: 1 }
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '')
}

function replacerReturnNull(): void {
    let replacer = (key: string, value: Any): Any => null
    let obj: ReplacerRootValueFields = { a: 1 }
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, 'null')
}

function replacerReturnFunc(): void {
    let replacer = (key: string, value: Any): Any => (() => { })
    let obj: ReplacerRootValueFields = { a: 1 }
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '')
}

function replacerArrUndef(): void {
    let replacer = (key: string, value: Any): Any => undefined
    let arr: number[] = [1, 2, 3]
    let str: string = JSON.stringify(arr, replacer)
    arktest.assertEQ(str, '')
}

function replacerArrFunc(): void {
    let replacer = (key: string, value: Any): Any => (() => { })
    let arr: number[] = [1, 2, 3]
    let str: string = JSON.stringify(arr, replacer)
    arktest.assertEQ(str, '')
}

function propListSelectWeek(): void {
    let obj: DatePartFields = { week: 45, month: 7, year: 2024 }
    let replacer: (string | number)[] = ['week', 'month']
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{"week":45,"month":7}')
}

function propListSelectB(): void {
    let obj: ThreeNumberFieldsForPropertyList = { a: 1, b: 2, c: 3 }
    let replacer: (string | number)[] = ['b']
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{"b":2}')
}

function propListExtraKeys(): void {
    let obj: ReplacerPropertyListFields = { a: 1 }
    let replacer: (string | number)[] = ['a', 'b', 'c']
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{"a":1}')
}

function propListNested(): void {
    let obj: NestedReplacerOuter = { outer: { inner: 'value' } }
    let replacer: (string | number)[] = ['outer']
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{"outer":{}}')
}

function propListDeepNested(): void {
    let obj: DeepNestedOuter = { a: { b: { c: 1 } } }
    let replacer: (string | number)[] = ['a', 'b', 'c']
    let str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{"a":{"b":{"c":1}}}')
}

function noReplacer1(): void {
    let obj: TwoOptionalNumberFields = { a: 1, b: 2 }
    let str: string = JSON.stringify(obj, undefined)
    arktest.assertEQ(str, '{"a":1,"b":2}')
}

function noReplacer2(): void {
    let obj: TwoOptionalNumberFields = { a: 1, b: 2 }
    let str: string = JSON.stringify(obj, undefined)
    arktest.assertEQ(str, '{"a":1,"b":2}')
}

function noReplacer3(): void {
    let obj: TwoOptionalNumberFields = { a: 1, b: 2 }
    let str: string = JSON.stringify(obj)
    arktest.assertEQ(str, '{"a":1,"b":2}')
}
