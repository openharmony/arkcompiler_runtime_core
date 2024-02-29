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

/**
 * A WeakMap is a collection of key/value pairs whose keys must be
 * objects or non-registered symbols, with values of any arbitrary
 * JavaScript type, and which does not create strong references to its
 * keys.
 */
export final class WeakMap<K, V> {
    /**
     * The WeakMap() constructor creates WeakMap objects.
     */
    constructor() {

    }

    /**
     * The set() method adds a new element with a specified key
     * and value to a WeakMap object.
     */
    set(k: K, v: V): WeakMap<K, V> {
        throw new Error("Not implemented");
        return this;
    }
    /**
     * The has() method returns a boolean indicating whether
     * an element with the specified key exists in the WeakMap
     * object or not.
     */
    has(k: K): boolean {
        throw new Error("Not implemented");
        return false;
    }

    /**
     * The get() method returns a specified element from
     * a WeakMap object.
     */
    get(k: K): V | null {
        throw new Error("Not implemented");
        return null;
    }

    /**
     * The delete() method removes the specified element from
     * a WeakMap object.
     */
    delete(k: K): boolean {
        throw new Error("Not implemented");
        return false;
    }
}
