/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Issue 16364. Different result to convert number to byte
tags: []
---*/


function main() {
    let n: Number = 4294967296.0

    let i1: Int = n.toInt();
    let i2: Int = Double.toInt(4294967296.0)

    let s1: Short = n.toShort();
    let s2: Short = Double.toShort(4294967296.0)

    let b1: Byte = n.toByte();
    let b2: Byte = Double.toByte(4294967296.0)

    assertEQ(i1, i2)
    assertEQ(Int.MAX_VALUE, i2, "Ints compare") //the result is the largest representable value of the target type.

    assertEQ(s1, s2)
    assertEQ(-1, s2, "Shorts compare") //discards all bits except the N lowest ones, where N is the number of bits used to represent type

    assertEQ(b1, b2)
    assertEQ(-1, b2, "Bytes compare") //discards all bits except the N lowest ones, where N is the number of bits used to represent type

}
