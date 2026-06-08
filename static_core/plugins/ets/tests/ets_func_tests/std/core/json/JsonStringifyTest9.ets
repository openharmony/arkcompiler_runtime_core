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
    const suite = new arktest.ArkTestsuite('JSON.stringify property-list and edge-case tests')
    suite.addTest('basic object stringify', basicObjStringify)
    suite.addTest('nested object stringify', nestedObjStringify)
    suite.addTest('mixed array stringify', mixedArrayStringify)
    suite.addTest('user list stringify', userListStringify)
    suite.addTest('replacer selects keys', replacerSelectKeys)
    suite.addTest('replacer numeric keys', replacerNumericKeys)
    suite.addTest('replacer mixed key types', replacerMixedKeyTypes)
    suite.addTest('replacer empty list', replacerEmptyList)
    suite.addTest('replacer with indent', replacerWithIndent)
    suite.addTest('replacer with tab space', replacerWithTabSpace)
    suite.addTest('replacer with undefined space', replacerUndefSpace)
    suite.addTest('replacer filters numeric keys', replacerFilterNumericKeys)
    suite.addTest('replacer large indent', replacerLargeIndent)
    suite.addTest('replacer NaN/Infinity keys', replacerNaNInfKeys)
    suite.addTest('replacer duplicate keys', replacerDupKeys)
    suite.addTest('replacer shallow nested', replacerShallowNested)
    suite.addTest('replacer zero space indent', replacerZeroSpace)
    suite.addTest('replacer nonexistent key', replacerNonexistKey)
    suite.addTest('replacer numeric key filter', replacerFilterNumeric2)
    suite.addTest('replacer null property', replacerNullProp)
    suite.addTest('replacer selects keys 2', replacerSelectKeys2)
    suite.addTest('replacer numeric keys 2', replacerNumericKeys2)
    suite.addTest('replacer empty list 2', replacerEmptyList2)
    suite.addTest('replacer with indent 2', replacerWithIndent2)
    suite.addTest('replacer with tab 2', replacerWithTab2)
    suite.addTest('replacer undefined space 2', replacerUndef2)
    suite.addTest('replacer large indent 2', replacerLargeIndent2)
    suite.addTest('replacer zero space 2', replacerZeroSpace2)
    suite.addTest('replacer shallow 2', replacerShallow2)
    suite.addTest('replacer nonexistent 2', replacerNonexist2)
    suite.addTest('replacer duplicate keys 2', replacerDupKeys2)
    suite.addTest('replacer skips users', replacerSkipUserList)
    suite.addTest('replacer numeric keys 3', replacerNumericKeys3)
    suite.addTest('replacer FixedArray select keys', replacerFixedSelectKeys)
    suite.addTest('replacer FixedArray mixed keys', replacerFixedMixedKeys)
    suite.addTest('replacer FixedArray empty', replacerFixedEmpty)
    suite.addTest('replacer FixedArray indent', replacerFixedIndent)
    suite.addTest('replacer FixedArray tab', replacerFixedTab)
    suite.addTest('replacer FixedArray zero space', replacerFixedZero)
    suite.addTest('replacer FixedArray large space', replacerFixedLarge)
    suite.addTest('replacer FixedArray undef space', replacerFixedUndef)
    suite.addTest('replacer FixedArray shallow', replacerFixedShallow)
    suite.addTest('replacer FixedArray duplicates', replacerFixedDup)
    suite.addTest('replacer FixedArray missing key', replacerFixedMissing)
    suite.addTest('large int boundary', largeIntEdge)
    suite.addTest('large char array stringify', largeCharArray)
    suite.addTest('SimpleObj stringify', simpleObjStr)
    suite.addTest('SimpleObj undef replacer', simpleObjUndefReplacer)
    suite.addTest('SimpleObj pretty print', simpleObjPrettyPrint)
    suite.addTest('BooleanObj pretty print', booleanObjPrettyPrint)
    suite.addTest('SimpleObj tab indent', simpleObjTabIndent)
    suite.addTest('SimpleObj 15-char indent', simpleObjLargeIndent)
    suite.addTest('SimpleObj zero indent', simpleObjZeroIndent)
    suite.addTest('SimpleObj empty string indent', simpleObjEmptyStrIndent)
    suite.addTest('byte values stringify', byteStringify)
    suite.addTest('byte validation set', byteValidation)
    suite.addTest('short values stringify', shortStringify)
    suite.addTest('short validation set', shortValidation)
    suite.addTest('int values stringify', intStringify)
    suite.addTest('int negative sign check', intNegative)
    suite.addTest('int positive sign check', intPositive)
    suite.addTest('bigint values stringify', bigintStringify)
    suite.addTest('bigint validation set', bigintValidation)
    suite.addTest('bigint negative sign', bigintNegative)
    suite.addTest('bigint positive sign', bigintPositive)
    suite.addTest('bigint large values stringify', bigintLargeStringify)
    suite.addTest('bigint large validation', bigintLargeValidation)
    suite.addTest('ASCII char stringify', asciiCharStringify)
    suite.addTest('large deep-nested string', largeDeepNestedStr)
    suite.addTest('large number array', largeNumArray)
    suite.addTest('large object stringify', largeObjStr)
    suite.addTest('JSONPath symbols round-trip', jsonPathSymbols)
    suite.addTest('multilingual punct reviver', multiLangPunctReviver)
    suite.addTest('reviver char count', reviverCharCount)
    suite.addTest('reviver string replace', reviverStringReplace)
    suite.addTest('circular reference self', circularRefSelf)
    suite.addTest('circular reference mutual', circularRefMutual)
    suite.addTest('circular reference triple chain', circularRefTriple)
    suite.addTest('circular reference via array', circularRefObjArr)
    suite.addTest('circular reference self 2', circularRefSelf2)
    suite.addTest('key order numeric', keyOrderNumeric)
    suite.addTest('key order mixed numeric/string', keyOrderMixed)
    suite.addTest('key order array-like', keyOrderArrayLike)
    suite.addTest('key order array-like no length', keyOrderArrayLikeNoLen)
    return suite.run()
}

