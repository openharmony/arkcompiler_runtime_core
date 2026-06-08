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
    const suite = new arktest.ArkTestsuite('JSON.stringify object replacer and spacing tests')
    suite.addTest('property list filter keeps only a and c', propList)
    suite.addTest('2-space indentation with null replacer', indent2)
    suite.addTest('replacer removes string values', strFlt)
    suite.addTest('double dash string indentation', indentDD)
    suite.addTest('15-space indentation truncated to 10', indent15)
    suite.addTest('replacer receives top-level key a', trackKey)
    suite.addTest('indent string truncated to 10 chars', truncIndent)
    suite.addTest('property list outputs in declared order', reorderProp)
    suite.addTest('zero indentation flattens object', indent0a)
    suite.addTest('nested replacer removes string values', nestStrFlt)
    suite.addTest('replacer doubles nested value', nestMul2)
    suite.addTest('function in array becomes null', arrFuncNul)
    suite.addTest('null indent parameter flattens object', indentNul)
    suite.addTest('replacer removes key b', fltB)
    suite.addTest('replacer removes array index 1', fltIdx1)
    suite.addTest('tab character indentation', indentTab)
    suite.addTest('nested replacer removes string and keeps number', nestStrNumFlt)
    suite.addTest('comment-style string indentation', indentCmt)
    suite.addTest('single space indentation', indent1)
    suite.addTest('replacer converts zero to true via equality', zeroTrue)
    suite.addTest('array replacer first key is index 0', arrKey0)
    suite.addTest('newline character as indentation', indentNl)
    suite.addTest('deeply nested replacer removes strings', deepStrFlt)
    suite.addTest('replacer converts string to number', str2Num)
    suite.addTest('undefined and function in array become null', undefFuncNul)
    suite.addTest('2-space indentation with multiple keys', indent2Obj)
    suite.addTest('replacer converts number to string', num2Str)
    suite.addTest('replacer returns function for index 1', arrFuncRepl)
    suite.addTest('root-level replacer returns only field a', rootFltA)
    suite.addTest('10-space indentation for value 25', indent10a)
    suite.addTest('single dash as indentation string', indent1Dash)
    suite.addTest('replacer removes even integers', fltEven)
    suite.addTest('space and greater-than indentation', indentGt)
    suite.addTest('negative indentation treated as zero', indentNeg5)
    suite.addTest('replacer replaces key b with null', b2Nul)
    suite.addTest('10-space indentation for value 7', indent10b)
    suite.addTest('function property omitted from output', funcValObj)
    suite.addTest('replacer removes strings from mixed object array', mixStrFlt)
    suite.addTest('replacer converts equality check to boolean', aIs1)
    suite.addTest('tab and space combined indentation', indentTabSp)
    suite.addTest('replacer removes array index 2', fltIdx2)
    suite.addTest('property list with 2-space indentation', propListInd2)
    suite.addTest('replacer wraps value and doubles it', wrapDbl)
    suite.addTest('double pipe as indentation string', indentPipe)
    suite.addTest('replacer converts number 100 to string', num2Str100)
    suite.addTest('5-space indentation', indent5)
    suite.addTest('space and dash indentation', indentDashSp)
    suite.addTest('nested array replacer removes strings', nstArrStrFlt)
    suite.addTest('replacer removes key a', fltA)
    suite.addTest('10-space indentation with space param 11', indent10c)
    suite.addTest('function as first array element becomes null', arrFunc1st)
    suite.addTest('replacer replaces strings with new-str', arrStrRep)
    suite.addTest('replacer converts inequality check to true', aNot1)
    suite.addTest('replacer converts all ints to strings', ints2Strs)
    suite.addTest('replacer adds 5 to first array element', firstP5)
    suite.addTest('replacer rewrites object as flat array', obj2Arr)
    suite.addTest('3-space indentation', indent3)
    suite.addTest('standalone function stringifies to empty', funcEmpty)
    suite.addTest('replacer uppercases string value', toUpper)
    suite.addTest('arrow string indentation', indentArrow)
    suite.addTest('zero indentation for value 25', indent0b)
    suite.addTest('replacer converts null to zero', nul2Zero)
    suite.addTest('replacer prepends hash to value', prefixHash)
    suite.addTest('10-space indentation for value 30', indent10d)
    suite.addTest('deeply nested array replacer removes strings', deepArrStrFlt)
    suite.addTest('abc123 string indentation', indentAbc)
    suite.addTest('replacer converts index 1 to boolean true', idx1True)
    suite.addTest('tab and dash combined indentation', indentTabDash)
    suite.addTest('replacer removes nested key b', nestFltB)
    suite.addTest('replacer converts object to array of objects', obj2Arr2)
    suite.addTest('6-space indentation', indent6)
    suite.addTest('replacer converts comparison to false', aNotPos)
    suite.addTest('star string indentation', indentStar)
    suite.addTest('negative one indentation treated as zero', indentNeg1)
    suite.addTest('replacer replaces all strings with new', arrStrRep2)
    suite.addTest('property list with duplicate keys', propDup)
    suite.addTest('replacer removes nested key z', nestFltZ)
    suite.addTest('10-space indentation with nested object', indent10Nest)
    suite.addTest('array of nested objects replacer removes strings', arrNstStrFlt)
    suite.addTest('replacer rewrites object as incrementing array', obj2ArrInc)
    suite.addTest('double tab indentation', indent2Tab)
    suite.addTest('replacer explicitly filters functions', arrFltFunc)
    suite.addTest('replacer multiplies nested value by 3', nestMul3)
    return suite.run()
}

