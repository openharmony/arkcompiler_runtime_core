/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package api;

export namespace util {
    const percentRate = 100;

    export class LRUCache<K, V> {
        private cache: Map<K, V>;
        // Default current size
        private maxSize: number;
        // Default maximum size
        private putCount: long = 0;
        private createCount: long = 0;
        private evictionCount: long = 0;
        private hitCount: long = 0;
        private missCount: long = 0;

        get length(): number {
            return this.cache.size;
        }

        public constructor(capacity?: number) {
            if(capacity == undefined) {
                this.maxSize = 64;
            } else {
                if (this.checkCapacityIsIllegal(capacity)) {
                    throw new BusinessError(`Parameter error. The type of ${capacity} must be small integer`);
                }
                this.maxSize = capacity;
            }
            this.cache = new Map<K, V>();
        }
        
        private checkCapacityIsIllegal(capacity: number): boolean {
            return !(Number.isInteger(capacity) && capacity > 0);
        }
        
        private getLRUKey(): K {
            return this.cache.keys().next().value as K;
        }
        
        private getLRUValue(): V {
            return this.cache.values().next().value as V;
        }
        
        private reduceCapacity(newCapacity: number): void {
            while (this.cache.keys().next() && this.cache.size > newCapacity) {
                let lruKey = this.getLRUKey();
                this.afterRemoval(true, lruKey, this.getLRUValue(), undefined);
                this.cache.delete(lruKey);
                this.evictionCount++;
            }
        }

        protected afterRemoval(isEvict: boolean, key: K | undefined, value: V | undefined,
            newValue: V | undefined): void {
        }
        
        public updateCapacity(newCapacity: number): void {
            if (this.checkCapacityIsIllegal(newCapacity)) {
                throw new BusinessError(`Parameter error. The type of ${newCapacity} must be small integer`);
            } else if (this.cache.size > newCapacity) {
                this.reduceCapacity(newCapacity);
            }
            this.maxSize = newCapacity;
        }
        
        public getCapacity(): number {
            return this.maxSize;
        }
        
        public clear(): void {
            this.cache.forEach((v, k) => {
                this.afterRemoval(false, k, v, undefined);
            })
            this.cache.clear();
        }
        
        public isEmpty(): boolean {
            return (this.cache.size == 0);
        }
        
        public put(key: K, value: V): V|undefined {
            let valueBefore: V|undefined = undefined;
            this.putCount++;
            if (this.cache.has(key)) {
                valueBefore = this.cache.get(key);
                this.cache.delete(key);
                this.afterRemoval(false, key, valueBefore, value);
            }
            this.cache.set(key, value);
            if (this.cache.size > this.maxSize) {
                this.reduceCapacity(this.maxSize);
            }
            return valueBefore;
        }
        
        override toString(): String {
            let peek: number = 0;
            let hitRate: number = 0;
            peek = this.hitCount + this.missCount;
            if (peek != 0) {
                // The value is 100 times larger
                hitRate = percentRate * this.hitCount / peek;
            }
            return 'LRUCache[ maxSize = ' + this.maxSize + ', hits = ' + this.hitCount +
                ', misses = ' + this.missCount + ', hitRate = ' + hitRate + '% ]';
        }
        
        public getCreateCount(): number {
            return this.createCount as number;
        }
        
        public getMissCount(): number {
            return this.missCount as number;
        }

        public getRemovalCount(): number {
            return this.evictionCount as number;
        }

        public getMatchCount(): number {
            return this.hitCount as number;
        }

        public getPutCount(): number {
            return this.putCount as number;
        }
        
        public $_iterator(): IterableIterator<[K, V]> {
            return this.cache.entries();
        }
        
        public entries(): IterableIterator<[K, V]> {
            return this.cache.entries();
        }

        public keys(): Array<K> {
            let arr = Array.from<K>(this.cache.keys());
            return arr;
        }
        
        public values(): Array<V> {
            let arr = Array.from<V>(this.cache.values());
            return arr;
        }
        
        public remove(key: K): V|undefined {
            if (this.cache.has(key)) {
                let former: V|undefined = this.cache.get(key);
                this.cache.delete(key);
                if (former != undefined) {
                    this.afterRemoval(false, key, former, undefined);
                    return former;
                }
            }
            return undefined;
        }
        
        public get(key: K): V|undefined {
            if (this.cache.has(key)) {
                return this.containsInner(key);
            }
            this.missCount++;
            let createValue: V|undefined = this.createDefault(key);
            if (createValue == undefined) {
                return undefined;
            } else {
                this.put(key, createValue);
                this.createCount++;
                return createValue;
            }
            
        }
        
        protected createDefault(key: K): V|undefined {
            return undefined;
        }
        
        private containsInner(key: K): V|undefined {
            this.hitCount++;
            let value: V|undefined = this.cache.get(key);
            this.cache.delete(key);
            this.cache.set(key, value as V);
            return value;
        }
        
        public contains(key: K): boolean {
            if (this.cache.has(key)) {
                this.containsInner(key);
                return true;
            }
            this.missCount++;
            return false;
        }
    }
}