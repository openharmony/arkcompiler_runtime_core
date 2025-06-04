/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

function main(): void {
    const s = new Set<int>()
    s.add(1)
    s.add(2)
    s.add(3)
    s.add(1)
    assertEQ( s.size, 3, "size")
    assertTrue( s.has(1), "has 1")
    assertTrue( s.has(2), "has 2")
    assertTrue( s.has(3), "has 3")
    const it = s.values()
    assertEQ( it.next().value!, 1, "it1")
    assertEQ( it.next().value!, 2, "it2")
    assertEQ( it.next().value!, 3, "it3")

    test_bad_hash()
    test_obj()
}

class BadHash {
    value: int
    constructor(value: int) {
        this.value = value
    }

    override $_hashCode(): int {
        return 0
    }
}

function test_bad_hash(): void {
    const badHash0 = new BadHash(0)
    const badHash1 = new BadHash(1)
    const badHash5 = new BadHash(5)
    const badHash30 = new BadHash(30)

    const toAdd = Array.of(badHash0, badHash1, badHash5, badHash30)

    const s = new Set<BadHash>(toAdd)
    s.add(badHash0)
    s.add(badHash1)
    s.add(badHash5)
    s.add(badHash30)

    assertTrue( s.has(badHash0), "has 0")
    assertTrue( s.has(badHash1), "has 1")
    assertTrue( s.has(badHash5), "has 5")
    assertTrue( s.has(badHash30), "has 30")

    assertTrue( !s.has(new BadHash(29)), "!has 29")

    s.clear()
    assertEQ( s.size, 0)
    assertTrue( !s.has(new BadHash(0)), "!has 0")
}


function test_obj(): void {
    const a = new Object()
    const b = new Object()
    const s = new Set<Object>()
    s.add(a)
    assertTrue( !s.has(b))
    assertTrue( s.has(a))

    for (let i = 0; i < 100; i++) {
        const c = new Object()
        if (a.$_hashCode() != c.$_hashCode()) {
            return;
        }
    }
    assertTrue( false, "100 hashes were identical")
}