interface StringAndNumberFields {
    a: string
    b: number
}

class PropertyListCandidate {
    a: number
    b: number
    c: number
}

interface SingleXNumberFields {
    x: number
}

interface SingleYNumberFields {
    y: number
}

interface SingleZNumberFields {
    z: number
}

interface SingleANumberFields {
    a: number
}

interface SingleMNumberFields {
    m: number
}

interface NestedStringFilterRoot {
    a: NestedStringFilterChild
}

interface NestedStringFilterChild {
    b: number
    c: string
}

interface NestedMultiplierRoot {
    x: NestedMultiplierValue
}

interface NestedMultiplierValue {
    y: number
}

interface TwoNumberFields {
    a: number
    b: number
}

interface NumberFlagFields {
    num: number
    flag: boolean
}

interface NestedStringNumberRoot {
    x: StringNumberFields
}

interface StringNumberFields {
    str: string
    num: number
}

interface DeepStringNumberRoot {
    a: DeepStringNumberMiddle
}

interface DeepStringNumberMiddle {
    b: DeepStringNumberLeaf
}

interface DeepStringNumberLeaf {
    c: string
    d: number
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

interface FunctionAndValueFields {
    func: Function
    val: number
}

interface NullAndNumberFields {
    a: null
    b: number
}

interface KeyStringFields {
    key: string
}

interface WrappedSingleANumberFields {
    a: SingleANumberFields
}

interface WeatherStatusFields {
    temp: number
    status: string
}

interface DataStringFields {
    data: string
}

interface ZNumberFields {
    z: number
}

interface XStringYNumberFields {
    x: string
    y: number
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

interface TextStringFields {
    text: string
}

interface CNumberFields {
    c: number
}

interface LineStringFields {
    line: string
}

interface NestedBCAndDFields {
    a: BCNumberFields
    d: number
}

interface BCNumberFields {
    b: number
    c: number
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

interface IdNameFields {
    id: number
    name: string
}

interface YZNumberFields {
    y: number
    z: number
}

interface NestedYZAndWFields {
    x: YZNumberFields
    w: number
}

interface BNumberFields {
    b: number
}

interface NestedBNumberFields {
    a: BNumberFields
}

interface NestedStringNumberFields {
    nested: StringNumberFields
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

interface MessageStringFields {
    msg: string
}

interface DescriptionStringFields {
    desc: string
}

interface XNumberFields {
    x: number
}

interface NestedABRootFields {
    a: BNumberFields
}

interface LargeSmallNumberFields {
    largeNum: number
    smallNum: number
}

function propList() {
    const obj: PropertyListCandidate = { a: 1, b: 2, c: 3 }
    const jsonStr = JSON.stringify(obj, ['a', 'c'])
    arktest.assertEQ(jsonStr, '{"a":1,"c":3}')
}

function indent2() {
    const obj: SingleXNumberFields = { x: 10 }
    const jsonStr = JSON.stringify(obj, null, 2)
    arktest.assertEQ(jsonStr, '{\n  "x": 10\n}')
}

function strFlt() {
    const obj: StringAndNumberFields = { a: 'test', b: 5 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => typeof v === 'string' ? undefined : v)
    arktest.assertEQ(jsonStr, '{"b":5}')
}

function indentDD() {
    const obj: SingleYNumberFields = { y: 20 }
    const jsonStr = JSON.stringify(obj, null, '--')
    arktest.assertEQ(jsonStr, '{\n--"y": 20\n}')
}

function indent15() {
    const obj: SingleZNumberFields = { z: 30 }
    const jsonStr = JSON.stringify(obj, null, 15)
    const expected = '{\n          "z": 30\n}'
    arktest.assertEQ(jsonStr, expected)
}

function trackKey() {
    let topKey = ''
    JSON.stringify({ a: 1 } as SingleANumberFields, (k: string, v: Any) => {
        topKey = k
        return v
    })
    arktest.assertEQ(topKey, 'a')
}

function truncIndent() {
    const obj: SingleMNumberFields = { m: 5 }
    const spaceStr = '1234567890123'
    const jsonStr = JSON.stringify(obj, null, spaceStr)
    const expected = '{\n1234567890"m": 5\n}'
    arktest.assertEQ(jsonStr, expected)
}

function reorderProp() {
    const obj: SingleANumberFields = { a: 1 }
    const jsonStr = JSON.stringify(obj, ['b', 'a'])
    arktest.assertEQ(jsonStr, '{"a":1}')
}

function indent0a() {
    const obj: SingleANumberFields = { a: 15 }
    const jsonStr = JSON.stringify(obj, null, 0)
    arktest.assertEQ(jsonStr, '{"a":15}')
}

function nestStrFlt() {
    const obj: NestedStringFilterRoot = { a: { b: 2, c: 'str' } }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => typeof v === 'string' ? undefined : v)
    arktest.assertEQ(jsonStr, '{"a":{"b":2}}')
}

function nestMul2() {
    const obj: NestedMultiplierRoot = { x: { y: 3 } }
    const jsonStr = JSON.stringify(obj,
        (k: string, v: Any) => k === 'x' ? { y: (v as NestedMultiplierValue).y * 2 } as NestedMultiplierValue : v)
    arktest.assertEQ(jsonStr, '{"x":{"y":6}}')
}

function arrFuncNul() {
    const arr = [1, () => {
    }, 3]
    const jsonStr = JSON.stringify(arr)
    arktest.assertEQ(jsonStr, '[1,null,3]')
}

function indentNul() {
    const obj: TwoNumberFields = { a: 1, b: 2 }
    const jsonStr = JSON.stringify(obj, null)
    arktest.assertEQ(jsonStr, '{"a":1,"b":2}')
}

function fltB() {
    const obj: TwoNumberFields = { a: 1, b: 2 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'b' ? undefined : v)
    arktest.assertEQ(jsonStr, '{"a":1}')
}

function fltIdx1() {
    const arr = [1, 2, 3]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => Number(k) === 1 ? undefined : v)
    arktest.assertEQ(jsonStr, '[1,null,3]')
}

function indentTab() {
    const obj: SingleANumberFields = { a: 5 }
    const jsonStr = JSON.stringify(obj, null, '\t')
    arktest.assertEQ(jsonStr, '{\n\t"a": 5\n}')
}

function nestStrNumFlt() {
    const obj: NestedStringNumberRoot = { x: { str: 'test', num: 5 } }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => typeof v === 'string' ? undefined : v)
    arktest.assertEQ(jsonStr, '{"x":{"num":5}}')
}

function indentCmt() {
    const obj: SingleANumberFields = { a: 8 }
    const jsonStr = JSON.stringify(obj, null, '/* ')
    arktest.assertEQ(jsonStr, '{\n/* "a": 8\n}')
}

function indent1() {
    const obj: SingleANumberFields = { a: 12 }
    const jsonStr = JSON.stringify(obj, null, 1)
    arktest.assertEQ(jsonStr, '{\n "a": 12\n}')
}

function zeroTrue() {
    const obj: SingleANumberFields = { a: 0 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'a' ? v === 0 : v)
    arktest.assertEQ(jsonStr, '{"a":true}')
}

function arrKey0() {
    let initialKey = ''
    JSON.stringify([1, 2], (k: string, v: Any) => {
        if (initialKey === '') {
            initialKey = k
        }
        return v
    })
    arktest.assertEQ(initialKey, '0')
}

function indentNl() {
    const obj: SingleANumberFields = { a: 15 }
    const jsonStr = JSON.stringify(obj, null, '\n')
    arktest.assertEQ(jsonStr, '{\n\n"a": 15\n}')
}

function deepStrFlt() {
    const obj: DeepStringNumberRoot = { a: { b: { c: 'str', d: 5 } } }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => typeof v === 'string' ? undefined : v)
    arktest.assertEQ(jsonStr, '{"a":{"b":{"d":5}}}')
}

