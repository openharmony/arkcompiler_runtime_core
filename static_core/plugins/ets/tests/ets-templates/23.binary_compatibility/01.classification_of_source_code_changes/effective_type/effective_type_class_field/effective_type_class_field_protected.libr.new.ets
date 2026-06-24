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
desc: New lib version. Effective type of a protected field are the same as in the old lib.
tags: [compile-only, not-a-test]
---*/

type TOP=Any

export class A {
    // Effective type: string|undefined
    protected fldString: string|undefined = 'abc'

    // Effective type: Tuple2
    protected fldTuple: [byte, string] = [127, 'abc']

    // Effective type: FixedArray<Any>
    protected fldFixedArray: FixedArray<TOP> = [null, undefined, 'abc']

    // Effective type: ValueArray<number>
    protected fldValueArray: ValueArray<number> = [11.1, 22.2]

    // Effective type: Array
    protected fldArray: Array<long|double> = [1 as long, 2.0]
}

export class B extends A {}
