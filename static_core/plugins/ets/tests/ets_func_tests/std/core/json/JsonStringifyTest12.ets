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

interface EmptyObjectFields { }


interface TwoNumberFields {
    a: number
    b: number
}

interface NumberFlagFields {
    num: number
    flag: boolean
}

interface StringNumberTextFields {
    str: string
}

interface PersonFields {
    name: string
    age: number
}

interface UndefinedAndNumberFields {
    a: undefined,
    b: number
}

interface NullAndNumberFields {
    a: null
    b: number
}

interface KeyStringFields {
    key: string
}

interface WeatherStatusFields {
    temp: number
    status: string
}

interface ItemStringFields {
    item: string
}

interface QuoteStringFields {
    quote: string
}

interface SymbolStringFields {
    symbol: string
}

interface CountActiveFields {
    count: number
    active: boolean
}

interface CNumberFields {
    c: number
}

interface LineStringFields {
    line: string
}

interface NameAgeScoreFields {
    name: string
    age: number
    score: number
}

interface ProductStockFields {
    price: number
    inStock: boolean
}

interface NullDeepRootFields {
    a: NullDeepMiddleFields
}

interface NullDeepMiddleFields {
    b: null
    c: DNumberLeafFields
}

interface DNumberLeafFields {
    d: number
}

interface DescriptionStringFields {
    desc: string
}

interface LargeSmallNumberFields {
    largeNum: number
    smallNum: number
}

interface SingleANumberFields {
    a: number;
}

interface OptionalNestedNumberFields {
    x?: number;
    y?: number;
    arr?: number[];
    a?: OptionalNestedChildFields;
}

interface OptionalNestedChildFields {
    b?: OptionalNestedArrayFields;
}

interface OptionalNestedArrayFields {
    c?: number[];
}

interface ErrorFieldHolder {
    err: Error;
}

class EmptyClass { }

