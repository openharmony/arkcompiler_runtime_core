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
desc: The number of initializer expressions enclosed in braces of the array initializer
    determines the length of the array to be constructed.
---*/

type FUNC = (p: int) => int

function voidFn() {}

class Container<T> {}

enum Color { Red, Green, Blue }

type TT = [string, boolean]

class WithField {
    fld: number = 0
}

function main(): int {
    let suite = new arktest.ArkTestsuite("array_literal")

    suite.addTest("empty array", () => {
        let x: number[] = []
        arktest.assertEQ( x.length, 0 )
    })

    suite.addTest("array of primitives", () => {
        let x: int[] = [1, 2, 3]
        arktest.assertEQ( x.length, 3 )
    })

    suite.addTest("array of boxed", () => {
        let x: Double[] = [new Double(3.1415926)]
        arktest.assertEQ( x.length, 1 )
    })

    suite.addTest("array of strings", () => {
        let x: string[] = ["aaa", "bbb"]
        arktest.assertEQ( x.length, 2 )
    })

    suite.addTest("array of Objects", () => {
        let x: Object[] = ["aaa", "bbb", "aaa", [true, false], [[1, 2, 3], [4, 5, 6]], new Long(), new Short(), new Number(), new Error(), "ccc",]
        arktest.assertEQ( x.length, 10 )
    })

    suite.addTest("array of functions (FUNC type)", () => {
        let x: FUNC[] = [(p:int):int => {return p+1}, (p:int):int => {return p+2}]
        arktest.assertEQ( x.length, 2 )
    })

    suite.addTest("array of functions (void)", () => {
        let x: (() => void)[] = [voidFn, voidFn, voidFn]
        arktest.assertEQ( x.length, 3 )
    })

    suite.addTest("array of arrays", () => {
        let x: Number[][] = [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [7.0, 8.0, 9.0]]
        arktest.assertEQ( x.length, 3 )
    })

    suite.addTest("array of tuples", () => {
        let x: [boolean, Error][] = [[false, new Error()], [true, new Error()]]
        arktest.assertEQ( x.length, 2 )
    })

    suite.addTest("array of unions", () => {
        let x: (Number|null|undefined|String)[] = [3.0, null, "X"]
        arktest.assertEQ( x.length, 3 )
    })

    suite.addTest("array of parameterized objects", () => {
        let x: Container<String>[] = [new Container<String>(), new Container<String>(), new Container<String>()]
        arktest.assertEQ( x.length, 3 )
    })

    suite.addTest("array of enums", () => {
        let x: Color[] = [Color.Red, Color.Green, Color.Blue]
        arktest.assertEQ( x.length, 3 )
    })

    suite.addTest("array of nullish (Long|undefined)", () => {
        let x: (Long|undefined)[] = [new Long((1).toLong()), undefined, new Long((2).toLong()), undefined, new Long((3).toLong())]
        arktest.assertEQ( x.length, 5 )
    })

    suite.addTest("array of nullish (TT|null)", () => {
        let x: (TT|null)[] = [null, null, ["a", true] as TT, ["b", false] as TT, null]
        arktest.assertEQ( x.length, 5 )
    })

    suite.addTest("array of objects", () => {
        let x: WithField[] = [{fld: 1.0}, {fld: 1.0}, {fld: 1.0}, {fld: 1.0}]
        arktest.assertEQ( x.length, 4 )
    })

    return suite.run()
}
