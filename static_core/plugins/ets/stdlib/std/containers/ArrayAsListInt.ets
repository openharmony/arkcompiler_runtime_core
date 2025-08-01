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

package std.containers;

export class ArrayAsListInt implements ListInt {

    private init(capacity: int, val: Int): void {
        this.data = new Int[capacity];
        for (let i = 0; i < this.data.length; ++i) {
            this.data[i] = val;
        }
        this.curSize = capacity;
    }

    /**
     * Constructs new ArrayAsList with respect to capacity and initial value
     *
     * @param capacity
     *
     * @param val
     */
    constructor(capacity: int, val: Int) {
        this.init(capacity, val);
    }

    /**
     * Constructs new empty ArrayAsList
     */
    constructor() {
        this.init(0, new Int());
    }

    /**
     * Constructs new ArrayAsList with required capacity
     *
     * @param capacity
     */
    constructor(capacity: int) {
        this.init(capacity, new Int());
    }

    /**
     * Increases capacity if passed argument is greater than current capacity
     *
     * @param capacity
     */
    public reserve(capacity: int): void {
        if (this.data.length < capacity) {
            let newData : FixedArray<Int> = new Int[capacity];
            for (let i = 0; i < this.curSize; ++i) {
               newData[i] = this.data[i];
            }
            this.data = newData;
        }
    }

    /**
     * Gets a shallow-copy of the underlying array
     *
     * @returns a shallow-copy of the underlying array
     */
    public toArray(): Int[] {
        let data = new Int[this.curSize];
        for (let i = 0; i < this.curSize; ++i) {
            data[i] = this.data[i];
        }
        return data;
    }

    private static getNewCapacity(currentCapacity: int): int {
        // TODO(ivan-tyulyandin): select proper capacity increasing strategy
        const fastGrowThreshold = 8192;
        const multiplier = 2;
        if (currentCapacity < fastGrowThreshold) {
            // Adding 4 to jump over 0
            return (currentCapacity + 4) * multiplier * 2;
        } else {
            return currentCapacity * multiplier;
        }
    }

    /**
     * Pushes a value to the begin of the List
     *
     * @param e value to push
     */
    public override pushFront(e: Int): void {
        let dst = this.data;
        if (this.curSize == this.data.length) {
            dst = new Int[ArrayAsListInt.getNewCapacity(this.data.length)];
        }
        for (let i = this.curSize; i != 0; --i) {
            dst[i] = this.data[i-1];
        }
        this.data = dst;
        this.data[0] = e;
        ++this.curSize;
    }

    /**
     * Pops a value from the begin of the List
     *
     * @returns popped value
     */
    public override popFront(): Int {
        if (this.curSize === 0) {
            throw new AssertionError("No data to popFront from ArrayAsList!")
        }
        let res: Int = this.data[0];
        for (let i = 1; i < this.curSize; ++i) {
            this.data[i-1] = this.data[i];
        }
        --this.curSize;
        return res;
    }

    /**
     * Pushes a value to the end of the List
     *
     * @param e value to push
     */
    public override pushBack(e: Int): void {
        if (this.curSize == this.data.length) {
            let newData : FixedArray<Int> = new Int[ArrayAsListInt.getNewCapacity(this.data.length)];
            for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i];
            }
            this.data = newData;
        }
        this.data[this.curSize] = e;
        ++this.curSize;
    }

    /**
     * Pops a value from the end of the List
     *
     * @returns popped value
     */
    public override popBack(): Int {
        if (this.curSize === 0) {
            throw new AssertionError("No data to popBack from ArrayAsList!")
        }
        --this.curSize;
        return this.data[this.curSize];
    }

    /**
     * Returns number of elements in the List
     */
    public override size(): int {
        return this.curSize;
    }

    /**
     * Returns an element at the specified index
     *
     * @param index element position
     *
     * @returns an element
     */
    public override at(index: int): Int {
        return this.data[index];
    }

    /**
     * Sets an element at the specified index
     *
     * @param index element position
     *
     * @param e new value
     */
    public override set(index: int, e: Int): void {
        this.data[index] = e;
    }

    /**
     * Checks if an element is in the List
     *
     * @param e value to find
     *
     * @returns true if value present, false otherwise
     */
    public override has(e: Int): boolean {
        /*
        for (let i = 0; i < this.curSize; ++i) {
            if (this.data[i].equals(e)) {
                return true;
            }
        }
        */
        throw new AssertionError("Not implemented");
    }