class LocalizedPunctuationMessages {
    en: string = '';
    fr: string = '';
    zh: string = '';
}

class SimpleObj {
    name: string = '';
    year: number = 0;
    value: number = 0;
}

class BooleanObj {
    boolVal: boolean = true;
}

class MultiLangPunctObj {
    messages: LocalizedPunctuationMessages = new LocalizedPunctuationMessages();
}
class CharCount {
    text: string = '';
    count: number;
}
class StringReplace {
    text: string = '';
    replaced: string = '';
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

function basicObjStringify(): void {
    let name: Object = 'test' as Object
    let value: Object = 123 as Object
    let active: Object = true as Object
    let val: Record<string, Object> = {}
    val['name'] = name
    val['value'] = value
    val['active'] = active
    const str: string = JSON.stringify(val)
    arktest.assertTrue(str.startsWith('{'))
    arktest.assertTrue(str.endsWith('}'))
    arktest.assertTrue(str.indexOf('"name"') >= 0)
    arktest.assertTrue(str.indexOf('"value"') >= 0)
}

function nestedObjStringify(): void {
    let e: Object = 3 as Object
    let c: Object = 2 as Object
    let a: Object = 1 as Object
    let inner: Record<string, Object> = {}
    inner['e'] = e
    let mid: Record<string, Object> = {}
    mid['c'] = c
    mid['d'] = inner as Object
    let val: Record<string, Object> = {}
    val['a'] = a
    val['b'] = mid as Object
    const str: string = JSON.stringify(val)
    arktest.assertTrue(str.indexOf('"b"') >= 0)
    arktest.assertTrue(str.indexOf('"c"') >= 0)
    arktest.assertTrue(str.indexOf('"d"') >= 0)
}

function mixedArrayStringify(): void {
    let val: Array<Object | null> = []
    val.push(1 as Object)
    val.push('string' as Object)
    val.push(true as Object)
    val.push(null)
    const str: string = JSON.stringify(val)
    arktest.assertTrue(str.startsWith('['))
    arktest.assertTrue(str.endsWith(']'))
    arktest.assertEQ(str.split(',').length, 4)
}

function userListStringify(): void {
    let id1: Object = 1 as Object
    let name1: Object = 'Alice' as Object
    let id2: Object = 2 as Object
    let name2: Object = 'Bob' as Object
    let count: Object = 2 as Object
    let valid: Object = true as Object

    let user1: Record<string, Object> = {}
    user1['id'] = id1
    user1['name'] = name1
    let user2: Record<string, Object> = {}
    user2['id'] = id2
    user2['name'] = name2

    let users: Array<Object> = []
    users.push(user1)
    users.push(user2)

    let val: Record<string, Object> = {}
    val['users'] = users as Object
    val['count'] = count
    val['valid'] = valid

    const str: string = JSON.stringify(val)
    arktest.assertTrue(str.startsWith('{'))
    arktest.assertTrue(str.endsWith('}'))
    arktest.assertTrue(str.indexOf('"users"') >= 0)
    arktest.assertTrue(str.indexOf('[') >= 0)
    arktest.assertTrue(str.indexOf(']') >= 0)
}

function replacerSelectKeys(): void {
    let obj: Record<string, Object> = {}
    obj['name'] = 'Alice' as Object
    obj['age'] = 25 as Object
    obj['city'] = 'Beijing' as Object

    let replacer: Array<string> = ['name', 'age']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"name"') >= 0)
    arktest.assertTrue(str.indexOf('"age"') >= 0)
    arktest.assertTrue(str.indexOf('"city"') < 0)
}

function replacerNumericKeys(): void {
    let obj: Record<string, Object> = {}
    obj['0'] = 'first' as Object
    obj['1'] = 'second' as Object
    obj['2'] = 'third' as Object

    let replacer: Array<double | string> = [0 as double, 2 as double]
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"0"') >= 0)
    arktest.assertTrue(str.indexOf('"2"') >= 0)
    arktest.assertTrue(str.indexOf('"1"') < 0)
}

function replacerMixedKeyTypes(): void {
    let obj: Record<string, Object> = {}
    obj['0'] = 'zero' as Object
    obj['2'] = 'two' as Object
    obj['length'] = 5 as Object

    let replacer: Array<double | string> = [0 as double, 2 as double, 'length' as string]
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.startsWith('{'))
    arktest.assertTrue(str.indexOf('length') >= 0)
    arktest.assertTrue(str.indexOf('zero') >= 0 || str.indexOf('two') >= 0)
}

