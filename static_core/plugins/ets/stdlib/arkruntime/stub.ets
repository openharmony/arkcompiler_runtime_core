/**
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

package arkruntime;

@annotation.InternalAPI
export namespace stub {
    export function toValueArray(arr: FixedArray<boolean>): ValueArray<boolean> {
        let result = new ValueArray<boolean>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toFixedArray(arr: ValueArray<boolean>): FixedArray<boolean> {
        let result = new FixedArray<boolean>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toValueArray(arr: FixedArray<byte>): ValueArray<byte> {
        let result = new ValueArray<byte>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toFixedArray(arr: ValueArray<byte>): FixedArray<byte> {
        let result = new FixedArray<byte>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toValueArray(arr: FixedArray<char>): ValueArray<char> {
        let result = new ValueArray<char>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toFixedArray(arr: ValueArray<char>): FixedArray<char> {
        let result = new FixedArray<char>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toValueArray(arr: FixedArray<short>): ValueArray<short> {
        let result = new ValueArray<short>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toFixedArray(arr: ValueArray<short>): FixedArray<short> {
        let result = new FixedArray<short>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toValueArray(arr: FixedArray<int>): ValueArray<int> {
        let result = new ValueArray<int>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toFixedArray(arr: ValueArray<int>): FixedArray<int> {
        let result = new FixedArray<int>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toValueArray(arr: FixedArray<long>): ValueArray<long> {
        let result = new ValueArray<long>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toFixedArray(arr: ValueArray<long>): FixedArray<long> {
        let result = new FixedArray<long>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toValueArray(arr: FixedArray<float>): ValueArray<float> {
        let result = new ValueArray<float>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toFixedArray(arr: ValueArray<float>): FixedArray<float> {
        let result = new FixedArray<float>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }

    export function toValueArray(arr: FixedArray<double>): ValueArray<double> {
        let result = new ValueArray<double>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }



    export function toFixedArray(arr: ValueArray<double>): FixedArray<double> {
        let result = new FixedArray<double>(arr.length)
        for (let i = 0; i < arr.length; ++i) {
            result[i] = arr[i]
        }
        return result
    }


}
