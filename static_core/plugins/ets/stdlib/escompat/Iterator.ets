/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

export class IteratorResult<T> {
    done: boolean
    value: T | undefined

    constructor() {
        this.done = true
        this.value = undefined
    }

    constructor(done: boolean, value: T | undefined) {
        this.done = done
        this.value = value
    }

    constructor(value: T) {
        this.done = false
        this.value = value
    }
}

export interface Iterator<out T> {
    next(): IteratorResult<T>
}

export interface Iterable<out T> {
    $_iterator(): Iterator<T>
}

export interface IterableIterator<out T> extends Iterator<T>, Iterable<T> {
    $_iterator(): IterableIterator<T> {
        return this;
    }
}

function iteratorForEach<V>(x: Iterator<V>, fn: (x: V) => void): void {
    while (true) {
        const v = x.next()
        if (v.done) {
            return
        }
        fn(v.value as V)
    }
}

function tryGetIteratorLength (a: Object): int | null {
    const aType = Type.of(a) as ClassType
    for (let i = 0; i < aType.getMethodsNum(); ++i) {
        let m = aType.getMethod(i)
        if (m.getName() == "__Iterator_getLength" && !m.isStatic() && m.getType().getParametersNum() == 0) {
            return m.invoke(a, [])! as int
        }
    }
    return null
}
