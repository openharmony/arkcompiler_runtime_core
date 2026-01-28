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
desc: Use instanceof to distinguish between specific error types in a catch block.
---*/

class UnknownError extends Error {
    error: Error
    constructor (error: Error) {
        super()
        this.error = error
    }
}

function get_array_element<T>(array: T[], index: int): T | null {
    try {
        return array[index]
    } catch (error) {
        if (error instanceof RangeError) {
            return null
        }
        throw new UnknownError(error)
    }
}

function main(): void {
    let arr: number[] = [10, 20, 30]

    let val0 = get_array_element<number>(arr, 0)
    arktest.assertEQ(val0, 10)

    let val2 = get_array_element<number>(arr, 2)
    arktest.assertEQ(val2, 30)

    let valOut = get_array_element<number>(arr, 100)
    arktest.assertEQ(valOut, null)
}
