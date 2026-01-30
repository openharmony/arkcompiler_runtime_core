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

function setDeleteRemovesValue(): void {
    let set: Set<string> = new Set<string>(4)
    set.add("a")
    set.add("b")
    arktest.assertEQ(set.size, 2)
    arktest.assertTrue(set.has("a"))

    let deleted: boolean = set.delete("a")
    arktest.assertTrue(deleted)
    arktest.assertEQ(set.size, 1)
    arktest.assertFalse(set.has("a"))
    arktest.assertTrue(set.has("b"))

    let values: string[] = []
    for (let v of set.values()) {
        values.push(v)
    }
    arktest.assertEQ(values.length, 1)
    arktest.assertEQ(values[0], "b")
}

function setDeleteMultipleWithoutRehash(): void {
    let set: Set<string> = new Set<string>(8)
    set.add("v1")
    set.add("v2")
    set.add("v3")
    arktest.assertEQ(set.size, 3)

    set.delete("v2")
    arktest.assertEQ(set.size, 2)
    arktest.assertFalse(set.has("v2"))
    arktest.assertTrue(set.has("v1"))
    arktest.assertTrue(set.has("v3"))

    set.delete("v1")
    arktest.assertEQ(set.size, 1)
    arktest.assertFalse(set.has("v1"))
    arktest.assertTrue(set.has("v3"))

    set.delete("v3")
    arktest.assertEQ(set.size, 0)
    arktest.assertFalse(set.has("v3"))
}

function setDeleteNonexistentReturnsFalse(): void {
    let set: Set<string> = new Set<string>(4)
    set.add("a")
    arktest.assertFalse(set.delete("b"))
    arktest.assertEQ(set.size, 1)
    arktest.assertTrue(set.has("a"))
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Set delete clears references')
    suite.addTest('Set delete removes value and clears references', setDeleteRemovesValue)
    suite.addTest('Set delete multiple without rehash', setDeleteMultipleWithoutRehash)
    suite.addTest('Set delete nonexistent returns false', setDeleteNonexistentReturnsFalse)

    return suite.run()
}
