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
  Array literals can be nested, i.e., the initializer expression that
  specifies an array element can be an array literal if that element is of an array type.
---*/

function wrapInArray(arr: Object[]): Object[] {
    return [arr]
}
function unwrapArray(arr: Object): Object[] {
    return arr as Object[]
}

class SimpleClass {}

class FactoryClass {}
function createPair(): FactoryClass[] {
    return [{}, {}] as FactoryClass[]
}

function sum2D(p: int[][]): int {
    let sum = 0
    for (let i=0; i < p.length; i++) {
        for (let j=0; j < p[i].length; j++) {
            sum += p[i][j]
        }
    }
    return sum
}

function main(): int {
    let suite = new arktest.ArkTestsuite("arr_nest")

    suite.addTest("deeply nested string array literal", () => {
        let a: string[][][][] = [[[["a"]]]]
        arktest.assertEQ(  a[0][0][0][0],  "a" )
    })

    suite.addTest("nesting Object[] inside itself", () => {
        let a: Object[] = [new Number(42.0)]
        a = [a]
        arktest.assertEQ(  (a[0] as Object[])[0] as Number,  42.0 )
    })

    suite.addTest("nesting via function return values", () => {
        let a: Object[] = [new Number(42.0)]
        a = wrapInArray([a])
        a = wrapInArray([a])
        arktest.assertEQ(  unwrapArray(unwrapArray(unwrapArray(unwrapArray(a[0])[0])[0])[0])[0] as Number,  42.0 )
    })

    suite.addTest("nested class array literal with object literals", () => {
        let a: SimpleClass[][] = [[{}, {}], [{}, {}], [{}, {}]]
        arktest.assertTrue(  a[1][1] instanceof SimpleClass )
        arktest.assertEQ(  a.length,  3 )
        arktest.assertEQ(  a[1].length,  2 )
    })

    suite.addTest("nested class array via factory function", () => {
        let a: FactoryClass[][] = [createPair(), createPair(), createPair()]
        arktest.assertTrue(  a[1][1] instanceof FactoryClass )
        arktest.assertEQ(  a.length,  3 )
        arktest.assertEQ(  a[1].length,  2 )
    })

    suite.addTest("nested int[][] passed to function", () => {
        arktest.assertEQ(  sum2D([[1, 11], [2, 22], [3, 33], [4, 44]] as int[][]),   120 )
    })

    return suite.run()
}