function replacerEmptyList(): void {
    let obj: Record<string, Object> = {}
    obj['a'] = 1 as Object
    obj['b'] = 2 as Object

    let replacer: Array<string> = []
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{}')
}

function replacerWithIndent(): void {
    let obj: Record<string, Object> = {}
    obj['x'] = 1 as Object

    let replacer: Array<string> = ['x']
    let space: int = 2
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf('\n') >= 0)
    arktest.assertTrue(str.indexOf('  ') >= 0)
}

function replacerWithTabSpace(): void {
    let obj: Record<string, Object> = {}
    obj['y'] = 'test' as Object

    let replacer: Array<string> = ['y']
    let space: string = '\t'
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf('\t') >= 0)
}

function replacerUndefSpace(): void {
    let obj: Record<string, Object> = {}
    obj['z'] = 123 as Object

    let replacer: Array<string> = ['z']
    const str: string = JSON.stringify(obj, replacer, undefined)
    arktest.assertTrue(str.indexOf('\n') < 0)
    arktest.assertEQ(str, '{"z":123}')
}

function replacerFilterNumericKeys(): void {
    let obj: Record<string, Object> = {}
    obj['0'] = 'zero' as Object
    obj['1'] = 'one' as Object
    obj['2'] = 'two' as Object

    let replacer: Array<string> = ['0', '2']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('zero') >= 0)
    arktest.assertTrue(str.indexOf('two') >= 0)
    arktest.assertTrue(str.indexOf('one') < 0)
}

function replacerLargeIndent(): void {
    let obj: Record<string, Object> = {}
    obj['key'] = 'value' as Object

    let replacer: Array<string> = ['key']
    let space: int = 20
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf('          ') >= 0)
}

function replacerNaNInfKeys(): void {
    let obj: Record<string, Object> = {}
    obj['0'] = 'a' as Object
    obj['1'] = 'b' as Object

    let replacer: Array<double | string> = [0 as double, NaN as double, Infinity as double]
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"0"') >= 0)
}

function replacerDupKeys(): void {
    let obj: Record<string, Object> = {}
    obj['name'] = 'test' as Object
    obj['value'] = 100 as Object

    let replacer: Array<string> = ['name', 'name', 'value']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"name"') >= 0)
    arktest.assertTrue(str.indexOf('"value"') >= 0)
}

function replacerShallowNested(): void {
    let inner: Record<string, Object> = {}
    inner['x'] = 1 as Object
    inner['y'] = 2 as Object
    let obj: Record<string, Object> = {}
    obj['a'] = 'outer' as Object
    obj['inner'] = inner as Object

    let replacer: Array<string> = ['a']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"a"') >= 0)
    arktest.assertTrue(str.indexOf('"inner"') < 0)
}

function replacerZeroSpace(): void {
    let obj: Record<string, Object> = {}
    obj['m'] = 1 as Object
    obj['n'] = 2 as Object

    let replacer: Array<string> = ['m', 'n']
    let space: int = 0
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf(' ') < 0)
    arktest.assertEQ(str, '{"m":1,"n":2}')
}

function replacerNonexistKey(): void {
    let obj: Record<string, Object> = {}
    obj['exists'] = 'yes' as Object

    let replacer: Array<string> = ['exists', 'nonexistent']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"exists"') >= 0)
    arktest.assertTrue(str.indexOf('"nonexistent"') < 0)
}

function replacerFilterNumeric2(): void {
    let obj: Record<string, Object> = {}
    obj['0'] = 'a' as Object
    obj['1'] = 'b' as Object
    obj['2'] = 'c' as Object

    let replacer: Array<string> = ['0', '2']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('a') >= 0)
    arktest.assertTrue(str.indexOf('c') >= 0)
    arktest.assertTrue(str.indexOf('b') < 0)
}

function replacerNullProp(): void {
    let obj: Record<string, Object | null> = {}
    obj['valid'] = 'value' as Object
    obj['nullProp'] = null

    let replacer: Array<string> = ['valid', 'nullProp']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"valid"') >= 0)
    arktest.assertTrue(str.indexOf('null') >= 0)
}

function replacerSelectKeys2(): void {
    let obj: Record<string, Object> = {}
    obj['name'] = 'Alice' as Object
    obj['age'] = 25 as Object
    obj['city'] = 'Beijing' as Object

    let replacer: Array<string> = ['name', 'age']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"name"') >= 0)
    arktest.assertTrue(str.indexOf('"age"') >= 0)
    arktest.assertTrue(str.indexOf('"city"') < 0)
}

function replacerNumericKeys2(): void {
    let obj: Record<string, Object> = {}
    obj['0'] = 'zero' as Object
    obj['1'] = 'one' as Object
    obj['2'] = 'two' as Object

    let replacer: Array<string> = ['0', '2']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('zero') >= 0)
    arktest.assertTrue(str.indexOf('two') >= 0)
    arktest.assertTrue(str.indexOf('one') < 0)
}

