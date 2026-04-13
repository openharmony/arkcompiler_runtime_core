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


    /**
     * Static method that converts primitive boolean to boxed version
     *
     * @param { boolean } b value to be converted
     * @returns { Boolean } The boxed Boolean object representing the input value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export function valueOf(b: boolean): Boolean {
        if (b) {
            return Boolean.TRUE;
        };
        return Boolean.FALSE;
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @param { byte } value value to box
     * @returns { Byte } boxed value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export function valueOf(value: byte): Byte {
        return new Byte(value);
    }

        /**
     * Returns boxed representation of the primitive
     * @param { short } value value to box
     * @returns { short } boxed value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export function valueOf(value: short): Short {
        return new Short(value);
    }

    /**
     * Returns boxed representation of the primitive
     * @param { int } value value to box
     * @returns { int } boxed value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */    
    export function valueOf(value: int): Int {
        return new Int(value);
    }

    /**
     * Returns boxed representation of the primitive
     * @param { long } value value to box
     * @returns { long } boxed value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */   
    export function valueOf(value: long): Long {
        return new Long(value);
    }

    /**
     * Returns boxed representation of the primitive
     * @param { float } value value to box
     * @returns { float } boxed value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */   
    export function valueOf(value: float): Float {
        return new Float(value);
    }

    /**
     * Returns boxed representation of the primitive
     * @param { double } value value to box
     * @returns { double } boxed value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */   
    export function valueOf(value: double): Double {
        return new Double(value);
    }
}
