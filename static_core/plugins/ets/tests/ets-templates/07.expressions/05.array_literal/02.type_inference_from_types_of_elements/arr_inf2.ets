/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: >-
  If the type of an array literal [ expr1, ... , exprN ] cannot be inferred from the context,
  then it is inferred from the initialization expressions by using the following algorithm.
  The array literal type is an array of elements of the union type.
---*/

class ClassACallable {}

type MyType8 = Long|Double|null|Short|undefined

type F9 = () => number

type MyType19 = (p: string) => number

class ClassA21 {}
class ClassB21 {}
class ClassC21 {}

class ClassA22 {}
class ClassB22 extends ClassA22 {}
class ClassC22 extends ClassB22 {}

interface I25 {}
interface J25 {}
class ClassA25 implements I25 {}
class ClassB25 implements J25 {}

type MyTuple27 = [number, Error]

type TS31 = string|undefined

function main(): int {
    let suite = new arktest.ArkTestsuite("Array literal type inference from element types");

    suite.addTest("Object and Boolean elements", () => {
        let a = [new Object, new Boolean]
        arktest.assertTrue( a instanceof Array )
        arktest.assertTrue( a[0] instanceof Object )
        arktest.assertTrue( a[1] instanceof Object )
    })

    suite.addTest("string from string literals and concatenation", () => {
        let b = ["a", "b" + "c"]
        arktest.assertTrue( b instanceof Array )
        arktest.assertTrue( b[0] instanceof string )
        arktest.assertTrue( b[1] instanceof string )
    })

    suite.addTest("int from numeric literals", () => {
        let c = [1, 2, 3]
        arktest.assertTrue( c instanceof Array )
        arktest.assertTrue( c[0] instanceof int )
        arktest.assertTrue( c[1] instanceof int )
        arktest.assertTrue( c[2] instanceof int )
    })

    suite.addTest("String Int Double union via lambda parameter", () => {
        let x = ["a", 1, 3.14]
        let f = (p: (String|Int|Double)[]) => p
        let d = f(x)
        arktest.assertTrue( d instanceof Array, "instanceof Array failed" )
        arktest.assertTrue( d[0] instanceof (String|Int|Double), "instanceof d[0] failed" )
        arktest.assertTrue( d[1] instanceof (String|Int|Double), "instanceof d[1] failed" )
        arktest.assertTrue( d[2] instanceof (String|Int|Double), "instanceof d[2] failed" )
    })

    suite.addTest("String Int Double union with concatenated string", () => {
        let x = ["a" + "", 1, 3.14]
        let f = (p: (String|Int|Double)[]) => p
        let d = f(x)
        arktest.assertTrue( d instanceof Array, "instanceof d failed" )
        arktest.assertTrue( d[0] instanceof (String|Int|Double), "instanceof d[0] failed" )
        arktest.assertTrue( d[1] instanceof (String|Int|Double), "instanceof d[1] failed" )
        arktest.assertTrue( d[2] instanceof (String|Int|Double), "instanceof d[2] failed" )
    })

    suite.addTest("lambda and class instance union", () => {
        let e = [(): void => {}, new ClassACallable]
        arktest.assertTrue( e instanceof Array )
        arktest.assertTrue( e[0] instanceof ((() => void) | ClassACallable) )
        arktest.assertTrue( e[1] instanceof ((() => void) | ClassACallable) )
    })

    suite.addTest("Long Double Short boxed numerics", () => {
        let y = [new Long(), new Number(), new Short()]
        let f = (p: (Long|Double|Short)[]) => p
        let x = f(y)
        arktest.assertTrue( x instanceof Array )
        arktest.assertTrue( x[0] instanceof Long|Double|Short )
        arktest.assertTrue( x[1] instanceof Long|Double|Short )
        arktest.assertTrue( x[2] instanceof Long|Double|Short )
    })

    suite.addTest("union with null and undefined via type alias", () => {
        let y = [new Long(), new Number(), null, new Short(), undefined]
        let f = (p: MyType8[]) => p
        let x = f(y)
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof MyType8)
        }
    })

    suite.addTest("function references and null union", () => {
        let cnt: number = 0
        let foo = (): number => cnt++
        let bar = (): number => ++cnt
        let baz = (): number => cnt--
        let y = [bar, null, baz, foo]
        let f = (p: (F9|null)[]) => p
        let x = f(y)
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (F9|null))
        }
    })

    suite.addTest("Boolean Int Double union", () => {
        let y = [false, 1, 14, 33.98]
        let f = (p: (Boolean|Int|Double)[]) => p
        let x = f(y)
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (Boolean|Int|Double))
        }
    })

    suite.addTest("narrowing as casts to Long string Short Int", () => {
        let y = [0 as long, "0" as string, 0 as short, 0x1 as int]
        let f = (p: (Long|string|Short|Int)[]) => p
        let x = f(y)
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (Long|string|Short|Int))
        }
    })

    suite.addTest("Object and null union", () => {
        let x = [new Object(), null, new Error(), null]
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (Object|null))
        }
    })

    suite.addTest("Boolean string long union", () => {
        let x = [new Boolean(true), "aaa", new Long()]
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (Boolean|string|long))
        }
    })

    suite.addTest("string and null union", () => {
        let x = ["aa", "bb", null, "cc", null]
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (string|null))
        }
    })

    suite.addTest("string and int mixed", () => {
        let x = [0, 1, 2, "aaa", 3, "bbb"]
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (string|int))
        }
    })

    suite.addTest("only nulls", () => {
        let x = [null, null, null, ]
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof null)
        }
    })

    suite.addTest("only undefineds", () => {
        let x = [undefined, undefined]
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof undefined)
        }
    })

    suite.addTest("null and undefined union", () => {
        let x = [null, undefined]
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (null|undefined))
        }
    })

    suite.addTest("functional type alias element", () => {
        let x0: MyType19 = (p: string) => { return p.length }
        let x = [x0]
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof MyType19)
        }
    })

    suite.addTest("Boolean string Number union from literals", () => {
        let a = [true, "a", 42.0]
        arktest.assertTrue( a instanceof Array )
        for(let idx = 0; idx < a.length; idx++) {
            arktest.assertTrue(a[idx] instanceof (Boolean|string|Number))
        }
    })

    suite.addTest("three unrelated classes union", () => {
        let a = [{} as ClassA21, {} as ClassB21, {} as ClassC21, {} as ClassA21, {} as ClassB21, {} as ClassC21]
        arktest.assertTrue( a instanceof Array )
        for(let idx = 0; idx < a.length; idx++) {
            arktest.assertTrue(a[idx] instanceof (ClassA21|ClassB21|ClassC21))
        }
    })

    suite.addTest("inheritance hierarchy collapses to base", () => {
        let a = [{} as ClassA22, {} as ClassB22, {} as ClassC22, {} as ClassA22, {} as ClassB22, {} as ClassC22]
        arktest.assertTrue( a instanceof Array )
        for(let idx = 0; idx < a.length; idx++) {
            arktest.assertTrue(a[idx] instanceof ClassA22)
        }
    })

    suite.addTest("string literals with trailing comma", () => {
        let a = ["a", "b", "c", "s", ]
        arktest.assertTrue( a instanceof Array )
        for(let idx = 0; idx < a.length; idx++) {
            arktest.assertTrue(a[idx] instanceof string)
        }
    })

    suite.addTest("string literals plus string variable", () => {
        let s: string = "s"
        let a = ["a", "b", "c", s, ]
        arktest.assertTrue( a instanceof Array )
        for(let idx = 0; idx < a.length; idx++) {
            arktest.assertTrue(a[idx] instanceof string)
        }
    })

    suite.addTest("interfaces and string union", () => {
        let a: I25 = new ClassA25
        let b: J25 = new ClassB25
        let x = [a, b, "a", "b", a, b, "a", "b"]
        arktest.assertTrue( x instanceof Array )
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (I25|J25|string))
        }
    })

    suite.addTest("char and string union", () => {
        let a = c'1'
        let x = [a, c'2', '1', '2',]
        arktest.assertTrue( x instanceof Array)
        for(let idx = 0; idx < x.length; idx++) {
            arktest.assertTrue(x[idx] instanceof (string|Char))
        }
    })

    suite.addTest("union types and tuple element", () => {
        let b: bigint|double = 0n
        let c: double|string = "0"
        let d: MyTuple27 = [1.0, new Error]
        let x = [b, c, d, "1"]
        arktest.assertTrue(x instanceof Array)
        arktest.assertTrue(x[0] instanceof bigint|double)
        arktest.assertTrue(x[1] instanceof double|string)
        arktest.assertTrue(x[2] instanceof Object)
        arktest.assertTrue(x[3] instanceof string)
    })

    suite.addTest("spread of string array", () => {
        let source = ['1', '2', '3']
        let target = [...source]
        arktest.assertTrue(target instanceof Array)
        arktest.assertEQ(3, target.length)
        arktest.assertTrue(target[0] instanceof string)
    })

    suite.addTest("spread of mixed union array", () => {
        let source = [1, true, '3']
        let target = [...source]
        arktest.assertTrue(target instanceof Array)
        arktest.assertEQ(3, target.length)
        arktest.assertTrue(target[0] instanceof string|int|boolean)
    })

    suite.addTest("spread combined with literals", () => {
        let source = ['1', '2', '3']
        let target = [1, true, ...source]
        arktest.assertTrue(target instanceof Array)
        arktest.assertEQ(5, target.length)
        arktest.assertTrue(target[0] instanceof int|string|boolean)
    })

    suite.addTest("spread of nested array literal", () => {
        let source = [['1', '2', '3']]
        let target = [1, true, ...source]
        arktest.assertTrue(target instanceof Array)
        arktest.assertEQ(3, target.length)
        arktest.assertTrue(target[0] instanceof int|Array|boolean)
    })

    suite.addTest("spread of typed alias array", () => {
        let source: TS31[] = ['1', '2', '3']
        let target = [1, true, ...source]
        arktest.assertTrue(target instanceof Array)
        arktest.assertEQ(5, target.length)
        arktest.assertTrue(target[0] instanceof int|boolean|TS31)
    })

    return suite.run()
}
