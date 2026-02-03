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
 * @file Defines the ArrayLike for ArkTS
 * @kit ArkTS
 */

package std.core;

class ArrayLikeIterator<T> implements IterableIterator<T> {
    private base: ArrayLike<T>
    private index: int = 0

    constructor(base: ArrayLike<T>) {
        this.base = base
    }

    override next(): IteratorResult<T> {
        if (this.index >= this.base.length) {
            return new IteratorResult<T>()
        }
        return new IteratorResult<T>(this.base.$_get(this.index++))
    }
}
/**
 * Represents an object that has a length property and can be indexed.
 *
 * @extends Iterable<T>
 * @interface ArrayLike
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export interface ArrayLike<out T> extends Iterable<T> {
    /**
     * Gets the element at the specified index.
     *
     * @param { int } index The zero-based index of the element to get.
     * @returns { T } The element at the specified index.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    $_get(index: int): T

    get length(): int

    /**
     * Returns an iterator for the elements in the ArrayLike object.
     *
     * @returns { IterableIterator<T> } An iterator instance.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    $_iterator(): IterableIterator<T> {
        return new ArrayLikeIterator<T>(this)
    }
}
