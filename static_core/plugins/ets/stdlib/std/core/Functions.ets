/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

package std.core

function toStrHelper(val: long, radix: int): String {
    if (radix < 2 || radix > 36) {
        throw new RangeError('Radix must be between 2 and 36')
    }
    if (val == 0) {
        return '0'
    }

    // NOTE (igorlegalov): workaround for #26305, replace with 0 when the circular dependency bug is fixed
    const zero: int = (0.0).toInt()
    const negative: boolean = val < 0

    let digitsNum: int = negative ? 1 : zero
    let value: long = val
    if (negative) {
        value /= radix
        digitsNum++
        value = -value
    }
    for (let val: long = value; val > 0; val /= radix) {
        digitsNum++
    }

    const data: FixedArray<char> = new FixedArray<char>(digitsNum)
    let i: int = digitsNum - 1
    if (negative) {
        let remainder: long = -(val % radix)
        data[i--] = digitChars[remainder.toInt()]
        val = -(val / radix)
    }
    while (val > 0) {
        let remainder: long = val % radix
        data[i--] = digitChars[remainder.toInt()]
        val /= radix
    }
    if (negative) {
        data[0] = c'-'
    }
    return new String(data)
}
