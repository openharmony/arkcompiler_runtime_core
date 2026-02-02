/*
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

/**
 * @file Defines the ConcatArray for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * this is a concatArray interface
 * 
 * @extends ArrayLike<T>
 * @interface ConcatArray
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export interface ConcatArray<out T> extends ArrayLike<T> {

    /**
     * Returns the element at the specified index in the array.
     *
     * @param { int } index The index of the element to return.
     * @returns { T | undefined } The element at the specified index, or undefined if the index is out of bounds.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    at(index: int): T | undefined;

    /**
     * Joins all elements of an array into a string.
     *
     * @param { String } [ separator ] A string used to separate one element of the array from the next in the 
     * resulting string. If omitted, the array elements are separated with a comma.
     * @returns { string } A string containing all the elements of the array joined.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    join(separator?: String): string;

    /**
     * Returns a section of an array.
     *
     * @param { int } [ start ] The beginning of the specified portion of the array.
     * @param { int } [ end ] The end of the specified portion of the array.
     * @returns { Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T> } A new array containing the extracted elements.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    slice(start?: int, end?: int): Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T>;
}