function str2Num() {
    const obj: StringNumberTextFields = { str: '5' }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'str' ? Number(v as string) : v)
    arktest.assertEQ(jsonStr, '{"str":5}')
}

function undefFuncNul() {
    const arr = [undefined, () => {
    }, 3]
    const jsonStr = JSON.stringify(arr)
    arktest.assertEQ(jsonStr, '[null,null,3]')
}

function indent2Obj() {
    const obj: TwoNumberFields = { a: 10, b: 20 }
    const jsonStr = JSON.stringify(obj, null, 2)
    arktest.assertEQ(jsonStr, '{\n  "a": 10,\n  "b": 20\n}')
}

function num2Str() {
    const obj: SingleANumberFields = { a: 5 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'a' ? (v as number).toString() : v)
    arktest.assertEQ(jsonStr, '{"a":"5"}')
}

function arrFuncRepl() {
    const arr = [1, 2, 3]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => Number(k) === 1 ? () => {
    } : v)
    arktest.assertEQ(jsonStr, '[1,null,3]')
}

function rootFltA() {
    const obj: TwoNumberFields = { a: 1, b: 2 }
    const jsonStr = JSON.stringify(obj,
        (k: string, v: Any) => k === '' ? { a: (v as TwoNumberFields).a } as SingleANumberFields : v)
    arktest.assertEQ(jsonStr, '{"a":1}')
}

