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

/*---
desc: 05 Generics
name: 05.generics/generic_04
tags: []
---*/

interface Hashable {
    hash(): number
}

class MyHashable implements Hashable {
    override hash(): number {
        return 6;
    }
}

class HasMap<K extends Hashable, V> {
    public set(k: K, v: V): void {
        let key: number = k.hash();
        assertEQ(key, 6)
        assertTrue( k instanceof MyHashable)
    }
}

function main(): void {
    let myHasMap = new HasMap<MyHashable, string>();
    myHasMap.set(new MyHashable(), "F");
}