function replacerEmptyList2(): void {
    let obj: Record<string, Object> = {}
    obj['a'] = 1 as Object
    obj['b'] = 2 as Object

    let replacer: Array<string> = []
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{}')
}

function replacerWithIndent2(): void {
    let obj: Record<string, Object> = {}
    obj['x'] = 1 as Object

    let replacer: Array<string> = ['x']
    let space: int = 2
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf('\n') >= 0)
    arktest.assertTrue(str.indexOf('  ') >= 0)
}

function replacerWithTab2(): void {
    let obj: Record<string, Object> = {}
    obj['y'] = 'test' as Object

    let replacer: Array<string> = ['y']
    let space: string = '\t'
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf('\t') >= 0)
}

function replacerUndef2(): void {
    let obj: Record<string, Object> = {}
    obj['z'] = 123 as Object

    let replacer: Array<string> = ['z']
    const str: string = JSON.stringify(obj, replacer, undefined)
    arktest.assertTrue(str.indexOf('\n') < 0)
    arktest.assertEQ(str, '{"z":123}')
}

function replacerLargeIndent2(): void {
    let obj: Record<string, Object> = {}
    obj['key'] = 'value' as Object

    let replacer: Array<string> = ['key']
    let space: int = 20
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf('          ') >= 0)
}

function replacerZeroSpace2(): void {
    let obj: Record<string, Object> = {}
    obj['m'] = 1 as Object
    obj['n'] = 2 as Object

    let replacer: Array<string> = ['m', 'n']
    let space: int = 0
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf(' ') < 0)
    arktest.assertEQ(str, '{"m":1,"n":2}')
}

function replacerShallow2(): void {
    let inner: Record<string, Object> = {}
    inner['x'] = 1 as Object
    inner['y'] = 2 as Object
    let obj: Record<string, Object> = {}
    obj['a'] = 'outer' as Object
    obj['inner'] = inner as Object

    let replacer: Array<string> = ['a']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"a"') >= 0)
    arktest.assertTrue(str.indexOf('"inner"') < 0)
}

function replacerNonexist2(): void {
    let obj: Record<string, Object> = {}
    obj['exists'] = 'yes' as Object

    let replacer: Array<string> = ['exists', 'nonexistent']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"exists"') >= 0)
    arktest.assertTrue(str.indexOf('"nonexistent"') < 0)
}

function replacerDupKeys2(): void {
    let obj: Record<string, Object> = {}
    obj['name'] = 'test' as Object
    obj['value'] = 100 as Object

    let replacer: Array<string> = ['name', 'name', 'value']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"name"') >= 0)
    arktest.assertTrue(str.indexOf('"value"') >= 0)
}

function replacerSkipUserList(): void {
    let user1: Record<string, Object> = {}
    user1['id'] = 1 as Object
    user1['name'] = 'Alice' as Object

    let users: Array<Object> = [user1 as Object]
    let obj: Record<string, Object> = {}
    obj['users'] = users as Object
    obj['total'] = 1 as Object

    let replacer: Array<string> = ['total']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"total"') >= 0)
    arktest.assertTrue(str.indexOf('"users"') < 0)
}

function replacerNumericKeys3(): void {
    let obj: Record<string, Object> = {}
    obj['0'] = 'first' as Object
    obj['1'] = 'second' as Object
    obj['2'] = 'third' as Object

    let replacer: Array<string> = ['0', '2']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('first') >= 0)
    arktest.assertTrue(str.indexOf('third') >= 0)
    arktest.assertTrue(str.indexOf('second') < 0)
}

function replacerFixedSelectKeys(): void {
    let obj: Record<string, Object> = {}
    obj['name'] = 'Alice' as Object
    obj['age'] = 25 as Object
    obj['city'] = 'Beijing' as Object

    let replacer: FixedArray<string> = ['name', 'age']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"name"') >= 0)
    arktest.assertTrue(str.indexOf('"age"') >= 0)
    arktest.assertTrue(str.indexOf('"city"') < 0)
}

function replacerFixedMixedKeys(): void {
    let obj: Record<string, Object> = {}
    obj['0'] = 'zero' as Object
    obj['2'] = 'two' as Object
    obj['count'] = 5 as Object

    let replacer: FixedArray<double | string> = [0 as double, 2 as double, 'count']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('zero') >= 0)
    arktest.assertTrue(str.indexOf('count') >= 0)
    arktest.assertTrue(str.indexOf('two') >= 0)
}

function replacerFixedEmpty(): void {
    let obj: Record<string, Object> = {}
    obj['a'] = 1 as Object
    obj['b'] = 2 as Object

    let replacer: FixedArray<string> = []
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertEQ(str, '{}')
}

function replacerFixedIndent(): void {
    let obj: Record<string, Object> = {}
    obj['key'] = 'value' as Object

    let replacer: FixedArray<string> = ['key']
    let space: int = 2
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf('\n') >= 0)
    arktest.assertTrue(str.indexOf('  ') >= 0)
}