function indent10a() {
    const obj: SingleANumberFields = { a: 25 }
    const jsonStr = JSON.stringify(obj, null, 10)
    const expected = '{\n          "a": 25\n}'
    arktest.assertEQ(jsonStr, expected)
}

function indent1Dash() {
    const obj: SingleANumberFields = { a: 8 }
    const jsonStr = JSON.stringify(obj, null, '-')
    arktest.assertEQ(jsonStr, '{\n-"a": 8\n}')
}

function fltEven() {
    const arr = [1, 2, 3, 4]
    const jsonStr = JSON.stringify(arr,
        (k: string, v: Any) => typeof v === 'int' && (v as int) % 2 === 0 ? undefined : v)
    arktest.assertEQ(jsonStr, '[1,null,3,null]')
}

function indentGt() {
    const obj: SingleANumberFields = { a: 12 }
    const jsonStr = JSON.stringify(obj, null, '  >')
    arktest.assertEQ(jsonStr, '{\n  >"a": 12\n}')
}

function indentNeg5() {
    const obj: SingleANumberFields = { a: 15 }
    const jsonStr = JSON.stringify(obj, null, -5)
    arktest.assertEQ(jsonStr, '{"a":15}')
}

function b2Nul() {
    const obj: TwoNumberFields = { a: 1, b: 2 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'b' ? null : v)
    arktest.assertEQ(jsonStr, '{"a":1,"b":null}')
}

