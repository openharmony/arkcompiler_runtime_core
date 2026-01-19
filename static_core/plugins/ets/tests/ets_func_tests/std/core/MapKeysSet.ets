 /*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function check<K, V>(map: Map<K, V>, names: Set<K>) {
    let keys = map.keySet()
    arktest.assertEQ(keys.size, names.size)
    for (let key of keys) {
        arktest.assertTrue(names.has(key))
    }
}

function testMapKeys() {
    let empty = new Map<string, string>()
    let emptyInt = new Map<int, int>()
    let strings = new Map<string, string>([
        ["a", ""],
        ["b", ""]
    ])
    let ints = new Map<int, int>([
        [1, 0],
        [2, 0]
    ])
    check(empty, new Set<string>())
    check(emptyInt, new Set<int>())
    check(strings, new Set<string>(["a", "b"]))
    check(ints, new Set<int>([1, 2]))
}

function testRecordKeys() {
    let empty = new Record<string, string>()
    let emptyInt = new Record<int, int>()
    let strings = new Record<string, string>()
    strings["a"] = ""
    strings["b"] = ""
    let ints = new Record<int, int>()
    ints[1] = 0
    ints[2] = 0
    check(empty, new Set<string>())
    check(emptyInt, new Set<int>())
    check(strings, new Set<string>(["a", "b"]))
    check(ints, new Set<int>([1, 2]))
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Map.keySet")

    suite.addTest("Test map keys", testMapKeys)
    suite.addTest("Test record keys", testRecordKeys)

    return suite.run()
}