function replacerFixedTab(): void {
    let obj: Record<string, Object> = {}
    obj['data'] = 'test' as Object

    let replacer: FixedArray<string> = ['data']
    let space: string = '\t'
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf('\t') >= 0)
}

function replacerFixedZero(): void {
    let obj: Record<string, Object> = {}
    obj['m'] = 1 as Object
    obj['n'] = 2 as Object

    let replacer: FixedArray<string> = ['m', 'n']
    let space: int = 0
    const str: string = JSON.stringify(obj, replacer, space)
    arktest.assertTrue(str.indexOf(' ') < 0)
    arktest.assertEQ(str, '{"m":1,"n":2}')
}

function replacerFixedLarge(): void {
    let obj: Record<string, Object> = {}
    obj['prop'] = 'value' as Object

    let replacer: FixedArray<string> = ['prop']
    let space: int = 20
    const str: string = JSON.stringify(obj, replacer, space)

    arktest.assertTrue(str.indexOf('          ') >= 0)
}

function replacerFixedUndef(): void {
    let obj: Record<string, Object> = {}
    obj['x'] = 100 as Object

    let replacer: FixedArray<string> = ['x']
    const str: string = JSON.stringify(obj, replacer, undefined)
    arktest.assertTrue(str.indexOf('\n') < 0)
    arktest.assertEQ(str, '{"x":100}')
}

function replacerFixedShallow(): void {
    let inner: Record<string, Object> = {}
    inner['a'] = 1 as Object
    inner['b'] = 2 as Object
    let obj: Record<string, Object> = {}
    obj['outer'] = 'value' as Object
    obj['nested'] = inner as Object

    let replacer: FixedArray<string> = ['outer']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"outer"') >= 0)
    arktest.assertTrue(str.indexOf('"nested"') < 0)
}

function replacerFixedDup(): void {
    let obj: Record<string, Object> = {}
    obj['key1'] = 'first' as Object
    obj['key2'] = 'second' as Object

    let replacer: FixedArray<string> = ['key1', 'key1', 'key2']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"key1"') >= 0)
    arktest.assertTrue(str.indexOf('"key2"') >= 0)
}

function replacerFixedMissing(): void {
    let obj: Record<string, Object> = {}
    obj['present'] = 'yes' as Object

    let replacer: FixedArray<string> = ['present', 'missing']
    const str: string = JSON.stringify(obj, replacer)
    arktest.assertTrue(str.indexOf('"present"') >= 0)
    arktest.assertTrue(str.indexOf('"missing"') < 0)
}

function largeIntEdge(): void {
    const arr: FixedArray<int> = [
        Long.toInt(-2147483649),
        Long.toInt(2147483648)
    ] as FixedArray<int>
    const str: string = JSON.stringify(arr)
    arktest.assertEQ(str, '[2147483647,-2147483648]')
}

function largeCharArray(): void {
    const chars: FixedArray<char> = [c'A', c'B', c'C', c'D', c'E', c'F', c'G', c'H', c'I', c'J', c'K', c'L', c'M', c'N',
        c'O', c'P', c'Q', c'R', c'S', c'T', c'U', c'V', c'W', c'X', c'Y', c'Z']

    let arr: FixedArray<char> = []
    for (let i = 0; i < 1000; i++) {
        arr = [...arr, chars[i % 26]]
    }
    const str: string = JSON.stringify(arr)

    arktest.assertTrue(str.startsWith('["A"'))
    arktest.assertTrue(str.endsWith('"]'))
    arktest.assertEQ(str.split(',').length, 1000)

    arktest.assertTrue(str.indexOf(',]') < 0)
}

function simpleObjStr(): void {
    let obj = new SimpleObj()
    obj.name = 'Default'
    obj.value = 0
    const str = JSON.stringify(obj as Object, null)
    arktest.assertEQ(str, '{"name":"Default","year":0,"value":0}')
}

function simpleObjUndefReplacer(): void {
    let obj = new SimpleObj()
    obj.name = 'UndefinedReplacer'
    obj.value = 123
    const str = JSON.stringify(obj as Object, undefined)
    arktest.assertEQ(str, '{"name":"UndefinedReplacer","year":0,"value":123}')
}

function simpleObjPrettyPrint(): void {
    let obj = new SimpleObj()
    obj.name = 'A'
    obj.value = 1
    const str = JSON.stringify(obj as Object, null, 2)
    const expected = '{\n  "name": "A",\n  "year": 0,\n  "value": 1\n}'
    arktest.assertEQ(str, expected)
}

function booleanObjPrettyPrint(): void {
    let obj = new BooleanObj()
    obj.boolVal = true
    const str = JSON.stringify(obj, null, 4)
    const expected = '{\n    "boolVal": true\n}'
    arktest.assertEQ(str, expected)
}

function simpleObjTabIndent(): void {
    let obj = new SimpleObj()
    obj.name = 'Tab'
    obj.value = 0
    const str = JSON.stringify(obj as Object, null, '\t')
    const expected = '{\n\t"name": "Tab",\n\t"year": 0,\n\t"value": 0\n}'
    arktest.assertEQ(str, expected)
}