function indent10b() {
    const obj: SingleANumberFields = { a: 7 }
    const jsonStr = JSON.stringify(obj, null, 10)
    const expected = '{\n          "a": 7\n}'
    arktest.assertEQ(jsonStr, expected)
}

function funcValObj() {
    const obj: FunctionAndValueFields = {
        func: () => {
        }, val: 5
    }
    const jsonStr = JSON.stringify(obj)
    arktest.assertEQ(jsonStr, '{"val":5}')
}

function mixStrFlt() {
    const arr = [{ a: 'str', b: 5 } as StringAndNumberFields, { a: 10 } as SingleANumberFields]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => typeof v === 'string' ? undefined : v)
    arktest.assertEQ(jsonStr, '[{"b":5},{"a":10}]')
}

function aIs1() {
    const obj: SingleANumberFields = { a: 1 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'a' ? v as number === 1 : v)
    arktest.assertEQ(jsonStr, '{"a":true}')
}

function indentTabSp() {
    const obj: SingleANumberFields = { a: 11 }
    const jsonStr = JSON.stringify(obj, null, '\t ')
    arktest.assertEQ(jsonStr, '{\n\t "a": 11\n}')
}

function fltIdx2() {
    const arr = [1, 2, 3, 4]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => Number(k) === 2 ? undefined : v)
    arktest.assertEQ(jsonStr, '[1,2,null,4]')
}

function propListInd2() {
    const obj: PropertyListCandidate = { a: 1, b: 2, c: 3 }
    const jsonStr = JSON.stringify(obj, ['a', 'c'], 2)
    arktest.assertEQ(jsonStr, '{\n  "a": 1,\n  "c": 3\n}')
}

function wrapDbl() {
    const obj: SingleANumberFields = { a: 5 }
    const jsonStr = JSON.stringify(obj,
        (k: string, v: Any) => k === '' ? {
            a: { a: (v as SingleANumberFields).a * 2 } as SingleANumberFields } as WrappedSingleANumberFields : v)
    arktest.assertEQ(jsonStr, '{"a":{"a":10}}')
}

function indentPipe() {
    const obj: KeyStringFields = { key: 'val' }
    const jsonStr = JSON.stringify(obj, null, '||')
    arktest.assertEQ(jsonStr, '{\n||"key": "val"\n}')
}

function num2Str100() {
    const obj: SingleANumberFields = { a: 100 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'a' ? (v as number).toString() : v)
    arktest.assertEQ(jsonStr, '{"a":"100"}')
}

function indent5() {
    const obj: SingleANumberFields = { a: 7 }
    const jsonStr = JSON.stringify(obj, null, 5)
    arktest.assertEQ(jsonStr, '{\n     "a": 7\n}')
}

function indentDashSp() {
    const obj: DataStringFields = { data: 'test' }
    const jsonStr = JSON.stringify(obj, null, '  - ')
    arktest.assertEQ(jsonStr, '{\n  - "data": "test"\n}')
}

function nstArrStrFlt() {
    const arr = [['str1', 5], ['str2', 10]]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => typeof v === 'string' ? undefined : v)
    arktest.assertEQ(jsonStr, '[[null,5],[null,10]]')
}

function fltA() {
    const obj: TwoNumberFields = { a: 1, b: 2 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'a' ? undefined : v)
    arktest.assertEQ(jsonStr, '{"b":2}')
}

function indent10c() {
    const obj: SingleANumberFields = { a: 3 }
    const jsonStr = JSON.stringify(obj, null, 11)
    const expected = '{\n          "a": 3\n}'
    arktest.assertEQ(jsonStr, expected)
}

