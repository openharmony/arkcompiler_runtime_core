/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

class Key {
    readonly id: int

    constructor(id: int) {
        this.id = id
    }
}

function weakSetOperations(): void {
    const weakSet = new WeakSet<Key>()

    const id3 = new Key(3)

    weakSet.add(id3)
    assertTrue(weakSet.has(id3))

    assertTrue(weakSet.delete(id3))
    assertFalse(weakSet.delete(id3))

    assertFalse(weakSet.has(id3))
}

function newWeakSetUsingElementsArray(): void {
    const elem1 = new Key(21)
    const elem2 = new Key(22)

    const weakSet = new WeakSet<Key>([elem1, elem2])

    assertTrue(weakSet.has(elem1))
    assertTrue(weakSet.has(elem2))
}

function newWeakSetUsingElementsIterable(): void {
    const elem1 = new Object()
    const elem2 = new Object()

    const elems: Iterable<Object> = Array.of<Object>(elem1, elem2)
    const weakSet = new WeakSet<Object>(elems)

    assertTrue(weakSet.has(elem1))
}

function newWeakSetUsingNullElements(): void {
    const weakSet = new WeakSet<Key>(null)
    
    const elem1 = new Key(31)
    weakSet.add(elem1)

    assertTrue(weakSet.has(elem1))
}

function  newWeakSetUsingUndefinedElements(): void {
    const weakSet = new WeakSet<Key>(undefined)
}

function main(): int {
    const suite = new ArkTestsuite("WeakSet API tests")
    suite.addTest("new WeakSet using elements array", newWeakSetUsingElementsArray)
    suite.addTest("new WeakSet using elements iterable", newWeakSetUsingElementsIterable)
    suite.addTest("WeakSet operations", weakSetOperations)
    suite.addTest("new WeakSet using null elements", newWeakSetUsingNullElements)
    suite.addTest("new WeakSet using undefined elements", newWeakSetUsingUndefinedElements)

    return suite.run()
}
