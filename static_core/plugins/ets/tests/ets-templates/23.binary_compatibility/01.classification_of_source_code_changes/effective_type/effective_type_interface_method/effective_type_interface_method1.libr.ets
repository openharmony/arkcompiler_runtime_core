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
desc: Old lib version
tags: [compile-only, not-a-test]
---*/
export class C<T> {
    public fld: T
    constructor(p: T) {
        this.fld = p
    }
}

export interface I {
    // Effective type: Array
    mArray(): number[] {
        return [1]
    }

    // Effective type: FixedArray<byte|undefined>
    mFixedArray(): FixedArray<byte|undefined> {
        return [1, 2]
    }

    // Effective type: Tuple3
    mTuple(): [int, number, string] {
        return [1, 1.0, 'a']
    }

    // Effective type: C
    mC(): Readonly<C<string>> {
        return new C<string>('abc')
    }
}