function simpleObjLargeIndent(): void {
    let obj = new SimpleObj()
    obj.value = 1
    const str = JSON.stringify(obj, null, 15)
    const space10 = '          '
    arktest.assertTrue(str.includes('\n' + space10 + '"name"'))
    const space11 = '           '
    arktest.assertFalse(str.includes('\n' + space11 + '"name"'))
}

function simpleObjZeroIndent(): void {
    let obj = new SimpleObj()
    obj.name = 'Compact'
    obj.value = 1
    const str = JSON.stringify(obj as Object, null, 0)
    arktest.assertEQ(str, '{"name":"Compact","year":0,"value":1}')
}

function simpleObjEmptyStrIndent(): void {
    let obj = new SimpleObj()
    obj.name = 'CompactStr'
    obj.value = 1
    const str = JSON.stringify(obj as Object, null, '')
    arktest.assertEQ(str, '{"name":"CompactStr","year":0,"value":1}')
}

function byteStringify(): void {
    const testBytes: byte[] = [-128, -100, -10, -1, 0, 1, 10, 100, 127]
    const expectedLen: number[] = [4, 4, 3, 2, 1, 1, 2, 3, 3]

    for (let i = 0; i < testBytes.length; i++) {
        const byteVal: byte = testBytes[i]
        const str = JSON.stringify(byteVal)

        arktest.assertEQ(typeof str, 'string')
        arktest.assertEQ(str.length, expectedLen[i])
    }
}

function byteValidation(): void {
    const validationSet: byte[] = [-128, -96, -48, -16, -2, 3, 15, 31, 63, 95, 111, 127]
    const expectedStr: string[] = ['-128', '-96', '-48', '-16', '-2', '3', '15', '31', '63', '95', '111', '127']
    const expectedLen: number[] = [4, 3, 3, 3, 2, 1, 2, 2, 2, 2, 3, 3]

    for (let i = 0; i < validationSet.length; i++) {
        const input: byte = validationSet[i]
        const actual: string = JSON.stringify(input)

        arktest.assertEQ(actual, expectedStr[i])
        arktest.assertEQ(actual.length, expectedLen[i])

        const containsDash: boolean = actual.indexOf('-') >= 0
        arktest.assertEQ(containsDash, input < 0)
    }
}

function shortStringify(): void {
    const testShorts: short[] = [-32768, -10000, -100, -1, 0, 1, 100, 10000, 32767]
    const expectedLens: number[] = [6, 6, 4, 2, 1, 1, 3, 5, 5]

    for (let i = 0; i < testShorts.length; i++) {
        const shortVal: short = testShorts[i]
        const str = JSON.stringify(shortVal)

        arktest.assertEQ(typeof str, 'string')
        arktest.assertEQ(str.length, expectedLens[i])
    }
}

function shortValidation(): void {
    const validationSet: short[] = [-32768, -16384, -4096, -256, -16, -2, 3, 15, 127, 1024, 8192, 32767]
    const expectedStr: string[] = ['-32768', '-16384', '-4096', '-256', '-16', '-2', '3', '15', '127', '1024', '8192',
        '32767']
    const expectedLen: number[] = [6, 6, 5, 4, 3, 2, 1, 2, 3, 4, 4, 5]

    for (let i = 0; i < validationSet.length; i++) {
        const input: short = validationSet[i]
        const actual: string = JSON.stringify(input)

        arktest.assertEQ(actual, expectedStr[i])
        arktest.assertEQ(actual.length, expectedLen[i])

        const containsDash: boolean = actual.indexOf('-') >= 0
        arktest.assertEQ(containsDash, input < 0)
    }
}

function intStringify(): void {
    const testInts: int[] = [-2147483648, -1000000000, -1000000, -1, 0, 1, 1000000, 1000000000, 2147483647]
    const expectedLens: number[] = [11, 11, 8, 2, 1, 1, 7, 10, 10]

    for (let i = 0; i < testInts.length; i++) {
        const intVal: int = testInts[i]
        const str = JSON.stringify(intVal)

        arktest.assertEQ(typeof str, 'string')
        arktest.assertEQ(str.length, expectedLens[i])
    }
}

function intNegative(): void {
    const negativeSamples: int[] = [-2147483648, -1000000, -1]

    for (let i = 0; i < negativeSamples.length; i++) {
        const val: int = negativeSamples[i]
        const str = JSON.stringify(val)

        arktest.assertTrue(str.indexOf('-') >= 0)
        arktest.assertEQ(str, val.toString())
    }
}

function intPositive(): void {
    const positiveSamples: int[] = [1, 1000000, 2147483647]

    for (let i = 0; i < positiveSamples.length; i++) {
        const val: int = positiveSamples[i]
        const str = JSON.stringify(val)

        arktest.assertFalse(str.indexOf('-') >= 0)
        arktest.assertEQ(str, val.toString())
    }
}

function bigintStringify(): void {
    const testLongs: bigint[] = [-9223372036854775808n, -1000000000000000000n, -1n, 0n, 1n, 1000000000000000000n,
        9223372036854775807n]
    const expectedLens: number[] = [20, 20, 2, 1, 1, 19, 19]

    for (let i = 0; i < testLongs.length; i++) {
        const longVal: bigint = testLongs[i]
        const str = JSON.stringify(longVal)

        arktest.assertEQ(typeof str, 'string')
        arktest.assertEQ(str.length, expectedLens[i])
    }
}

