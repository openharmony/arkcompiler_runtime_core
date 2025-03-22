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
    assert(val == true)
}

function checkBooleanBoxed(val: Boolean) {
    assert(val == true)
}

function checkByte(val: byte) {
    assert(val == 1)
}

function checkByteOverflow(val: byte, oriVal: int) {
    assert(val == oriVal as byte)
}

function checkByteBoxed(val: Byte) {
    assert(val == 1)
}

function checkShort(val: short) {
    assert(val == 1)
}

function checkShortBoxed(val: Short) {
    assert(val == 1)
}

function checkShortOverflow(val: short, oriVal: int) {
    assert(val == oriVal as short)
}

function checkInt(val: int) {
    assert(val == 1)
}

function checkIntBoxed(val: Int) {
    assert(val == 1)
}

function checkIntOverflow(val: int, oriVal: long) {
    assert(val == oriVal as int)
}

function checkLong(val: long) {
    assert(val == 1)
}

function checkLongBoxed(val: Long) {
    assert(val == 1)
}

function checkFloat(val: float) {
    assert(val == 1)
}

function checkFloatBoxed(val: Float) {
    assert(val == 1)
}

function checkFloatOverflow(val: float, oriVal: number) {
    assert(val == oriVal as float)
}

function checkDouble(val: double) {
    assert(val == 1)
}

function checkDoubleBoxed(val: Double) {
    assert(val == 1)
}

function checkNumber(val: number) {
    assert(val == 1)
}

function checkNumberBoxed(val: Number) {
    assert(val == 1)
}

function checkChar(val: char) {
    assert(val == c'1')
}

function checkCharBoxed(val: Char) {
    assert(val.unboxed() == c'1')
}

function checkString(val: string) {
    assert(val == '1')
}

function checkStringBoxed(val: String) {
    assert(val == '1')
}

function checkBigint(val: bigint) {
    assert(val == 1n)
}

function checkBigintBoxed(val: BigInt) {
    assert(val == 1n)
}

function checkNull(val: null) {
    return val === null
}

function checkUndefined(val: null) {
    return val === undefined
}