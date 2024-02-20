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
 * A WeakSet is a collection of garbage-collectable values.  A value
 * in the WeakSet may only occur once. It is unique in the WeakSet's
 * collection.
 */
export final class WeakSet<K> {
    /**
     * The WeakSet() constructor creates WeakSet objects.
     */
    constructor() {

    }

    /**
     * The add() method appends a new object to the end of a WeakSet
     * object.
     */
    add(v: K): WeakSet<K> {
        throw new Error("Not implemented");
        return this;
    }

    /**
     * The has() method returns a boolean indicating whether an object
     * exists in a WeakSet or not.
     */
    has(v: K): boolean {
        throw new Error("Not implemented");
        return false;
    }

    /**
     * The delete() method removes the specified element from
     * a WeakSet object.
     */
    delete(v: K): boolean {
        throw new Error("Not implemented");
        return false;
    }
}
