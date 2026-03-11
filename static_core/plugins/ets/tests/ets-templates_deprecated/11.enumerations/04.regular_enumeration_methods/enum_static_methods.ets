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

/*---
desc: >-
  Regular enumeration static methods: values(), getValueOf(name),
  fromValue(value), and name indexing by value.
---*/

enum Season {
    Spring,
    Summer,
    Autumn,
    Winter
}

enum Fruit {
    Apple = "APPLE",
    Banana = "BANANA",
    Cherry = "CHERRY"
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Regular Enumeration Static Methods")

    suite.addTest("values() returns all members for int enum", () => {
        let vals = Season.values()
        arktest.assertEQ(vals.length, 4)
        arktest.assertEQ(vals[0], Season.Spring)
        arktest.assertEQ(vals[1], Season.Summer)
        arktest.assertEQ(vals[2], Season.Autumn)
        arktest.assertEQ(vals[3], Season.Winter)
    })

    suite.addTest("values() returns all members for string enum", () => {
        let vals = Fruit.values()
        arktest.assertEQ(vals.length, 3)
        arktest.assertEQ(vals[0], Fruit.Apple)
        arktest.assertEQ(vals[1], Fruit.Banana)
        arktest.assertEQ(vals[2], Fruit.Cherry)
    })

    suite.addTest("getValueOf returns correct member by name", () => {
        let s = Season.getValueOf("Autumn")
        arktest.assertEQ(s, Season.Autumn)
    })

    suite.addTest("getValueOf for string enum", () => {
        let f = Fruit.getValueOf("Banana")
        arktest.assertEQ(f, Fruit.Banana)
    })

    suite.addTest("fromValue returns correct member by value for int enum", () => {
        let s = Season.fromValue(2)
        arktest.assertEQ(s, Season.Autumn)
    })

    suite.addTest("fromValue returns correct member by value for string enum", () => {
        let f = Fruit.fromValue("CHERRY")
        arktest.assertEQ(f, Fruit.Cherry)
    })

    suite.addTest("Name indexing by value for int enum", () => {
        arktest.assertEQ(Season[Season.Spring], "Spring")
        arktest.assertEQ(Season[Season.Winter], "Winter")
    })

    suite.addTest("Name indexing by value for string enum", () => {
        arktest.assertEQ(Fruit[Fruit.Apple], "Apple")
        arktest.assertEQ(Fruit[Fruit.Cherry], "Cherry")
    })

    return suite.run()
}