function bigintValidation(): void {
    const validationSet: bigint[] = [-9223372036854775808n, -4611686018427387904n, -2147483648n, -1n, 0n, 1n,
        2147483647n, 4611686018427387903n, 9223372036854775807n]
    const expectedStr: string[] = ['-9223372036854775808', '-4611686018427387904', '-2147483648', '-1', '0', '1',
        '2147483647', '4611686018427387903', '9223372036854775807']
    const expectedLen: number[] = [20, 20, 11, 2, 1, 1, 10, 19, 19]

    for (let i = 0; i < validationSet.length; i++) {
        const input: bigint = validationSet[i]
        const actual: string = JSON.stringify(input)

        arktest.assertEQ(actual, expectedStr[i])
        arktest.assertEQ(actual.length, expectedLen[i])

        const containsDash: boolean = actual.indexOf('-') >= 0
        arktest.assertEQ(containsDash, input < 0)
    }
}

function bigintNegative(): void {
    const negativeSamples: bigint[] = [-9223372036854775808n, -1n, -4611686018427387904n]

    for (let i = 0; i < negativeSamples.length; i++) {
        const val: bigint = negativeSamples[i]
        const str = JSON.stringify(val)
        arktest.assertTrue(str.indexOf('-') >= 0)
        arktest.assertEQ(str, val.toString())
    }
}

function bigintPositive(): void {
    const positiveSamples: bigint[] = [1n, 4611686018427387903n, 9223372036854775807n]

    for (let i = 0; i < positiveSamples.length; i++) {
        const val: bigint = positiveSamples[i]
        const str = JSON.stringify(val)
        arktest.assertFalse(str.indexOf('-') >= 0)
        arktest.assertEQ(str, val.toString())
    }
}

function bigintLargeStringify(): void {
    const testBigints: bigint[] = [1n, 9007199254740991n, 9223372036854775807n, 123456789012345678901234567890n]
    const expectedLens: number[] = [1, 16, 19, 30]

    for (let i = 0; i < testBigints.length; i++) {
        const bigintVal: bigint = testBigints[i]
        const str = JSON.stringify(bigintVal)

        arktest.assertEQ(typeof str, 'string')
        arktest.assertEQ(str.length, expectedLens[i])
    }
}

function bigintLargeValidation(): void {
    const validationSet: bigint[] = [
        -9223372036854775808n,
        -9007199254740991n,
        -1n,
        0n,
        1n,
        9007199254740991n,
        9223372036854775807n,
        18446744073709551615n
    ]

    for (let i = 0; i < validationSet.length; i++) {
        const input: bigint = validationSet[i]
        const actual: string = JSON.stringify(input)
        const expected: string = input.toString()

        arktest.assertEQ(actual, expected)

        const containsDash: boolean = actual.indexOf('-') >= 0
        arktest.assertEQ(containsDash, input < 0)
    }
}

function asciiCharStringify(): void {
    const asciiChars: char[] = [
        c' ', c'!', c'"', c'#', c'$', c'%', c'&', c'\'', c'(', c')', c'*', c'+', c',', c'-', c'.', c'/',
        c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9',
        c':', c';', c'<', c'=', c'>', c'?', c'@',
        c'A', c'B', c'C', c'D', c'E', c'F', c'G', c'H', c'I', c'J', c'K', c'L', c'M',
        c'N', c'O', c'P', c'Q', c'R', c'S', c'T', c'U', c'V', c'W', c'X', c'Y', c'Z',
        c'[', c'\\', c']', c'^', c'_', c'`',
        c'a', c'b', c'c', c'd', c'e', c'f', c'g', c'h', c'i', c'j', c'k', c'l', c'm',
        c'n', c'o', c'p', c'q', c'r', c's', c't', c'u', c'v', c'w', c'x', c'y', c'z',
        c'{', c'|', c'}', c'~'
    ]

    for (let ch of asciiChars) {
        const str = JSON.stringify(ch)
        arktest.assertEQ(str.length, 3)
        arktest.assertEQ(str.charAt(0), c'"')
        arktest.assertEQ(str.charAt(1), ch)
        arktest.assertEQ(str.charAt(2), c'"')
    }
}

function largeDeepNestedStr(): void {
    const size = 100
    let treeString = '['
    for (let i = 0; i < size; i++) {
        treeString += '{"a":1},'
    }
    treeString += '{"a":1}]'

    const str = JSON.stringify(treeString)

    arktest.assertTrue(str.startsWith('"'))
    arktest.assertTrue(str.endsWith('"'))
    arktest.assertTrue(str.contains('\\"', 0))
    arktest.assertTrue(str.length > treeString.length)
}

function largeNumArray(): void {
    const largeArray: number[] = []
    for (let i: int = 0; i < 100; i++) {
        largeArray.push(i)
    }
    const str: string = JSON.stringify(largeArray)
    const expected: string = `[${largeArray.join(',')}]`

    arktest.assertEQ(str, expected)
}