/*  Code below is blocked by internal issue with lambdas #9994
    public forEach(fn: (e: Int): Int): ListInt {
        for (let i = 0; i < this.curSize; ++i) {
            this.data[i] = fn(this.data[i]);
        }
    }

    // public map<U, LU implements List<U>>(fn: (e: T): U): LU {
    public map(fn: (e: Int): ):  {
        let res = new ();
        for (let i = 0; i < this.curSize; ++i) {
            res.push_back(fn(this.data[i]));
        }
        return res;
    }

    public fold(combine: (lhs: Int, rhs: Int): Int): Int {
        if (this.curSize > 0) {
            let res = this.data[0];
            for (let i = 1; i < this.curSize; ++i) {
                res = combine(res, this.data[i]);
            }
            return res;
        }
        throw new NoDataException();
    }

    public foldWith(combine: (lhs: , rhs: Int): , initVal: ):  {
        let res = initVal;
        for (let i = 0; i < this.curSize; ++i) {
            res = combine(res, this.data[i]);
        }
        return res;
    }

    public filter(filterCond: (e: Int): boolean): ArrayAsListInt {
        let indicators = new boolean[data.length];
        let resAmount = 0;
        for (let i = 0; i < this.curSize; ++i) {
            indicators[i] = filterCond(this.data[i]);
            if (indicators[i]) {
                ++resAmount;
            }
        }
        let res = new Int[resAmount];
        for (let i = 0, j = 0; i < this.curSize; ++i) {
            if (indicators[i]) {
                res[j] = this.data[i];
                ++j;
            }
        }
        this.data = res;
        return this;
    }

    public sort(comparator: (lhs: Int, rhs: Int): boolean): ArrayAsListInt {
        sortPart(this.data, 0, this.curSize, comparator);
        return this;
    }

    private static sortPart(arr: Int[], l: int, r: int, comparator: (lhs: Int, rhs: Int): boolean): void {
        if (l < r) {
            // TODO(ivan-tyulyandin): select a proper constant to fall into bubbleSort instead of quickSort
            if (r - l <= 4) {
                bubbleSort(arr, l, r);
                return;
            }
            let part = partition(arr, l, r, comparator);

            sortPart(arr, l, mid, comparator);
            sortPart(arr, mid, r, comparator);
        }
    }

    private static partition(arr: Int[], l: int, r: int, comparator: (lhs: Int, rhs: Int): boolean): int {
        let last = r - 1;
        let pivot = arr[last];
        let lessInd = l - 1;
        for (let i = l; i < last; ++i) {
            if (comparator(arr[i], pivot)) {
                ++lessInd;
                let tmp = arr[i];
                arr[i] = arr[lessInd];
                arr[lessInd] = tmp;
            }
        }
        let tmp = arr[lessInd + 1];
        arr[lessInd + 1] = arr[last];
        arr[last] = tmp;
        return lessInd + 1;
    }

    private static bubbleSort(arr: Int[], l: int, r: int, comparator: (lhs: Int, rhs: Int): boolean): void {
        for (let i = l; i < r; ++i) {
            for (let j = i; j < r - i; ++j) {
                if (comparator(arr[j + 1], arr[j])) {
                    let tmp = arr[j + 1];
                    arr[j + 1] = arr[j];
                    arr[j] = tmp;
                }
            }
        }
    }

*/
    private data: FixedArray<Int> = [];
    private curSize: int;
}
