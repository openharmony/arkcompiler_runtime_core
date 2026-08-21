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
desc: New lib version. Effective types are the same as in the old lib.
tags: [compile-only, not-a-test]
---*/
export class C<T> {
    public fld: T
    constructor(p: T) {
        this.fld = p
    }
}

export class A {
    // Effective type: Array
    public static fldArray: Array<string> = ['a']

    // Effective type: FixedArray<byte|undefined>
    public static fldFixedArray: FixedArray<byte|undefined> = [3, 4]

    // Effective type: Tuple3
    public static fldTuple: [string, boolean, int] = ['a', false, 42]

    // Effective type: C
    public static fldC: C<number> = new C<number>(42.0)
}
