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

function mapDeleteRemovesEntry(): void {
    let map: Map<string|undefined, string> = new Map<string|undefined, string>(4)
    map.set("a", "valA")
    map.set("b", "valB")
    map.set(undefined, "valUndefined")
    arktest.assertEQ(map.size, 3)
    arktest.assertTrue(map.has("a"))
    arktest.assertEQ(map.get("a"), "valA")

    let deleted: boolean = map.delete("a")
    arktest.assertTrue(deleted)
    arktest.assertEQ(map.size, 2)
    arktest.assertFalse(map.has("a"))
    arktest.assertEQ(map.get("a"), undefined)
    arktest.assertTrue(map.has("b"))
    arktest.assertEQ(map.get("b"), "valB")
    arktest.assertTrue(map.has(undefined))
    arktest.assertEQ(map.get(undefined), "valUndefined")

    let keys: Array<string|undefined> = []
    for (let k of map.keys()) {
        keys.push(k)
    }
    arktest.assertEQ(keys.length, 2)
    arktest.assertEQ(keys[0], "b")
    arktest.assertEQ(keys[1], undefined)
}

function mapDeleteMultipleWithoutRehash(): void {
    let map: Map<string, int> = new Map<string, int>(8)
    map.set("k1", 1)
    map.set("k2", 2)
    map.set("k3", 3)
    arktest.assertEQ(map.size, 3)

    map.delete("k2")
    arktest.assertEQ(map.size, 2)
    arktest.assertFalse(map.has("k2"))
    arktest.assertEQ(map.get("k2"), undefined)
    arktest.assertTrue(map.has("k1"))
    arktest.assertTrue(map.has("k3"))

    map.delete("k1")
    arktest.assertEQ(map.size, 1)
    arktest.assertFalse(map.has("k1"))
    arktest.assertTrue(map.has("k3"))
    arktest.assertEQ(map.get("k3"), 3)

    map.delete("k3")
    arktest.assertEQ(map.size, 0)
    arktest.assertFalse(map.has("k3"))
    arktest.assertEQ(map.get("k3"), undefined)
}

function mapDeleteNonexistentReturnsFalse(): void {
    let map: Map<string, int> = new Map<string, int>(4)
    map.set("a", 1)
    arktest.assertFalse(map.delete("b"))
    arktest.assertEQ(map.size, 1)
    arktest.assertTrue(map.has("a"))
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Map delete clears references')
    suite.addTest('Map delete removes entry and clears references', mapDeleteRemovesEntry)
    suite.addTest('Map delete multiple without rehash', mapDeleteMultipleWithoutRehash)
    suite.addTest('Map delete nonexistent returns false', mapDeleteNonexistentReturnsFalse)

    return suite.run()
}