class StringOverrideToJSONClass implements JsonSerializable {
    foo: string = 'foo';
    toJSON(): string {
        return 'bar';
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

class OptionalPropClass {
    prop?: string;
}

class JsonStringifyNestedInner {
    innerProp: string = 'inner';
}

class JsonStringifyNestedOuter {
    outerProp: string = 'outer';
    inner: JsonStringifyNestedInner = new JsonStringifyNestedInner();
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

class NumberValueClass {
    value: number = 42;
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

class TwoLevelChild extends TwoLevelParent {
    childProp: string = 'child';
}

class ThreeLevelChild extends ThreeLevelParent {
    childProp: string = 'child';
}

class OverridePropChild extends OverridePropParent {
    prop: string = 'child';
}

class TwoLevelParent {
    parentProp: string = 'parent';
}

class ThreeLevelGrandparent {
    grandProp: string = 'grand';
}

class ThreeLevelParent extends ThreeLevelGrandparent {
    parentProp: string = 'parent';
}

class OverridePropParent {
    prop: string = 'parent';
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

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringify primitives, objects, and wrappers')
    suite.addTest('new Number(10)', number10)
    suite.addTest('new Map<...>(...)', mapA1)
    suite.addTest('object {num:-15, flag:true, label:hello}', objNumFlagLabel)
    suite.addTest('new Boolean(false)', booleanFalse)
    suite.addTest('{} as EmptyObjectFields', emptyEmptyObjectFields)
    suite.addTest('[null, 2, 3]', arrNull23)
    suite.addTest('new Date(2024-06-01T12:34:56+08:00)', date20240601T123456pos08)
    suite.addTest('[1, NaN, 3]', arr1NaN3)
    suite.addTest('[Infinity, 2, -Infinity]', arrInfinity2Infinity)
    suite.addTest('{ str: \'a"b\\nc\' } as S...', asObjStr)
    suite.addTest('new Set<number>([1, 2])', set12)
    suite.addTest('{ name: \'张三\', age: 30 ...', asObjNameAge)
    suite.addTest('object {a:3.14159, b:10} as TwoNum', twoNumPiAnd10)
    suite.addTest('object {a:undefined, b:5}', objWithUndefined)
    suite.addTest('new Date(2024-01-01T00:00:00.123Z)', date20240101T000000123Z)
    suite.addTest('object {a:null, b:5} as NullableObj', nullableObjNullAnd5)
    suite.addTest('new WeakMap<...>(...)', weakMapKeyVal)
    suite.addTest('[\'a\\\\b\', \'c\\\'d\']', arrABCD)
    suite.addTest('[\'张三\', \'李四\', 25]', arr25)
    suite.addTest('new Number(0)', number0)
    suite.addTest('object {temp:-12.5, status:active}', objTempStatus)
    suite.addTest('new Date(2024-02-29T00:00:00.000Z)', date20240229T000000000Z)
    suite.addTest('[1, [null, 3], 5]', array1)
    suite.addTest('new WeakSet<...>(...)', weakSetItemVal)
    suite.addTest('{ quote: \'He said "hi"...', asObjQuote)
    suite.addTest('{ symbol: \'!@#$%^\' } a...', asObjSymbol)
    suite.addTest('new Boolean(true)', booleanTrue)
    suite.addTest('object {count:100, active:true, names:[a,b,c]}', objCountActiveNames)
    suite.addTest('new Date(2024-05-01T18:00:00+08:00)', date20240501T180000pos08)
    suite.addTest('[undefined, 2, 3]', arrUndefined23)
    suite.addTest('array of NullableObj [{a:null,b:5},{a:null,b:5}]', arrNullableObjs)
    suite.addTest('{ line: \'first\\nsecond...', asObjLine1)
    suite.addTest('{ name: \'王五\', age: 28,...', asObjNameAgeScore)
    suite.addTest('object {price:99.99, inStock:true}', objPriceInStock)
    suite.addTest('new Date(2024-03-15T12:34:56.12...)', dateMarch15Precise)
    suite.addTest('nested object {a:{b:null,c:{d:[1,2,3]}}}', objNestedNullDeep)
    suite.addTest('{ line: \'line1\\rline2\'...', asObjLine2)
    suite.addTest('{ desc: \'测试：中文符号，。！\' }...', asObjDesc)
    suite.addTest('object {largeNum:123000, smallNum:0.000123}', objLargeSmallNums)
    suite.addTest('[NaN]', arrNaN)
    suite.addTest('SingleANumber {a:NaN}', singleANumberNaN)
    suite.addTest('object {a:Infinity, b:-Infinity}', objInfNegInf)
    suite.addTest('TwoNum {a:1, b:2}', twoNum1And2)
    suite.addTest('new Map<...>(...)', mapA1B)
    suite.addTest('new Set<number>([1, 2, 3])', set123)
    suite.addTest('new WeakMap<Object,...>()', weakMapObject)
    suite.addTest('new WeakSet<Object>()', weakSetObject)
    suite.addTest('SingleANumber {a:2}', singleANumber2)
    suite.addTest('{} as Record<string, never>', emptyRecord)
    suite.addTest('OptionalNested {x:5}', optionalNestedX5)
    suite.addTest('OptionalNested {x:5, y:6}', optionalNestedXY)
    suite.addTest('[1, \'false\', false] as...', arr1FalseFalseAsNumberS)
    suite.addTest('nested array [1, [2, 3], 4]', arrNestedMixed)
    suite.addTest('ArrayProp {arr:[1,2,3]}', arrayProp123)
    suite.addTest('array of OptionalNested [{x:1},{},{x:3}]', arrOptionalNested)
    suite.addTest('deeply nested {a:{b:{c:[1,2]}}}', objDeepNestedArray)
    suite.addTest('new Error(\'error\')', errorError)
    suite.addTest('[new Error(\'error\')]', arrNewErrorError)
    suite.addTest('{ err: new Error(\'erro...', asObjErr)
    suite.addTest('new Number(3)', number3)
    suite.addTest('new String(\'false\')', stringFalse)
    suite.addTest('\'foo\'', sFoo)
    suite.addTest('\'text\'', sText)
    suite.addTest('[1, undefined, 3]', arr1Undefined3)
    suite.addTest('[Infinity, -Infinity]', arrInfinityInfinity)
    suite.addTest('[1, 2, 3]', arr123)
    suite.addTest('new StringOverrideToJSONClass()', stringOverrideToJSONClass)
    suite.addTest('new ObjectLiteralTextToJSONClass()', objectLiteralTextToJSONClass)
    suite.addTest('new NumericTextToJSONClass()', numericTextToJSONClass)
    suite.addTest('new UndefinedTextToJSONClass()', undefinedTextToJSONClass)
    suite.addTest('new FunctionTextToJSONClass()', functionTextToJSONClass)
    suite.addTest('new ArrayTextToJSONClass()', arrayTextToJSONClass)
    suite.addTest('new SinglePropClass()', singlePropClass)
    suite.addTest('new ConstructorValueClass(42)', constructorValueClass42)
    suite.addTest('new PrimitiveFieldsClass()', primitiveFieldsClass)
    suite.addTest('new TwoLevelChild()', twoLevelChild)
    suite.addTest('new ThreeLevelChild()', threeLevelChild)
    suite.addTest('new OverridePropChild()', overridePropChild)
    suite.addTest('new PublicPropChild()', publicPropChild)
    suite.addTest('new PrivatePropChild()', privatePropChild)
    suite.addTest('new ProtectedPropChild()', protectedPropChild)
    suite.addTest('new InterfaceExtendsChild()', interfaceExtendsChild)
    suite.addTest('new ImplThenExtendChild()', implThenExtendChild)
    suite.addTest('new SingleFieldImpl()', singleFieldImpl)
    suite.addTest('new OptionalFieldImpl()', optionalFieldImpl)
    suite.addTest('new ReadonlyFieldImpl()', readonlyFieldImpl)
    suite.addTest('new MultiInterfaceImpl()', multiInterfaceImpl)
    suite.addTest('new OverlapInterfaceImpl()', overlapInterfaceImpl)
    suite.addTest('new InstanceMethodClass()', instanceMethodClass)
    suite.addTest('new PropAndMethodClass()', propAndMethodClass)
    suite.addTest('new StaticMethodClass()', staticMethodClass)
    suite.addTest('new GetterClass()', getterClass)
    suite.addTest('new SetterClass()', setterClass)
    suite.addTest('new GetterSetterClass()', getterSetterClass)
    suite.addTest('new PublicPropClass()', publicPropClass)
    suite.addTest('new PublicMethodClass()', publicMethodClass)
    suite.addTest('new NumberStringClass()', numberStringClass)
    suite.addTest('new PrivatePropClass()', privatePropClass)
    suite.addTest('new PrivateMethodClass()', privateMethodClass)
    suite.addTest('new PublicPrivatePropClass()', publicPrivatePropClass)
    suite.addTest('new ProtectedPropClass()', protectedPropClass)
    suite.addTest('new ProtectedMethodClass()', protectedMethodClass)
    suite.addTest('new PublicProtectedPropClass()', publicProtectedPropClass)
    suite.addTest('new ReadonlyPropClass()', readonlyPropClass)
    suite.addTest('new ReadonlyValueClass()', readonlyValueClass)
    suite.addTest('new PrivateReadonlyClass()', privateReadonlyClass)
    suite.addTest('new StaticPropClass()', staticPropClass)
    suite.addTest('new StaticMethodOnlyClass()', staticMethodOnlyClass)
    suite.addTest('new StaticAndInstanceClass()', staticAndInstanceClass)
    suite.addTest('new ConcreteSub()', concreteSub)
    suite.addTest('new GenericValueHolder<...>(...)', genericValueHolderTest)
    suite.addTest('new GenericValueHolder<...>(...)', genericValueHolder42)
    suite.addTest('GenericValueHolder with reference type', genericHolderRef)
    suite.addTest('new CustomToJSONClass()', customToJSONClass)
    suite.addTest('new InheritedToJSONChild()', inheritedToJSONChild)
    suite.addTest('new OverriddenToJSONChild()', overriddenToJSONChild)
    suite.addTest('new OptionalPropClass()', optionalPropClass)
    suite.addTest('new JsonStringifyNestedOuter()', jsonStringifyNestedOuter)
    suite.addTest('new ArrayContainer()', arrayContainer)
    suite.addTest('new MapContainer()', mapContainer)
    suite.addTest('new EmptyClass()', emptyClass)
    suite.addTest('new NumberValueClass()', numberValueClass)
    return suite.run()
}

function number10(): void {
    const result: string = JSON.stringify(new Number(10))
    arktest.assertEQ(result, '10')
}

function mapA1(): void {
    const result: string = JSON.stringify(new Map<string, number>([['a', 1]]))
    arktest.assertEQ(result, '{}')
}

function objNumFlagLabel(): void {
    const result: string = JSON.stringify({ num: -15, flag: true } as NumberFlagFields)
    arktest.assertEQ(result, '{"num":-15,"flag":true}')
}

function booleanFalse(): void {
    const result: string = JSON.stringify(new Boolean(false))
    arktest.assertEQ(result, 'false')
}

function emptyEmptyObjectFields(): void {
    const result: string = JSON.stringify({} as EmptyObjectFields)
    arktest.assertEQ(result, '{}')
}

function arrNull23(): void {
    const result: string = JSON.stringify([null, 2, 3])
    arktest.assertEQ(result, '[null,2,3]')
}

function date20240601T123456pos08(): void {
    const result: string = JSON.stringify(new Date('2024-06-01T12:34:56+08:00'))
    arktest.assertEQ(result, '"2024-06-01T04:34:56.000Z"')
}

function arr1NaN3(): void {
    const result: string = JSON.stringify([1, NaN, 3])
    arktest.assertEQ(result, '[1,null,3]')
}

function arrInfinity2Infinity(): void {
    const result: string = JSON.stringify([Infinity, 2, -Infinity])
    arktest.assertEQ(result, '[null,2,null]')
}

function asObjStr(): void {
    const result: string = JSON.stringify({ str: 'a"b\nc' } as StringNumberTextFields)
    arktest.assertEQ(result, '{"str":"a\\"b\\nc"}')
}

function set12(): void {
    const result: string = JSON.stringify(new Set<number>([1, 2]))
    arktest.assertEQ(result, '{}')
}

function asObjNameAge(): void {
    const result: string = JSON.stringify({ name: '张三', age: 30 } as PersonFields)
    arktest.assertEQ(result, '{"name":"张三","age":30}')
}

function twoNumPiAnd10(): void {
    const result: string = JSON.stringify({ a: 3.14159, b: 10 } as TwoNumberFields)
    arktest.assertEQ(result, '{"a":3.14159,"b":10}')
}

function objWithUndefined(): void {
    const result: string = JSON.stringify({ a: undefined, b: 5 } as UndefinedAndNumberFields)
    arktest.assertEQ(result, '{"b":5}')
}

function date20240101T000000123Z(): void {
    const result: string = JSON.stringify(new Date('2024-01-01T00:00:00.123Z'))
    arktest.assertEQ(result, '"2024-01-01T00:00:00.123Z"')
}

function nullableObjNullAnd5(): void {
    const result: string = JSON.stringify({ a: null, b: 5 } as NullAndNumberFields)
    arktest.assertEQ(result, '{"a":null,"b":5}')
}

function weakMapKeyVal(): void {
    const result: string = JSON.stringify(new WeakMap<KeyStringFields, number>([[{ key: 'val' } as KeyStringFields,
        10]]))
    arktest.assertEQ(result, '{}')
}

function arrABCD(): void {
    const result: string = JSON.stringify(['a\\b', 'c\'d'])
    arktest.assertEQ(result, '["a\\\\b","c\'d"]')
}

function arr25(): void {
    const result: string = JSON.stringify(['张三', '李四', 25])
    arktest.assertEQ(result, '["张三","李四",25]')
}

function number0(): void {
    const result: string = JSON.stringify(new Number(0))
    arktest.assertEQ(result, '0')
}

function objTempStatus(): void {
    const result: string = JSON.stringify({ temp: -12.5, status: 'ok' } as WeatherStatusFields)
    arktest.assertEQ(result, '{"temp":-12.5,"status":"ok"}')
}

function date20240229T000000000Z(): void {
    const result: string = JSON.stringify(new Date('2024-02-29T00:00:00.000Z'))
    arktest.assertEQ(result, '"2024-02-29T00:00:00.000Z"')
}

function array1(): void {
    const result: string = JSON.stringify([1, [null, 3], 5])
    arktest.assertEQ(result, '[1,[null,3],5]')
}

function weakSetItemVal(): void {
    const result: string = JSON.stringify(new WeakSet<ItemStringFields>([{ item: 'val' } as ItemStringFields]))
    arktest.assertEQ(result, '{}')
}

function asObjQuote(): void {
    const result: string = JSON.stringify({ quote: 'He said "hi"' } as QuoteStringFields)
    arktest.assertEQ(result, '{"quote":"He said \\"hi\\""}')
}

function asObjSymbol(): void {
    const result: string = JSON.stringify({ symbol: '!@#$%^' } as SymbolStringFields)
    arktest.assertEQ(result, '{"symbol":"!@#$%^"}')
}

function booleanTrue(): void {
    const result: string = JSON.stringify(new Boolean(true))
    arktest.assertEQ(result, 'true')
}

function objCountActiveNames(): void {
    const result: string = JSON.stringify({ count: 100, active: true } as CountActiveFields)
    arktest.assertEQ(result, '{"count":100,"active":true}')
}

function date20240501T180000pos08(): void {
    const result: string = JSON.stringify(new Date('2024-05-01T18:00:00+08:00'))
    arktest.assertEQ(result, '"2024-05-01T10:00:00.000Z"')
}

function arrUndefined23(): void {
    const result: string = JSON.stringify([undefined, 2, 3])
    arktest.assertEQ(result, '[null,2,3]')
}

function arrNullableObjs(): void {
    const result: string = JSON.stringify([{ a: null, b: 5 } as NullAndNumberFields, { c: 10 } as CNumberFields])
    arktest.assertEQ(result, '[{"a":null,"b":5},{"c":10}]')
}

function asObjLine1(): void {
    const result: string = JSON.stringify({ line: 'first\nsecond' } as LineStringFields)
    arktest.assertEQ(result, '{"line":"first\\nsecond"}')
}

function asObjNameAgeScore(): void {
    const result: string = JSON.stringify({ name: '王五', age: 28, score: 95.5 } as NameAgeScoreFields)
    arktest.assertEQ(result, '{"name":"王五","age":28,"score":95.5}')
}

function objPriceInStock(): void {
    const result: string = JSON.stringify({ price: 99.99, inStock: true } as ProductStockFields)
    arktest.assertEQ(result, '{"price":99.99,"inStock":true}')
}

function dateMarch15Precise(): void {
    const result: string = JSON.stringify(new Date('2024-03-15T12:34:56.123456Z'))
    arktest.assertEQ(result, '"2024-03-15T12:34:56.123Z"')
}

function objNestedNullDeep(): void {
    const result: string = JSON.stringify({ a: { b: null, c: { d: 5 } } } as NullDeepRootFields)
    arktest.assertEQ(result, '{"a":{"b":null,"c":{"d":5}}}')
}

function asObjLine2(): void {
    const result: string = JSON.stringify({ line: 'line1\rline2' } as LineStringFields)
    arktest.assertEQ(result, '{"line":"line1\\rline2"}')
}

function asObjDesc(): void {
    const result: string = JSON.stringify({ desc: '测试：中文符号，。！' } as DescriptionStringFields)
    arktest.assertEQ(result, '{"desc":"测试：中文符号，。！"}')
}

function objLargeSmallNums(): void {
    const result: string = JSON.stringify({ largeNum: 1.23e5, smallNum: 4.5e-2 } as LargeSmallNumberFields)
    arktest.assertEQ(result, '{"largeNum":123000,"smallNum":0.045}')
}

function arrNaN(): void {
    const result: string = JSON.stringify([NaN])
    arktest.assertEQ(result, '[null]')
}

function singleANumberNaN(): void {
    const result: string = JSON.stringify({ a: NaN } as SingleANumberFields)
    arktest.assertEQ(result, '{"a":null}')
}

function objInfNegInf(): void {
    const result: string = JSON.stringify({ a: Infinity, b: -Infinity } as TwoNumberFields)
    arktest.assertEQ(result, '{"a":null,"b":null}')
}

function twoNum1And2(): void {
    const result: string = JSON.stringify({ a: 1, b: 2 } as TwoNumberFields)
    arktest.assertEQ(result, '{"a":1,"b":2}')
}

function mapA1B(): void {
    const result: string = JSON.stringify(new Map<string, number>([['a', 1], ['b', 2]]))
    arktest.assertEQ(result, '{}')
}

function set123(): void {
    const result: string = JSON.stringify(new Set<number>([1, 2, 3]))
    arktest.assertEQ(result, '{}')
}

function weakMapObject(): void {
    const result: string = JSON.stringify(new WeakMap<Object, Object>())
    arktest.assertEQ(result, '{}')
}

function weakSetObject(): void {
    const result: string = JSON.stringify(new WeakSet<Object>())
    arktest.assertEQ(result, '{}')
}

function singleANumber2(): void {
    const result: string = JSON.stringify({ a: 2 } as SingleANumberFields)
    arktest.assertEQ(result, '{"a":2}')
}

function emptyRecord(): void {
    const result: string = JSON.stringify({} as Record<string, never>)
    arktest.assertEQ(result, '{}')
}

function optionalNestedX5(): void {
    const result: string = JSON.stringify({ x: 5 } as OptionalNestedNumberFields)
    arktest.assertEQ(result, '{"x":5}')
}

function optionalNestedXY(): void {
    const result: string = JSON.stringify({ x: 5, y: 6 } as OptionalNestedNumberFields)
    arktest.assertEQ(result, '{"x":5,"y":6}')
}

function arr1FalseFalseAsNumberS(): void {
    const result: string = JSON.stringify([1, 'false', false] as (number | string | boolean)[])
    arktest.assertEQ(result, '[1,"false",false]')
}

function arrNestedMixed(): void {
    const result: string = JSON.stringify([1, [2, 3], 4] as (number | number[])[])
    arktest.assertEQ(result, '[1,[2,3],4]')
}

function arrayProp123(): void {
    const result: string = JSON.stringify({ arr: [1, 2, 3] } as OptionalNestedNumberFields)
    arktest.assertEQ(result, '{"arr":[1,2,3]}')
}

function arrOptionalNested(): void {
    const result: string = JSON.stringify([{ x: 1 } as OptionalNestedNumberFields,
        { y: 2 } as OptionalNestedNumberFields])
    arktest.assertEQ(result, '[{"x":1},{"y":2}]')
}

function objDeepNestedArray(): void {
    const result: string = JSON.stringify({ a: { b: { c: [1, 2] } } } as OptionalNestedNumberFields)
    arktest.assertEQ(result, '{"a":{"b":{"c":[1,2]}}}')
}

function errorError(): void {
    const result: string = JSON.stringify(new Error('error'))
    arktest.assertEQ(result, '{}')
}

function arrNewErrorError(): void {
    const result: string = JSON.stringify([new Error('error')])
    arktest.assertEQ(result, '[{}]')
}

function asObjErr(): void {
    const result: string = JSON.stringify({ err: new Error('error') } as ErrorFieldHolder)
    arktest.assertEQ(result, '{"err":{}}')
}

function number3(): void {
    const result: string = JSON.stringify(new Number(3))
    arktest.assertEQ(result, '3')
}

function stringFalse(): void {
    const result: string = JSON.stringify(new String('false'))
    arktest.assertEQ(result, '"false"')
}

function sFoo(): void {
    const result: string = JSON.stringify('foo')
    arktest.assertEQ(result, '"foo"')
}

function sText(): void {
    const result: string = JSON.stringify('text')
    arktest.assertEQ(result, '"text"')
}

function arr1Undefined3(): void {
    const result: string = JSON.stringify([1, undefined, 3])
    arktest.assertEQ(result, '[1,null,3]')
}

function arrInfinityInfinity(): void {
    const result: string = JSON.stringify([Infinity, -Infinity])
    arktest.assertEQ(result, '[null,null]')
}

function arr123(): void {
    const result: string = JSON.stringify([1, 2, 3])
    arktest.assertEQ(result, '[1,2,3]')
}

function stringOverrideToJSONClass(): void {
    const result: string = JSON.stringify(new StringOverrideToJSONClass())
    arktest.assertEQ(result, '"bar"')
}

function objectLiteralTextToJSONClass(): void {
    const result: string = JSON.stringify(new ObjectLiteralTextToJSONClass())
    arktest.assertEQ(result, '"{ b: 2 }"')
}

function numericTextToJSONClass(): void {
    const result: string = JSON.stringify(new NumericTextToJSONClass())
    arktest.assertEQ(result, '"42"')
}

function undefinedTextToJSONClass(): void {
    const result: string = JSON.stringify(new UndefinedTextToJSONClass())
    arktest.assertEQ(result, '"undefined"')
}

function functionTextToJSONClass(): void {
    const result: string = JSON.stringify(new FunctionTextToJSONClass())
    arktest.assertEQ(result, '"() => {}"')
}

function arrayTextToJSONClass(): void {
    const result: string = JSON.stringify(new ArrayTextToJSONClass())
    arktest.assertEQ(result, '"[1,2,3]"')
}

function singlePropClass(): void {
    const result: string = JSON.stringify(new SinglePropClass())
    arktest.assertEQ(result, '{"name":"test"}')
}

function constructorValueClass42(): void {
    const result: string = JSON.stringify(new ConstructorValueClass(42))
    arktest.assertEQ(result, '{"value":42}')
}

function primitiveFieldsClass(): void {
    const result: string = JSON.stringify(new PrimitiveFieldsClass())
    arktest.assertEQ(result, '{"a":1,"b":"test","c":true}')
}

function twoLevelChild(): void {
    const result: string = JSON.stringify(new TwoLevelChild())
    arktest.assertEQ(result, '{"parentProp":"parent","childProp":"child"}')
}

function threeLevelChild(): void {
    const result: string = JSON.stringify(new ThreeLevelChild())
    arktest.assertEQ(result, '{"grandProp":"grand","parentProp":"parent","childProp":"child"}')
}

function overridePropChild(): void {
    const result: string = JSON.stringify(new OverridePropChild())
    arktest.assertEQ(result, '{"prop":"child"}')
}

function publicPropChild(): void {
    const result: string = JSON.stringify(new PublicPropChild())
    arktest.assertEQ(result, '{"publicProp":"public"}')
}

function privatePropChild(): void {
    const result: string = JSON.stringify(new PrivatePropChild())
    arktest.assertEQ(result, '{}')
}

function protectedPropChild(): void {
    const result: string = JSON.stringify(new ProtectedPropChild())
    arktest.assertEQ(result, '{}')
}

function interfaceExtendsChild(): void {
    const result: string = JSON.stringify(new InterfaceExtendsChild())
    arktest.assertEQ(result, '{"parentProp":"parent","interfaceProp":"interface"}')
}

function implThenExtendChild(): void {
    const result: string = JSON.stringify(new ImplThenExtendChild())
    arktest.assertEQ(result, '{"interfaceProp":"interface","childProp":"child"}')
}

function singleFieldImpl(): void {
    const result: string = JSON.stringify(new SingleFieldImpl())
    arktest.assertEQ(result, '{"prop":"value"}')
}

function optionalFieldImpl(): void {
    const result: string = JSON.stringify(new OptionalFieldImpl())
    arktest.assertEQ(result, '{"required":"value","optional":42}')
}

function readonlyFieldImpl(): void {
    const result: string = JSON.stringify(new ReadonlyFieldImpl())
    arktest.assertEQ(result, '{"readonlyProp":"value"}')
}

function multiInterfaceImpl(): void {
    const result: string = JSON.stringify(new MultiInterfaceImpl())
    arktest.assertEQ(result, '{"prop1":"value1","prop2":42}')
}

function overlapInterfaceImpl(): void {
    const result: string = JSON.stringify(new OverlapInterfaceImpl())
    arktest.assertEQ(result, '{"prop":"value"}')
}

function instanceMethodClass(): void {
    const result: string = JSON.stringify(new InstanceMethodClass())
    arktest.assertEQ(result, '{}')
}

function propAndMethodClass(): void {
    const result: string = JSON.stringify(new PropAndMethodClass())
    arktest.assertEQ(result, '{"prop":"value"}')
}

function staticMethodClass(): void {
    const result: string = JSON.stringify(new StaticMethodClass())
    arktest.assertEQ(result, '{}')
}

function getterClass(): void {
    const result: string = JSON.stringify(new GetterClass())
    arktest.assertEQ(result, '{}')
}

function setterClass(): void {
    const result: string = JSON.stringify(new SetterClass())
    arktest.assertEQ(result, '{}')
}

function getterSetterClass(): void {
    const result: string = JSON.stringify(new GetterSetterClass())
    arktest.assertEQ(result, '{}')
}

function publicPropClass(): void {
    const result: string = JSON.stringify(new PublicPropClass())
    arktest.assertEQ(result, '{"publicProp":"public"}')
}

function publicMethodClass(): void {
    const result: string = JSON.stringify(new PublicMethodClass())
    arktest.assertEQ(result, '{}')
}

function numberStringClass(): void {
    const result: string = JSON.stringify(new NumberStringClass())
    arktest.assertEQ(result, '{"a":1,"b":"test"}')
}

function privatePropClass(): void {
    const result: string = JSON.stringify(new PrivatePropClass())
    arktest.assertEQ(result, '{}')
}

function privateMethodClass(): void {
    const result: string = JSON.stringify(new PrivateMethodClass())
    arktest.assertEQ(result, '{}')
}

function publicPrivatePropClass(): void {
    const result: string = JSON.stringify(new PublicPrivatePropClass())
    arktest.assertEQ(result, '{"publicProp":"public"}')
}

function protectedPropClass(): void {
    const result: string = JSON.stringify(new ProtectedPropClass())
    arktest.assertEQ(result, '{}')
}

function protectedMethodClass(): void {
    const result: string = JSON.stringify(new ProtectedMethodClass())
    arktest.assertEQ(result, '{}')
}

function publicProtectedPropClass(): void {
    const result: string = JSON.stringify(new PublicProtectedPropClass())
    arktest.assertEQ(result, '{"publicProp":"public"}')
}

function readonlyPropClass(): void {
    const result: string = JSON.stringify(new ReadonlyPropClass())
    arktest.assertEQ(result, '{"readonlyProp":"readonly"}')
}

function readonlyValueClass(): void {
    const result: string = JSON.stringify(new ReadonlyValueClass())
    arktest.assertEQ(result, '{"prop":"value"}')
}

function privateReadonlyClass(): void {
    const result: string = JSON.stringify(new PrivateReadonlyClass())
    arktest.assertEQ(result, '{}')
}

function staticPropClass(): void {
    const result: string = JSON.stringify(new StaticPropClass())
    arktest.assertEQ(result, '{}')
}

function staticMethodOnlyClass(): void {
    const result: string = JSON.stringify(new StaticMethodOnlyClass())
    arktest.assertEQ(result, '{}')
}

function staticAndInstanceClass(): void {
    const result: string = JSON.stringify(new StaticAndInstanceClass())
    arktest.assertEQ(result, '{"instanceProp":"instance"}')
}

function concreteSub(): void {
    const result: string = JSON.stringify(new ConcreteSub())
    arktest.assertEQ(result, '{"abstractProp":"abstract","concreteProp":"concrete"}')
}

function genericValueHolderTest(): void {
    const result: string = JSON.stringify(new GenericValueHolder<string>('test'))
    arktest.assertEQ(result, '{"value":"test"}')
}

function genericValueHolder42(): void {
    const result: string = JSON.stringify(new GenericValueHolder<number>(42))
    arktest.assertEQ(result, '{"value":42}')
}

function genericHolderRef(): void {
    const result: string = JSON.stringify(new GenericValueHolder<Record<string, number>>({ 'a': 1 }))
    arktest.assertEQ(result, '{"value":{"a":1}}')
}

function customToJSONClass(): void {
    const result: string = JSON.stringify(new CustomToJSONClass())
    arktest.assertEQ(result, '"custom"')
}

function inheritedToJSONChild(): void {
    const result: string = JSON.stringify(new InheritedToJSONChild())
    arktest.assertEQ(result, '"parent"')
}

function overriddenToJSONChild(): void {
    const result: string = JSON.stringify(new OverriddenToJSONChild())
    arktest.assertEQ(result, '"child"')
}

function optionalPropClass(): void {
    const result: string = JSON.stringify(new OptionalPropClass())
    arktest.assertEQ(result, '{}')
}

function jsonStringifyNestedOuter(): void {
    const result: string = JSON.stringify(new JsonStringifyNestedOuter())
    arktest.assertEQ(result, '{"outerProp":"outer","inner":{"innerProp":"inner"}}')
}

function arrayContainer(): void {
    const result: string = JSON.stringify(new ArrayContainer())
    arktest.assertEQ(result, '{"items":[{"value":0},{"value":0}]}')
}

function mapContainer(): void {
    const result: string = JSON.stringify(new MapContainer())
    arktest.assertEQ(result, '{"itemMap":{"a":{"value":0},"b":{"value":0}}}')
}

function emptyClass(): void {
    const result: string = JSON.stringify(new EmptyClass())
    arktest.assertEQ(result, '{}')
}

function numberValueClass(): void {
    const result: string = JSON.stringify(new NumberValueClass())
    arktest.assertEQ(result, '{"value":42}')
}
