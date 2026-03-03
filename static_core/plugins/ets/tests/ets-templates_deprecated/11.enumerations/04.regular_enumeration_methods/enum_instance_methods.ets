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
  Regular enumeration instance methods: toString(), valueOf(), getName(),
  and the relationship toString() == valueOf().toString().
---*/

enum Color {
    Red,
    Green,
    Blue
}

enum Planet {
    Mercury = "MERCURY",
    Venus = "VENUS",
    Earth = "EARTH"
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Regular Enumeration Instance Methods")

    suite.addTest("valueOf() returns underlying int value", () => {
        arktest.assertEQ(Color.Red.valueOf(), 0)
        arktest.assertEQ(Color.Green.valueOf(), 1)
        arktest.assertEQ(Color.Blue.valueOf(), 2)
    })

    suite.addTest("valueOf() returns underlying string value", () => {
        arktest.assertEQ(Planet.Mercury.valueOf(), "MERCURY")
        arktest.assertEQ(Planet.Venus.valueOf(), "VENUS")
        arktest.assertEQ(Planet.Earth.valueOf(), "EARTH")
    })

    suite.addTest("toString() for int enum returns string of value", () => {
        arktest.assertEQ(Color.Red.toString(), "0")
        arktest.assertEQ(Color.Blue.toString(), "2")
    })

    suite.addTest("toString() for string enum returns the string value", () => {
        arktest.assertEQ(Planet.Earth.toString(), "EARTH")
    })

    suite.addTest("getName() returns member name for int enum", () => {
        arktest.assertEQ(Color.Red.getName(), "Red")
        arktest.assertEQ(Color.Green.getName(), "Green")
        arktest.assertEQ(Color.Blue.getName(), "Blue")
    })

    suite.addTest("getName() returns member name for string enum", () => {
        arktest.assertEQ(Planet.Mercury.getName(), "Mercury")
        arktest.assertEQ(Planet.Earth.getName(), "Earth")
    })

    suite.addTest("toString() equals valueOf().toString() for int enum", () => {
        let c = Color.Green
        arktest.assertEQ(c.toString(), c.valueOf().toString())
    })

    suite.addTest("toString() equals valueOf().toString() for string enum", () => {
        let p = Planet.Venus
        arktest.assertEQ(p.toString(), p.valueOf().toString())
    })

    return suite.run()
}
