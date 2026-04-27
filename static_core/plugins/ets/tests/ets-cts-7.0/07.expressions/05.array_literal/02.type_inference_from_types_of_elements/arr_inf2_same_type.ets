/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

/*---
desc: If all initialization expressions are of the same type T, then the type of the array literal is T[]
---*/

class ClassA {}

interface I {}
class ClassAImpl implements I {}

enum Color { Red, Green, Blue }

type FNumNum = (p: number) => number
type FNum = () => number
type FStrStr = (p: string) => string
type T = [number, string]

function main(): int {
    let suite = new arktest.ArkTestsuite("Array literal type inference from elements of same type");

    suite.addTest("array of new Object", () => {
        arktest.assertTrue([new Object, new Object] instanceof Array)
    })

    suite.addTest("nested array of new Object", () => {
        arktest.assertTrue([[new Object], [new Object]] instanceof Array)
    })

    suite.addTest("array of Object variables", () => {
        let a = new Object
        const b = new Object
        arktest.assertTrue([a, b, ] instanceof Array)
    })

    suite.addTest("nested array mixing Object array and variables", () => {
        let a = [new Object]
        const b = new Object
        arktest.assertTrue([a, [new Object], [b]] instanceof Array)
    })

    suite.addTest("array of new Error", () => {
        arktest.assertTrue([new Error, new Error, new Error] instanceof Array)
    })

    suite.addTest("nested array of Error variables", () => {
        let a: Error = new Error
        const b = new Error
        let c = b
        arktest.assertTrue([[c], [b], [new Error], [a], [new Error], ] instanceof Array)
    })

    suite.addTest("array of string literals", () => {
        arktest.assertTrue(['a', 'b', 'c'] instanceof Array)
    })

    suite.addTest("nested array of string literals", () => {
        arktest.assertTrue([['a'], ['b'], ['c']] instanceof Array)
    })

    suite.addTest("array of string variables", () => {
        let a: string = 'a'
        let b: string = 'b'
        const c: string = 'c'
        arktest.assertTrue([a, b, c] instanceof Array)
    })

    suite.addTest("nested array of string variables", () => {
        const a: string = 'a'
        let b: string = 'b'
        let c: string = 'c'
        arktest.assertTrue([[a], [b], [c]] instanceof Array)
    })

    suite.addTest("array mixing string variables and literals", () => {
        let a: string = 'a'
        let b: string = 'b'
        const c: string = 'c'
        arktest.assertTrue([a, b, c, 'd', 'e', ] instanceof Array)
    })

    suite.addTest("nested array mixing string variables and literals", () => {
        const a: string = 'a'
        let b: string = 'b'
        let c: string = 'c'
        arktest.assertTrue([['1'], ['2'], [a], [b], [c]] instanceof Array)
    })

    suite.addTest("array of string function calls", () => {
        let foo = (p: string): string => p + p
        arktest.assertTrue([foo('a'), foo('b'), foo('c')] instanceof Array)
    })

    suite.addTest("nested array of string function calls", () => {
        let foo = (p: string): string => p + p
        arktest.assertTrue([[foo('a')], [foo('b')], [foo('c')]] instanceof Array)
    })

    suite.addTest("array of user class instances", () => {
        let a = new ClassA
        const b = new ClassA
        let c = b
        arktest.assertTrue([{} as ClassA, a, new ClassA, b, c, ] instanceof Array)
    })

    suite.addTest("nested array of user class instances", () => {
        let a = new ClassA
        const b = new ClassA
        let c = b
        arktest.assertTrue([[c], [{} as ClassA], [a], [new ClassA], [b]] instanceof Array)
    })

    suite.addTest("array of interface implementations", () => {
        let a = new ClassAImpl
        arktest.assertTrue([{} as ClassAImpl, a, new ClassAImpl] instanceof Array)
    })

    suite.addTest("nested array of interface implementations", () => {
        let a = new ClassAImpl
        arktest.assertTrue([[{} as ClassAImpl], [a], [new ClassAImpl]] instanceof Array)
    })

    suite.addTest("array of enum values", () => {
        arktest.assertTrue([Color.Red, Color.Green] instanceof Array)
    })

    suite.addTest("nested array of enum values", () => {
        arktest.assertTrue([[Color.Red], [Color.Green]] instanceof Array)
    })

    suite.addTest("array of function references", () => {
        let foo = (p: number): number => p + p
        arktest.assertTrue([foo, foo, foo] instanceof Array)
    })

    suite.addTest("array variable of function references", () => {
        let cnt: number
        let foo = (): number => cnt++
        let bar = (): number => ++cnt
        let baz = (): number => cnt--
        let a = [foo, bar, baz, foo]
        arktest.assertTrue(a instanceof Array)
    })

    suite.addTest("array mixing function reference and lambda", () => {
        let foo = (p: number): number => p + p
        arktest.assertTrue([foo, foo, (p: number): number => 0] instanceof Array)
    })

    suite.addTest("nested array mixing function reference and lambda", () => {
        let foo = (p: number): number => p + p
        arktest.assertTrue([[(p: number): number => 0], [foo], [foo]] instanceof Array)
    })

    suite.addTest("array of empty lambdas", () => {
        arktest.assertTrue([() => {}, () => {}] instanceof Array)
    })

    suite.addTest("nested array of empty lambdas", () => {
        arktest.assertTrue([[() => {}], [() => {}]] instanceof Array)
    })

    suite.addTest("array of typed lambda variables", () => {
        let a: FStrStr = (p: string): string => p + p
        let b: FStrStr = (p: string): string => p + p + p
        let c: FStrStr = (p: string): string => p + p + p + p
        arktest.assertTrue([a, b, c] instanceof Array)
    })

    suite.addTest("nested array of typed lambda variables", () => {
        let a: FStrStr = (p: string): string => p + p
        let b: FStrStr = (p: string): string => p + p + p
        let c: FStrStr = (p: string): string => p + p + p + p
        arktest.assertTrue([[a], [b], [c]] instanceof Array)
    })

    suite.addTest("array of named tuple type", () => {
        let a: T = [1, '1']
        let b: T = [2, '1']
        arktest.assertTrue([a, b, a, [3, '3'] as T] instanceof Array)
    })

    suite.addTest("nested array of named tuple type", () => {
        let a: T = [1, '1']
        let b: T = [2, '1']
        arktest.assertTrue([[a], [b], [a], [[3, '3'] as T]] instanceof Array)
    })

    suite.addTest("array of inline tuple type", () => {
        let a: [number, string] = [1, '1']
        let b: [number, string] = [2, '1']
        arktest.assertTrue([a, b, a] instanceof Array)
    })

    suite.addTest("nested array of inline tuple type", () => {
        let a: [number, string] = [1, '1']
        let b: [number, string] = [2, '1']
        arktest.assertTrue([[a], [b], [a]] instanceof Array)
    })

    suite.addTest("array of boolean literals and variables", () => {
        let a = false
        const b = true
        arktest.assertTrue([a, true, true, b, false,] instanceof Array)
    })

    suite.addTest("nested array of boolean literals and variables", () => {
        let a = false
        const b = true
        arktest.assertTrue([[true], [true], [a], [false], [b]] instanceof Array)
    })

    suite.addTest("array of Boolean objects", () => {
        let a = new Boolean(false)
        const b = new Boolean(true)
        arktest.assertTrue([new Boolean(), a, new Boolean(), new Boolean(), b, ] instanceof Array)
    })

    suite.addTest("nested array of Boolean objects", () => {
        let a = new Boolean(false)
        const b = new Boolean(true)
        arktest.assertTrue([[b,], [new Boolean(),], [a,], [new Boolean(),], [new Boolean(),], ] instanceof Array)
    })

    suite.addTest("array of nullish Error", () => {
        let a: Error|undefined
        let b: Error|undefined = new Error
        arktest.assertTrue([a, b] instanceof Array)
    })

    suite.addTest("nested array of nullish Error", () => {
        let a: Error|undefined
        let b: Error|undefined = new Error
        arktest.assertTrue([[a], [b]] instanceof Array)
    })

    suite.addTest("array of nullish Error duplicate", () => {
        let a: Error|undefined
        let b: Error|undefined = new Error
        arktest.assertTrue([a, b] instanceof Array)
    })

    suite.addTest("nested array of nullish Error duplicate", () => {
        let a: Error|undefined
        let b: Error|undefined = new Error
        arktest.assertTrue([[a], [b]] instanceof Array)
    })

    suite.addTest("array of char literals", () => {
        arktest.assertTrue([c'1', c'2'] instanceof Array)
    })

    suite.addTest("array of char variables", () => {
        let a: char = c'a'
        const b: char = c'b'
        let c = b
        arktest.assertTrue([c, b, a] instanceof Array)
    })

    suite.addTest("array mixing char variables and literals", () => {
        let a: char = c'a'
        const b: char = c'b'
        let c = b
        arktest.assertTrue([c'd', c, c'e', b, a] instanceof Array)
    })

    suite.addTest("array of Char objects", () => {
        let a: Char = new Char(c'a')
        const b: Char = new Char(c'b')
        let c = b
        arktest.assertTrue([c, b, a,] instanceof Array)
    })

    return suite.run()
}
