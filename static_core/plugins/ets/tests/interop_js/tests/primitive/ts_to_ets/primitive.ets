/**
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

function checkBoolean(val: boolean) {
    assertTrue(val == true)
}

function checkBooleanBoxed(val: Boolean) {
    assertTrue(val == true)
}

function checkByte(val: byte) {
    assertTrue(val == 1)
}

function checkByteOverflow(val: byte, oriVal: int) {
    assertTrue(val == Int.toByte(oriVal))
}

function checkByteBoxed(val: Byte) {
    assertTrue(val == 1)
}

function checkShort(val: short) {
    assertTrue(val == 1)
}

function checkShortBoxed(val: Short) {
    assertTrue(val == 1)
}

function checkShortOverflow(val: short, oriVal: int) {
    assertTrue(val == Int.toShort(oriVal))
}

function checkInt(val: int) {
    assertTrue(val == 1)
}

function checkIntBoxed(val: Int) {
    assertTrue(val == 1)
}

function checkIntOverflow(val: int, oriVal: long) {
    assertTrue(val == Long.toInt(oriVal))
}

function checkLong(val: long) {
    assertTrue(val == 1)
}

function checkLongBoxed(val: Long) {
    assertTrue(val == 1)
}

function checkFloat(val: float) {
     assertTrue(val == 2.1f)
}

function checkFloatBoxed(val: Float) {
    assertTrue(val == 2.1f)
}

function checkFloatOverflow(val: float, oriVal: number) {
    assertTrue(val == Double.toFloat(oriVal))
}

function checkDouble(val: double) {
    assertTrue(val == 1)
}

function checkDoubleBoxed(val: Double) {
    assertTrue(val == 1)
}

function checkNumber(val: number) {
    assertTrue(val == 1)
}

function checkNumberBoxed(val: Number) {
    assertTrue(val == 1)
}

function checkChar(val: char) {
    assertTrue(val == c'1')
}

function checkCharBoxed(val: Char) {
    assertTrue(val.unboxed() == c'1')
}

function checkString(val: string) {
    assertTrue(val == '1')
}

function checkStringBoxed(val: String) {
    assertTrue(val == '1')
}

function checkBigint(val: bigint) {
    assertTrue(val == 1n)
}

function checkBigintBoxed(val: BigInt) {
    assertTrue(val == 1n)
}

function checkNull(val: null) {
    return val === null
}

function checkUndefined(val: null) {
    return val === undefined
}
