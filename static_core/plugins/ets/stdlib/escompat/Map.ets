/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

export final class Map<K extends Comparable<Object>, V> {
    private tree: Node<K, V> | null;

    /**
     * Constructs an empty Map
     */
    constructor() {
        this.tree = newEmpty<K, V>();
    }

    /**
     * Puts a pair of key and value into the Map
     *
     * @param k the key to put into the Map
     *
     * @param v the value to put into the Map
     */
    set(k: K, v: V): void {
        this.tree = addToTree(this.tree, k, v);
    }

    /**
     * Checks if a key is in the Map
     *
     * @param k the key to find in the Map
     *
     * @returns true if the value is in the Map
     */
    has(k: K): boolean {
        let r = lookupTree(this.tree, k);
        return r != null;
    }

    /**
     * Returns number of Entries with unique keys in the Map
     *
     * @returns number of Entries with unique keys in the Map
     */
    length(): int {
        return count(this.tree);
    }

    /**
     * Removes an Entry with specified key from the Map
     *
     * @param k the key to remove
     */
    delete(k: K): void {
        this.tree = removeFromTree(this.tree, k);
    }

    /**
     * Deletes all elements from the Map
     */
    clear(): void {
        this.tree = newEmpty<K, V>();
    }

    /**
     * Returns a value assosiated with key if present
     *
     * @param k the key to find in the Map
     *
     * @returns value if assosiated with key presents.
     * TODO: specify behaviour if not found
     */
    get(k: K): V {
        let r = lookupTree(this.tree, k);
        if (r != null) {
            return r!;
        } else {
            assert false : "Key not found";
        }
    }

    /**
     * Returns a value assosiated with key if present, or a default value otherwise
     *
     * @param k the key to find in the Map
     *
     * @param def a value to return if key is not in the Map
     *
     * @returns value if key presents, def otherwise
     */
    get(k: K, def: V): V {
        let r = lookupTree(this.tree, k);
        if (r != null) {
            return r!;
        } else {
            return def;
        }
    }

    /**
     * Returns elements from the Map as an keys Iterator
     *
     * @returns ValueIterator with map keys
     */
    keys(): ValuesIterator<K> {
        let en = treeToArray(this.tree);
        let ret = new K[en.length];

        for (let i: int = 0; i < ret.length; i++) {
            ret[i] = en[i].key;
        }

        return new ValuesIterator<K>(ret);
    }

    /**
     * Returns elements from the Map as an values Iterator
     *
     * @returns ValueIterator with map values
     */
    values(): ValuesIterator<V> {
        let en = treeToArray(this.tree);
        let ret = new V[en.length];

        for (let i: int = 0; i < ret.length; i++) {
            ret[i] = en[i].value;
        }

        return new ValuesIterator<V>(ret);
    }

    /**
     * Returns elements from the Map as an array of Entries
     *
     * @returns an array of Entries
     */
    entries(): MapIterator<K, V> {
        return new MapIterator<K, V>(treeToArray(this.tree));
    }

    private static forEachNode<K extends Comparable<Object>, V>(node: Node<K,V> | null, fn: (v: V, k: K) => void): void {
        if (isEmpty<K,V>(node)) {
            return
        }
        Map.forEachNode(node.left, fn)
        fn(node.value, node.key)
        Map.forEachNode(node.right, fn)
    }

    /**
     * Applies a function over all elements of the Map
     *
     * @param fn to apply
     */
    // TODO: return undefined
    forEach(fn: (v: V, k: K) => void): void {
        Map.forEachNode(this.tree, fn)
    }

    /**
     * Applies a function over all elements of the Map
     *
     * @param fn to apply
     */
    forEach(fn: (v: V) => void): void {
        let l: (v: V, k: K) => void = (v: V, k: K): void => { fn(v) }
        Map.forEachNode(this.tree, l)
    }
}
