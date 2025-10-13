/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

const etalon: long[] = [
3609898888,
2658912113,
2687541264,
2596904264,
2744799565,
2773428716,
964447980,
2830687017,
2859316168,
3626958991,
3517786640,
3546415791,
3452210824,
3603674092,
3632303243,
1819754539,
3689561544,
3718190695,
187298255,
3775448996
]

function testString() {
    const start: double = 42.0
    let num: double = start
    const delta: double = 0.000000001
    const str = "str998"
    const qty: int = 5000

    let map = new Map<string, int>();
    for (let i: int = 0; i < qty; ++i) {
        num += delta
        const key = str + num
        map.set(key, i)
        if (i < etalon.length) {
            const hash = Runtime.getHashCodeByValue(key)
            arktest.assertEQ(hash, etalon[i])
        }
    }
}

////////////////////////////////////////////////////////////////

function main(): int {
    const suite = new arktest.ArkTestsuite("GenerateGetHashCodeByValue tests")

    suite.addTest("Get hash code of string", testString)

    return suite.run()
}