function largeObjStr(): void {
    const tempObj: Record<string, double> = {}
    for (let i: int = 0; i < 100; i++) {
        tempObj[`key${i}`] = Int.toDouble(i)
    }
    const largeObj: Object = tempObj

    const str: string = JSON.stringify(largeObj)

    arktest.assertTrue(str instanceof String)
    arktest.assertTrue(str.length > 0)
}

function jsonPathSymbols(): void {
    const originalArr: string[] = [
        '$.store.book[0].title',
        '$.store.book[*].author',
        '$.store..price'
    ]
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr, (key: string, value: jsonx.JsonElement) => {
        return value
    })

    arktest.assertEQ(parsedArr[0].asString(), '$.store.book[0].title')
    arktest.assertEQ(parsedArr[2].asString(), '$.store..price')
}

function multiLangPunctReviver(): void {
    const originalObj: MultiLangPunctObj = {
        messages: {
            en: 'Hello! How are you?',
            fr: 'Bonjour ! Comment allez-vous ?',
            zh: '你好！你怎么样？'
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    const parsedObj: MultiLangPunctObj = JSON.parse<MultiLangPunctObj>(jsonStr, (key: string, value: Any) => {
        return value
    }, Type.from<MultiLangPunctObj>()) as MultiLangPunctObj

    arktest.assertEQ(parsedObj.messages.fr, 'Bonjour ! Comment allez-vous ?')
    arktest.assertEQ(parsedObj.messages.zh, '你好！你怎么样？')
}

function reviverCharCount(): void {
    const obj: CharCount = { text: 'abac', count: 0 }
    const json = JSON.stringify(obj)
    const parsed: CharCount = JSON.parse<CharCount>(json, (k: string, v: Any) => {
        if (k === '') {
            let c = 0
            for (let i = 0; i < (v as CharCount).text.length; i++) {
                if ((v as CharCount).text[i] === 'a') {
                    c++
                }
            }
            return { text: (v as CharCount).text, count: c } as CharCount
        }
        return v
    }, Type.from<CharCount>()) as CharCount
    arktest.assertEQ(parsed.count, 2)
}

function reviverStringReplace(): void {
    const obj: StringReplace = { text: 'abc', replaced: '' }
    const json = JSON.stringify(obj)
    const parsed: StringReplace = JSON.parse<StringReplace>(json, (k: string, v: Any) => {
        if (k === '') {
            const r = (v as StringReplace).text.replace('a', 'x')
            return { text: (v as StringReplace).text, replaced: r } as StringReplace
        }
        return v
    }, Type.from<StringReplace>()) as StringReplace
    arktest.assertEQ(parsed.replaced, 'xbc')
}

function circularRefSelf(): void {
    let obj: Record<string, Object> = {}
    obj['self'] = obj
    arktest.expectError(() => { JSON.stringify(obj) })
}

function circularRefMutual(): void {
    let obj1: Record<string, Object> = {}
    let obj2: Record<string, Object> = {}
    obj1['ref'] = obj2
    obj2['ref'] = obj1
    arktest.expectError(() => { JSON.stringify(obj1) })
}

function circularRefTriple(): void {
    let obj1: Record<string, Object> = {}
    let obj2: Record<string, Object> = {}
    let obj3: Record<string, Object> = {}
    obj1['ref'] = obj2
    obj2['ref'] = obj3
    obj3['ref'] = obj1
    arktest.expectError(() => { JSON.stringify(obj1) })
}

function circularRefObjArr(): void {
    let obj: Record<string, Object> = {}
    let arr: Object[] = new Array<Object>();
    obj['arr'] = arr
    arr.push(obj)
    arktest.expectError(() => { JSON.stringify(obj) })
}

function circularRefSelf2(): void {
    let obj: Record<string, Object> = {}
    obj['self'] = obj
    arktest.expectError(() => { JSON.stringify(obj) })
}

function keyOrderNumeric(): void {
    let obj: Record<string, string> = {}
    obj['2'] = 'b'
    obj['1'] = 'a'
    obj['3'] = 'c'
    let str: string = JSON.stringify(obj)
    arktest.assertEQ(str, '{"2":"b","1":"a","3":"c"}')
}

function keyOrderMixed(): void {
    let obj: Record<string, number | string> = {}
    obj['a'] = 1
    obj['2'] = 'b'
    obj['1'] = 'c'
    let str: string = JSON.stringify(obj)
    arktest.assertEQ(str, '{"a":1,"2":"b","1":"c"}')
}

function keyOrderArrayLike(): void {
    let obj: Record<string, Any> = {}
    obj['0'] = 'a'
    obj['1'] = 'b'
    obj['2'] = 'c'
    obj['length'] = 3
    let str: string = JSON.stringify(obj)
    arktest.assertEQ(str, '{"0":"a","1":"b","2":"c","length":3}')
}

function keyOrderArrayLikeNoLen(): void {
    let obj: Record<string, string> = {}
    obj['0'] = 'a'
    obj['1'] = 'b'
    obj['name'] = 'test'
    let str: string = JSON.stringify(obj)
    arktest.assertEQ(str, '{"0":"a","1":"b","name":"test"}')
}
