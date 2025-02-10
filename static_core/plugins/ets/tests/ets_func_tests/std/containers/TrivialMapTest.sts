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
    const s = new Map<int, int>()
    s.set(1, 1)
    s.set(2, 2)
    s.set(3, 3)
    s.set(1, 4)
    assertEQ( s.size, 3, "size")
    assertTrue( s.has(1), "has 1")
    assertTrue( s.has(2), "has 2")
    assertTrue( s.has(3), "has 3")
    const it = s.values()
    assertEQ( it.next().value!, 4, "it1")
    assertEQ( it.next().value!, 2, "it2")
    assertEQ( it.next().value!, 3, "it3")

    const kit = s.keys()
    assertEQ( kit.next().value!, 1, "kit1")
    assertEQ( kit.next().value!, 2, "kit2")
    assertEQ( kit.next().value!, 3, "kit3")
}