function arrFunc1st() {
    const arr = [() => {
    }, 2, 3]
    const jsonStr = JSON.stringify(arr)
    arktest.assertEQ(jsonStr, '[null,2,3]')
}

function arrStrRep() {
    const arr = [{ x: 'str', y: 5 } as XStringYNumberFields, { z: 10 } as ZNumberFields]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => typeof v === 'string' ? 'new-str' : v)
    arktest.assertEQ(jsonStr, '[{"x":"new-str","y":5},{"z":10}]')
}

function aNot1() {
    const obj: SingleANumberFields = { a: 0 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'a' ? (v as number) !== 1 : v)
    arktest.assertEQ(jsonStr, '{"a":true}')
}

function ints2Strs() {
    const arr = [1, 2, 3]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => typeof v === 'int' ? (v as int).toString() : v)
    arktest.assertEQ(jsonStr, '["1","2","3"]')
}

function firstP5() {
    const arr = [10, 20, 30]
    const jsonStr = JSON.stringify(arr,
        (k: string, v: Any) => Number(k) === 0 && typeof v === 'int' ? (v as int) + 5 : v)
    arktest.assertEQ(jsonStr, '[15,20,30]')
}

function obj2Arr() {
    const obj: SingleANumberFields = { a: 4 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === '' ? [
        (v as SingleANumberFields).a, (v as SingleANumberFields).a * 2] : v)
    arktest.assertEQ(jsonStr, '[4,8]')
}

function indent3() {
    const obj: SingleANumberFields = { a: 15 }
    const jsonStr = JSON.stringify(obj, null, 3)
    arktest.assertEQ(jsonStr, '{\n   "a": 15\n}')
}

function funcEmpty() {
    const func = () => {
    }
    const jsonStr = JSON.stringify(func)
    arktest.assertEQ(jsonStr, '')
}

function toUpper() {
    const obj: TextStringFields = { text: 'hello' }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'text' ? (v as string).toUpperCase() : v)
    arktest.assertEQ(jsonStr, '{"text":"HELLO"}')
}

function indentArrow() {
    const obj: SingleANumberFields = { a: 20 }
    const jsonStr = JSON.stringify(obj, null, '->')
    arktest.assertEQ(jsonStr, '{\n->"a": 20\n}')
}

function indent0b() {
    const obj: SingleANumberFields = { a: 25 }
    const jsonStr = JSON.stringify(obj, null, 0)
    arktest.assertEQ(jsonStr, '{"a":25}')
}

function nul2Zero() {
    const arr = [1, null, 3]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => v === null ? 0 : v)
    arktest.assertEQ(jsonStr, '[1,0,3]')
}

function prefixHash() {
    const obj: SingleANumberFields = { a: 123 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'a' ? `#${v}` : v)
    arktest.assertEQ(jsonStr, '{"a":"#123"}')
}

function indent10d() {
    const obj: SingleANumberFields = { a: 30 }
    const jsonStr = JSON.stringify(obj, null, 10)
    const expected = '{\n          "a": 30\n}'
    arktest.assertEQ(jsonStr, expected)
}

function deepArrStrFlt() {
    const arr = [[[1, 'str'], 3], 4]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => typeof v === 'string' ? undefined : v)
    arktest.assertEQ(jsonStr, '[[[1,null],3],4]')
}

function indentAbc() {
    const obj: SingleANumberFields = { a: 35 }
    const jsonStr = JSON.stringify(obj, null, 'abc123')
    arktest.assertEQ(jsonStr, '{\nabc123"a": 35\n}')
}

function idx1True() {
    const arr = [0, 1, 2]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => Number(k) === 1 ? true : v)
    arktest.assertEQ(jsonStr, '[0,true,2]')
}

function indentTabDash() {
    const obj: SingleANumberFields = { a: 40 }
    const jsonStr = JSON.stringify(obj, null, '\t-')
    arktest.assertEQ(jsonStr, '{\n\t-"a": 40\n}')
}

