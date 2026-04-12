/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
desc: typeof operator with other operators
---*/

function foo() {
    return ['one', 'two']
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Typeof operator precedence")

    // postfix: num++ and num-- mutate num, so increment/decrement pairs stay together
    suite.addTest("postfix increment and decrement", () => {
        let num: number = 1.0
        let str: string = ""

        str = typeof num++
        arktest.assertEQ(str, "number")
        arktest.assertEQ(num, 2.0)
        str = ""

        str = typeof num--
        arktest.assertEQ(str, "number")
        arktest.assertEQ(num, 1.0)
    })

    suite.addTest("postfix typeof typeof", () => {
        let num: number = 1.0
        let str: string = ""

        str = typeof typeof num++
        arktest.assertEQ(str, "string")
        arktest.assertEQ(num, 2.0)
        str = ""

        str = typeof typeof num--
        arktest.assertEQ(str, "string")
        arktest.assertEQ(num, 1.0)
    })

    // prefix: same coupling as postfix
    suite.addTest("prefix increment and decrement", () => {
        let num: number = 1.0
        let str: string = ""

        str = typeof ++num
        arktest.assertEQ(str, "number")
        arktest.assertEQ(num, 2.0)
        str = ""

        str = typeof --num
        arktest.assertEQ(str, "number")
        arktest.assertEQ(num, 1.0)
    })

    suite.addTest("prefix typeof typeof", () => {
        let num: number = 1.0
        let str: string = ""

        str = typeof typeof ++num
        arktest.assertEQ(str, "string")
        arktest.assertEQ(num, 2.0)
        str = ""

        str = typeof typeof --num
        arktest.assertEQ(str, "string")
        arktest.assertEQ(num, 1.0)
    })

    // unary plus: no mutation, each block is independent
    suite.addTest("unary plus typeof", () => {
        let num: number = 1.0

        let str = typeof +num
        arktest.assertEQ(str, "number")
        arktest.assertEQ(num, 1.0)
    })

    suite.addTest("unary plus typeof typeof", () => {
        let num: number = 1.0

        let str = typeof typeof +num
        arktest.assertEQ(str, "string")
        arktest.assertEQ(num, 1.0)
    })

    // unary minus: no mutation
    suite.addTest("unary minus typeof", () => {
        let num: number = 1.0

        let str = typeof -num
        arktest.assertEQ(str, "number")
        arktest.assertEQ(num, 1.0)
    })

    suite.addTest("unary minus typeof typeof", () => {
        let num: number = 1.0

        let str = typeof typeof -num
        arktest.assertEQ(str, "string")
        arktest.assertEQ(num, 1.0)
    })

    // not: no mutation
    suite.addTest("not typeof", () => {
        let bool: boolean = true

        let str = typeof !bool
        arktest.assertEQ(str, "boolean")
        arktest.assertTrue(bool)
    })

    suite.addTest("not typeof typeof", () => {
        let bool: boolean = true

        let str = typeof typeof !bool
        arktest.assertEQ(str, "string")
        arktest.assertTrue(bool)
    })

    // xor: no mutation
    suite.addTest("bitwise complement typeof", () => {
        let t255: int = 255

        let str = typeof ~t255
        arktest.assertEQ(str, "int")
    })

    suite.addTest("bitwise complement typeof typeof", () => {
        let t255: int = 255

        let str = typeof typeof ~t255
        arktest.assertEQ(str, "string")
    })

    // addition: each block resets num, all independent
    suite.addTest("addition typeof plus value", () => {
        let num: number = 42.0

        let str = typeof num + num
        arktest.assertEQ(str, "number42")
    })

    suite.addTest("addition typeof typeof plus value", () => {
        let num: number = 42.0

        let str = typeof typeof num + num
        arktest.assertEQ(str, "string42")
    })

    suite.addTest("addition typeof plus typeof", () => {
        let num: number = 42.0

        let str = typeof num + typeof num
        arktest.assertEQ(str, "numbernumber")
    })

    suite.addTest("addition typeof plus typeof typeof", () => {
        let num: number = 42.0

        let str = typeof num + typeof typeof num
        arktest.assertEQ(str, "numberstring")
    })

    // cast: each assignment to obj is independent
    suite.addTest("cast typeof as Object", () => {
        let num: number = 1.0

        let obj: Object = typeof num as Object
        arktest.assertEQ(obj as string, "number")
    })

    suite.addTest("cast typeof typeof as String", () => {
        let str: string = ""

        let obj: Object = typeof typeof str as String
        arktest.assertEQ(obj as string, "string")
    })

    suite.addTest("cast typeof typeof as String as Object", () => {
        let str: string = ""

        let obj: Object = typeof typeof str as String as Object
        arktest.assertEQ(obj as string, "string")
    })

    // instanceof: typeof result vs array
    suite.addTest("typeof instanceof Object", () => {
        let pt = foo()
        arktest.assertTrue(typeof pt instanceof Object)
    })

    suite.addTest("typeof not instanceof Array", () => {
        let pt = foo()
        arktest.assertFalse(typeof pt instanceof Array)
    })

    suite.addTest("value instanceof Array", () => {
        let pt = foo()
        arktest.assertTrue(pt instanceof Array)
    })

    // relational: grouped by typeof depth
    suite.addTest("relational typeof vs typeof", () => {
        let num: number = 1.0
        let str: string = ""

        arktest.assertTrue(typeof num < typeof str)
        arktest.assertTrue(typeof num <= typeof str)
        arktest.assertFalse(typeof num == typeof str)
    })

    suite.addTest("relational typeof typeof vs typeof", () => {
        let num: number = 1.0
        let str: string = ""

        arktest.assertTrue(typeof typeof num == typeof str)
        arktest.assertTrue(typeof typeof num <= typeof str)
        arktest.assertTrue(typeof typeof num >= typeof str)
        arktest.assertFalse(typeof typeof num < typeof str)
        arktest.assertFalse(typeof typeof num > typeof str)
    })

    suite.addTest("relational typeof typeof vs typeof typeof", () => {
        let num: number = 1.0
        let str: string = ""

        arktest.assertFalse(typeof typeof num != typeof typeof str)
        arktest.assertFalse(typeof typeof num > typeof typeof str)
        arktest.assertTrue(typeof typeof num >= typeof typeof str)
        arktest.assertFalse(typeof typeof num < typeof typeof str)
        arktest.assertTrue(typeof typeof num <= typeof typeof str)
    })

    // ternary: each block resets str, all independent
    suite.addTest("ternary less than", () => {
        let num: number = 1.0
        let str: string = ""

        str = typeof num < typeof str ? typeof num : typeof str
        arktest.assertEQ(str, "number")
    })

    suite.addTest("ternary greater than", () => {
        let num: number = 1.0
        let str: string = ""

        str = typeof num > typeof str ? typeof num : typeof str
        arktest.assertEQ(str, "string")
    })

    suite.addTest("ternary mixed", () => {
        let num: number = 1.0
        let str: string = ""

        str = typeof str < typeof typeof str ? typeof typeof num : typeof num + 1
        arktest.assertEQ(str, "number1")
    })

    // assignments: each block resets s and str, all independent
    suite.addTest("assignment typeof", () => {
        let num: number = 1.0

        let s = "1"
        let str = s += typeof num
        arktest.assertEQ(str, "1number")
        arktest.assertEQ(s, "1number")
    })

    suite.addTest("assignment typeof typeof", () => {
        let num: number = 1.0

        let s = "1"
        let str = s += typeof typeof num
        arktest.assertEQ(str, "1string")
        arktest.assertEQ(s, "1string")
    })

    suite.addTest("assignment typeof typeof plus typeof", () => {
        let num: number = 1.0

        let s = "1"
        let str = s += typeof typeof num + typeof num
        arktest.assertEQ(str, "1stringnumber")
        arktest.assertEQ(s, "1stringnumber")
    })

    return suite.run()
}
