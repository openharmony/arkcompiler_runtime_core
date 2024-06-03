/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

package escompat;

class ArrayLikeIterator<T> implements IterableIterator<T> {
    private base: ArrayLike<T>
    private index: number = 0

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

export interface ArrayLike<T> extends Iterable<T> {
    $_get(index: number): T;

    get length(): number

    $_iterator(): IterableIterator<T> {
        return new ArrayLikeIterator<T>(this)
    }
}