function nestFltB() {
    const obj: NestedBCAndDFields = { a: { b: 5, c: 6 } as BCNumberFields, d: 7 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'b' ? undefined : v)
    arktest.assertEQ(jsonStr, '{"a":{"c":6},"d":7}')
}

function obj2Arr2() {
    const obj: SingleANumberFields = { a: 3 }
    const jsonStr = JSON.stringify(obj,
        (k: string, v: Any) => k === '' ? [{ a: (v as SingleANumberFields).a } as SingleANumberFields, {
            a: (v as SingleANumberFields).a * 2 } as SingleANumberFields] : v)
    arktest.assertEQ(jsonStr, '[{"a":3},{"a":6}]')
}

function indent6() {
    const obj: SingleANumberFields = { a: 5 }
    const jsonStr = JSON.stringify(obj, null, 6)
    arktest.assertEQ(jsonStr, '{\n      "a": 5\n}')
}

function aNotPos() {
    const obj: SingleANumberFields = { a: 0 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'a' ? v as number > 0 : v)
    arktest.assertEQ(jsonStr, '{"a":false}')
}

function indentStar() {
    const obj: SingleANumberFields = { a: 10 }
    const jsonStr = JSON.stringify(obj, null, '  *  ')
    arktest.assertEQ(jsonStr, '{\n  *  "a": 10\n}')
}

function indentNeg1() {
    const obj: SingleANumberFields = { a: 15 }
    const jsonStr = JSON.stringify(obj, null, -1)
    arktest.assertEQ(jsonStr, '{"a":15}')
}

function arrStrRep2() {
    const arr = ['old1', 'old2', 3]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => typeof v === 'string' ? 'new' : v)
    arktest.assertEQ(jsonStr, '["new","new",3]')
}

function propDup() {
    const obj: TwoNumberFields = { a: 1, b: 2 }
    const jsonStr = JSON.stringify(obj, ['a', 'a', 'b'])
    arktest.assertEQ(jsonStr, '{"a":1,"a":1,"b":2}')
}

function nestFltZ() {
    const obj: NestedYZAndWFields = { x: { y: 5, z: 6 }, w: 7 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === 'z' ? undefined : v)
    arktest.assertEQ(jsonStr, '{"x":{"y":5},"w":7}')
}

function indent10Nest() {
    const obj: NestedBNumberFields = { a: { b: 5 } }
    const jsonStr = JSON.stringify(obj, null, 10)
    const expected = '{\n          "a": {\n                    "b": 5\n          }\n}'
    arktest.assertEQ(jsonStr, expected)
}

function arrNstStrFlt() {
    const arr = [{ nested: { str: 'test', num: 5 } } as NestedStringNumberFields]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => typeof v === 'string' ? undefined : v)
    arktest.assertEQ(jsonStr, '[{"nested":{"num":5}}]')
}

function obj2ArrInc() {
    const obj: SingleANumberFields = { a: 2 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === '' ? [(v as SingleANumberFields).a,
        (v as SingleANumberFields).a + 1] : v)
    arktest.assertEQ(jsonStr, '[2,3]')
}

function indent2Tab() {
    const obj: MessageStringFields = { msg: 'test' }
    const jsonStr = JSON.stringify(obj, null, '\t\t')
    arktest.assertEQ(jsonStr, '{\n\t\t"msg": "test"\n}')
}

function arrFltFunc() {
    const arr = [1, () => {
    }, 3]
    const jsonStr = JSON.stringify(arr, (k: string, v: Any) => typeof v === 'function' ? undefined : v)
    arktest.assertEQ(jsonStr, '[1,null,3]')
}

function nestMul3() {
    const obj: XNumberFields = { x: 4 }
    const jsonStr = JSON.stringify(obj, (k: string, v: Any) => k === '' ? {
        a: { b: (v as XNumberFields).x * 3 } } as NestedABRootFields : v)
    arktest.assertEQ(jsonStr, '{"a":{"b":12}}')
}

