/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

package std.core;

export const KEY_NOT_FOUND: int = -1;

// Range is [startIndex, endIndex), i.e. startIndex included and endIndex is excluded
function checkRange(arrLen: int, startIndex: int, endIndex: int): boolean {
    // Since mostly everywhere for loop is used from startIndex till endIndex exclusive,
    // startIndex <= endIndex is used to cover empty array case
    return ((0 <= startIndex) && (startIndex <= endIndex) && (endIndex <= arrLen));
}

//
// Methods for boolean[]
//

/**
 * fills arr in-place by specified value with respect to passed indexes
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr, exclusive, i.e. arr[endIndex] is not changed
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value, 0, arr.length)
 * ```
 */
export function fill(arr: boolean[], value: boolean, startIndex: int, endIndex: int): void throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("fill: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        arr[i] = value;
    }
}

/**
 * fills arr in-place by specified value
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value)
 * ```
 */
export function fill(arr: boolean[], value: boolean): void {
    try {
        fill(arr, value, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "fill: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
}

/**
 * copies src array into dst with respect to passed indexes.
 * dst must have enough space, otherwise out-of-bounds might occur
 *
 * @param src source array to be copied
 *
 * @param dst destination array
 *
 * @param dstStart index of dst to start from
 *
 * @param srcStart index of src to start from
 *
 * @param srcEnd last index of src to copy, exclusive, i.e. src[srcEnd] is not copied
 *
 * @example: copy src to dst
 * ```
 * copyTo(src, dst, 0, 0, src.length)
 * ```
 */
// TODO(petr-shumilov): make intrinsic
export function copyTo(src: boolean[], dst: boolean[], dstStart: int, srcStart: int, srcEnd: int): void throws {
    if (!checkRange(src.length, srcStart, srcEnd)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: src bounds verification failed")
    }
    if (!checkRange(dst.length, dstStart, dst.length)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: dst bounds verification failed")
    }
    if (!((srcEnd - srcStart) <= (dst.length - dstStart))) {
        throw new ArrayIndexOutOfBoundsException("Destination array must have enough space")
    }

    let j: int = dstStart;
    for (let i: int = srcStart; i < srcEnd; i++) {
        dst[j] = src[i];
        j++;
    }
}

/**
 * creates a copy of src array with respect to passed indexes.
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr to copy, exclusive, i.e. arr[endIndex] is not copied
 *
 * @returns copy of arr[startIndex; endIndex)
 *
 * @example: create exact copy of arr
 * ```
 * copyOf(arr, 0, arr.length)
 * ```
 */
export function copyOf(arr: boolean[], startIndex: int, endIndex: int): boolean[] throws {
    let res: boolean[] = new boolean[endIndex - startIndex];
    copyTo(arr, res, 0, startIndex, endIndex);
    return res;
}

/*
 * creates a copy of src array with respect to passed index
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of src to start from
 *
 * @returns copy of arr[startIndex; src.length)
 *
 * @example: create copy of arr without first element
 * ```
 * copyOf(arr, 1)
 * ```
 */
export function copyOf(arr: boolean[], startIndex: int): boolean[] throws {
    return copyOf(arr, startIndex, arr.length);
}

/*
 * copyOf(arr: boolean[]) creates a copy of src array
 *
 * @param arr source array to be copied
 *
 * @returns copy of arr
 */
export function copyOf(arr: boolean[]): boolean[] {
    try {
        return copyOf(arr, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "copyOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return new boolean[0]
}


function bubbleSort(arr: boolean[], startIndex: int, endIndex: int): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (((arr[i + 1]) ? 1 : 0) < ((arr[i]) ? 1 : 0)) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: boolean[], startIndex: int, endIndex: int): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    countSortBools(arr, startIndex, endIndex)
}


function bubbleSort(arr: boolean[], startIndex: int, endIndex: int, mustPrecede: (lhs: boolean, rhs: boolean) => boolean): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (mustPrecede(arr[i + 1], arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: boolean[], startIndex: int, endIndex: int, mustPrecede: (lhs: boolean, rhs: boolean) => boolean): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    if (mustPrecede(false, true)) {
        countSortBools(arr, startIndex, endIndex)
    } else {
        countSortBoolsInv(arr, startIndex, endIndex)
    }
}



/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 */
export function sort(arr: boolean[], mustPrecede: (lhs: boolean, rhs: boolean) => boolean): void {
    sort(arr, 0, arr.length, mustPrecede);
}

export function sort(arr: boolean[], startIndex: int, mustPrecede: (lhs: boolean, rhs: boolean) => boolean): void {
    sort(arr, startIndex, arr.length, mustPrecede)
}


function countSortTruthCnt(arr: boolean[], startIndex: int, endIndex: int): int {
    let truthCnt = 0
    for (let i = startIndex; i < endIndex; i++) {
        if (arr[i]) {
            truthCnt++
        }
    }
    return truthCnt
}

function countSortBools(arr: boolean[], startIndex: int, endIndex: int): void {
    const truthCnt = countSortTruthCnt(arr, startIndex, endIndex)
    for (let i = startIndex; i < endIndex - truthCnt; i++) {
            arr[i] = false
        }
    for (let i = 0; i < truthCnt; i++) {
        arr[endIndex - truthCnt + i] = true
    }
}
function countSortBoolsInv(arr: boolean[], startIndex: int, endIndex: int): void {
    const truthCnt = countSortTruthCnt(arr, startIndex, endIndex)
    for (let i = 0; i < truthCnt; i++) {
        arr[startIndex + i] = true
    }
    for (let i = startIndex + truthCnt; i < endIndex; i++) {
        arr[i] = false
    }
}

export function sort(arr: boolean[], startIndex: int): void {
    sort(arr, startIndex, arr.length)
}

export function sort(arr: boolean[]): void {
    sort(arr, 0, arr.length)
}

// ======== tests section ========

/** note: used for tests, {@link test_sortAllOn} */
class test_SortDataboolean {
    name: string
    arr: boolean[]

    constructor(name: string, arr: boolean[]) {
        this.name = name
        this.arr = arr
    }
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortCopy(arr: boolean[]): boolean[] {
    let c = new boolean[arr.length]
    for (let i = 0; i < arr.length; i++) {
        c[i] = arr[i]
    }
    return c
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpFwd(l: boolean, r: boolean): boolean {
    return ((l) ? 1 : 0) < ((r) ? 1 : 0)
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpInv(l: boolean, r: boolean): boolean {
    return ((r) ? 1 : 0) < ((l) ? 1 : 0)
}

/** note: used for tests, {@link test_sortAllOn} */
function test_printArr(arr: boolean[], startIndex: int, endIndex: int) {
    for (let i = startIndex; i < endIndex; i++) {
        console.print(arr[i] + ' ')
    }
    console.println('')
}

/**
 * Function used to test sorting in standard library tests.
 * There is only one exported function: `sort`, but for testing
 * we need access to all sub sorts that are used. Hence this part of "tests"
 * is located here. All related entities are prefixed whith `test_` to stand out
 */
export function test_sortAllOn(arr: boolean[]) {
    // block for comparator ``
    if (true) {
        const sorts: test_SortDataboolean[] = new test_SortDataboolean[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length)
        sorts[lastSort++] = new test_SortDataboolean("bubble", bubbled)

        
        const bcnt = test_sortCopy(arr)
        countSortBools(bcnt, 0, bcnt.length)
        sorts[lastSort++] = new test_SortDataboolean("bools cnt()", bcnt)

        const just = test_sortCopy(arr)
        sort(just)
        sorts[lastSort++] = new test_SortDataboolean("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts  are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for boolean')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpFwd`
    if (true) {
        const sorts: test_SortDataboolean[] = new test_SortDataboolean[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDataboolean("bubble", bubbled)

        
        const bcnt = test_sortCopy(arr)
        countSortBools(bcnt, 0, bcnt.length)
        sorts[lastSort++] = new test_SortDataboolean("bools cnt()", bcnt)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDataboolean("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpFwd are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for boolean')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpInv`
    if (true) {
        const sorts: test_SortDataboolean[] = new test_SortDataboolean[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDataboolean("bubble", bubbled)

        
        const bcnt = test_sortCopy(arr)
        countSortBoolsInv(bcnt, 0, bcnt.length)
        sorts[lastSort++] = new test_SortDataboolean("bools cnt()", bcnt)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDataboolean("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpInv are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for boolean')
                }
            }
        }
    }
}
// ======== end of tests section ========


/**
 *  tries to find key in arr with respect of passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index of key if found in arr[startIndex; endIndex), otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4, 0, 3) == KEY_NOT_FOUND // true since value 4 is not in half-open range [0,3)
 * ```
 */
export function indexOf(arr: boolean[], key: boolean, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("indexOf: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find key in arr, starting from index
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function indexOf(arr: boolean[], key: boolean, fromIndex: int): int throws {
    return indexOf(arr, key, fromIndex, arr.length)
}

/*
 * tries to find key in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4) == 3
 * ```
 */
export function indexOf(arr: boolean[], key: boolean): int {
    try {
        return indexOf(arr, key, 0, arr.length)
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "indexOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return KEY_NOT_FOUND
}

/*
 * checks whether key is in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns true if key is in arr, false otherwise
 *
 * @example
 * ```
 * includes([1,2,3,4], 4) == true
 * ```
 */
export function includes(arr: boolean[], key: boolean): boolean {
    return indexOf(arr, key) != KEY_NOT_FOUND;
}

/*
 * tries to find last entry of a key into arr with respect to passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param smallerIndex an index of arr to end search with
 *
 * @param largerIndex a last index to start search in arr
 *
 * @returns index of key if found in arr(smallerIndex; largerIndex], otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: boolean[], key: boolean, smallerIndex: int, largerIndex: int): int throws {
    if (!checkRange(arr.length, smallerIndex + 1, largerIndex + 1)) {
        throw new ArrayIndexOutOfBoundsException("lastIndexOf: bounds verification failed")
    }

    for (let i: int = largerIndex; i > smallerIndex; i--) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find last entry of key into arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: boolean[], key: boolean): int {
    try {
        return lastIndexOf(arr, key, arr.length - 1)
    } catch (e) {
        assert false : "can't happen"
    }
}

/*
 * lastIndexOf(arr: boolean[], key: boolean, fromIndex: int) tries to find entry of key into arr which is not greater than fromIndex
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with (including, search is performed backwards)
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
function lastIndexOf(arr: boolean[], key: boolean, fromIndex: int): int throws {
    return lastIndexOf(arr, key, -1, fromIndex)
}

// C++ semantics
// (lower|upper)BoundSearch([1, 1, 2, 2, 2, 3, 3], 2, 0, 7)
//                                 ^        ^
//                                 |        |
//                                 |    upper bound
//                             lower bound


/**
 * tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is endIndex
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of. It may be not in arr, lower bound will present anyway
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than endIndex
 */
export function lowerBoundSearch(arr: boolean[], key: boolean, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("lowerBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] == false && key == true) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 * tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is endIndex
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of. It may be not in arr, lower bound will present anyway
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than arr.length
 */
export function lowerBoundSearch(arr: boolean[], key: boolean): int {
    try {
        return lowerBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "lowerBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than endIndex
 */
export function upperBoundSearch(arr: boolean[], key: boolean, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("upperBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] == false && key == true || arr[middle] == key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than arr.length
 */
export function upperBoundSearch(arr: boolean[], key: boolean): int {
    try {
        return upperBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "upperBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}


/**
 * concatenates two arrays into a single one
 *
 * @param a first array to concatenate
 *
 * @param b second array to concatenate
 *
 * @returns concatenation of the arguments
 */
export function concat(a: boolean[], b: boolean[]): boolean[] {
    let res: boolean[] = new boolean[a.length + b.length]
    try {
        copyTo(a, res, 0, 0, a.length);
        copyTo(b, res, a.length, 0, b.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "concat: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return res;
}

/**
 * takes an array and reverses it in-place
 *
 * @param arr an array to be reversed
 */
export function reverse(arr: boolean[]): void {
    let arrLen: int = arr.length;
    for (let i: int = 0; i < arrLen / 2; i++) {
        let tmp: boolean = arr[i];
        arr[i] = arr[arrLen - i - 1];
        arr[arrLen - i - 1] = tmp;
    }
}

/**
 * Joins elements with delimeter
 *
 * @param arr array to be joined in String
 *
 * @param delimiter a string to insert between elements of arr
 *
 * @returns joined elements of arr with delimiter between
 */

export function join(arr: boolean[], delimiter: String): String {
    let sb: StringBuilder = new StringBuilder();
    let len: int = arr.length;
    for (let i: int = 0; i < len; i++) {
        sb.append(arr[i] ? "true" : "false")
        if (i != len - 1) {
            sb.append(delimiter);
        }
    }
    return sb.toString();
}


/**
 * runs fn for elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function forEach(arr: boolean[], fn: (value: boolean, index: int, self: boolean[]) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i], i, arr);
    }
}

/**
 * runs fn over all elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 */
export function forEach(arr: boolean[], fn: (value: boolean) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i]);
    }
}

/**
 * updates element arr[i] using fn
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function map(arr: boolean[], fn: (value: boolean, index: int, self: boolean[]) => boolean): boolean[] {
    let res: boolean[] = new boolean[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i], i, arr);
    }
    return res;
}

/**
 * creates a new array using fn(arr[i])
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply for each element of arr
 *
 * @returns boolean[] - a copy of arr where for each element fn was applied
 */
export function map(arr: boolean[], fn: (value: boolean) => boolean): boolean[] {
    let res: boolean[] = new boolean[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i]);
    }
    return res;
}

/**
 * creates an array only with elements of arr where fn applied to element returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a filter with use of adjacent elements
 *
 * @returns an array of elements from arr where fn returned true
 */
export function filter(arr: boolean[], fn: (value: boolean, index: int, self: boolean[]) => boolean): boolean[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i], i, arr)) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: boolean[] = new boolean[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

/**
 * creates an array only with elements of arr where fn(arr[i]) returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition for value
 *
 * @returns an array of elements from arr where fn(arr[i]) returned true
 */
export function filter(arr: boolean[], fn: (value: boolean) => boolean): boolean[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i])) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: boolean[] = new boolean[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}


//
// Methods for byte[]
//

/**
 * fills arr in-place by specified value with respect to passed indexes
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr, exclusive, i.e. arr[endIndex] is not changed
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value, 0, arr.length)
 * ```
 */
export function fill(arr: byte[], value: byte, startIndex: int, endIndex: int): void throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("fill: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        arr[i] = value;
    }
}

/**
 * fills arr in-place by specified value
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value)
 * ```
 */
export function fill(arr: byte[], value: byte): void {
    try {
        fill(arr, value, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "fill: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
}

/**
 * copies src array into dst with respect to passed indexes.
 * dst must have enough space, otherwise out-of-bounds might occur
 *
 * @param src source array to be copied
 *
 * @param dst destination array
 *
 * @param dstStart index of dst to start from
 *
 * @param srcStart index of src to start from
 *
 * @param srcEnd last index of src to copy, exclusive, i.e. src[srcEnd] is not copied
 *
 * @example: copy src to dst
 * ```
 * copyTo(src, dst, 0, 0, src.length)
 * ```
 */
// TODO(petr-shumilov): make intrinsic
export function copyTo(src: byte[], dst: byte[], dstStart: int, srcStart: int, srcEnd: int): void throws {
    if (!checkRange(src.length, srcStart, srcEnd)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: src bounds verification failed")
    }
    if (!checkRange(dst.length, dstStart, dst.length)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: dst bounds verification failed")
    }
    if (!((srcEnd - srcStart) <= (dst.length - dstStart))) {
        throw new ArrayIndexOutOfBoundsException("Destination array must have enough space")
    }

    let j: int = dstStart;
    for (let i: int = srcStart; i < srcEnd; i++) {
        dst[j] = src[i];
        j++;
    }
}

/**
 * creates a copy of src array with respect to passed indexes.
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr to copy, exclusive, i.e. arr[endIndex] is not copied
 *
 * @returns copy of arr[startIndex; endIndex)
 *
 * @example: create exact copy of arr
 * ```
 * copyOf(arr, 0, arr.length)
 * ```
 */
export function copyOf(arr: byte[], startIndex: int, endIndex: int): byte[] throws {
    let res: byte[] = new byte[endIndex - startIndex];
    copyTo(arr, res, 0, startIndex, endIndex);
    return res;
}

/*
 * creates a copy of src array with respect to passed index
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of src to start from
 *
 * @returns copy of arr[startIndex; src.length)
 *
 * @example: create copy of arr without first element
 * ```
 * copyOf(arr, 1)
 * ```
 */
export function copyOf(arr: byte[], startIndex: int): byte[] throws {
    return copyOf(arr, startIndex, arr.length);
}

/*
 * copyOf(arr: byte[]) creates a copy of src array
 *
 * @param arr source array to be copied
 *
 * @returns copy of arr
 */
export function copyOf(arr: byte[]): byte[] {
    try {
        return copyOf(arr, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "copyOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return new byte[0]
}


function bubbleSort(arr: byte[], startIndex: int, endIndex: int): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if ((arr[i + 1] < arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: byte[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if ((arr[startIndex + idxFromStart] < arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex)
}

function heapSortDown(arr: byte[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && (arr[startIndex + best] < arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && (arr[startIndex + best] < arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: byte[], startIndex: int, endIndex: int): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i)
    }
}

function quickSortSplit(arr: byte[], startIndex: int, endIndex: int): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while ((arr[i] < pivot)) {
            i++
        }
        while ((pivot < arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if ((arr[i] < pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: byte[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: byte[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex)
}

function quickSort(arr: byte[], startIndex: int, endIndex: int): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: byte[], startIndex: int, endIndex: int): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    if (endIndex - startIndex > 1024) {
        countSort(arr, startIndex, endIndex)
    }
    quickSort(arr, startIndex, endIndex);
}


function bubbleSort(arr: byte[], startIndex: int, endIndex: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (mustPrecede(arr[i + 1], arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: byte[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if (mustPrecede(arr[startIndex + idxFromStart], arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex, mustPrecede)
}

function heapSortDown(arr: byte[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: byte[], startIndex: int, endIndex: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1, mustPrecede)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i, mustPrecede)
    }
}

function quickSortSplit(arr: byte[], startIndex: int, endIndex: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while (mustPrecede(arr[i], pivot)) {
            i++
        }
        while (mustPrecede(pivot, arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if (mustPrecede(arr[i], pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: byte[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: byte[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex, mustPrecede)
}

function quickSort(arr: byte[], startIndex: int, endIndex: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits, mustPrecede)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: byte[], startIndex: int, endIndex: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex, mustPrecede);
}



/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 */
export function sort(arr: byte[], mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    sort(arr, 0, arr.length, mustPrecede);
}

export function sort(arr: byte[], startIndex: int, mustPrecede: (lhs: byte, rhs: byte) => boolean): void {
    sort(arr, startIndex, arr.length, mustPrecede)
}


function countSort(arr: byte[], startIndex: int, endIndex: int): void {
    const cnts = new int[256]
    for (let i = startIndex; i < endIndex; i++) {
        cnts[arr[i] + 128]++
    }
    let idx = 0
    for (let i = 0; i < 256; i++) {
        for (let j = 0; j < cnts[i]; j++) {
            arr[startIndex + idx++] = (i - 128) as byte
        }
    }
}

export function sort(arr: byte[], startIndex: int): void {
    sort(arr, startIndex, arr.length)
}

export function sort(arr: byte[]): void {
    sort(arr, 0, arr.length)
}

// ======== tests section ========

/** note: used for tests, {@link test_sortAllOn} */
class test_SortDatabyte {
    name: string
    arr: byte[]

    constructor(name: string, arr: byte[]) {
        this.name = name
        this.arr = arr
    }
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortCopy(arr: byte[]): byte[] {
    let c = new byte[arr.length]
    for (let i = 0; i < arr.length; i++) {
        c[i] = arr[i]
    }
    return c
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpFwd(l: byte, r: byte): boolean {
    return l < r
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpInv(l: byte, r: byte): boolean {
    return r < l
}

/** note: used for tests, {@link test_sortAllOn} */
function test_printArr(arr: byte[], startIndex: int, endIndex: int) {
    for (let i = startIndex; i < endIndex; i++) {
        console.print(arr[i] + ' ')
    }
    console.println('')
}

/**
 * Function used to test sorting in standard library tests.
 * There is only one exported function: `sort`, but for testing
 * we need access to all sub sorts that are used. Hence this part of "tests"
 * is located here. All related entities are prefixed whith `test_` to stand out
 */
export function test_sortAllOn(arr: byte[]) {
    // block for comparator ``
    if (true) {
        const sorts: test_SortDatabyte[] = new test_SortDatabyte[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length)
        sorts[lastSort++] = new test_SortDatabyte("bubble", bubbled)
        const cnt = test_sortCopy(arr)
        countSort(cnt, 0, cnt.length)
        sorts[lastSort++] = new test_SortDatabyte("cnt()", cnt)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length)
        sorts[lastSort++] = new test_SortDatabyte("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length)
        sorts[lastSort++] = new test_SortDatabyte("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length)
        sorts[lastSort++] = new test_SortDatabyte("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just)
        sorts[lastSort++] = new test_SortDatabyte("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts  are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for byte')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpFwd`
    if (true) {
        const sorts: test_SortDatabyte[] = new test_SortDatabyte[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatabyte("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatabyte("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatabyte("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatabyte("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatabyte("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpFwd are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for byte')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpInv`
    if (true) {
        const sorts: test_SortDatabyte[] = new test_SortDatabyte[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatabyte("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatabyte("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatabyte("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatabyte("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatabyte("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpInv are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for byte')
                }
            }
        }
    }
}
// ======== end of tests section ========


/**
 *  tries to find key in arr with respect of passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index of key if found in arr[startIndex; endIndex), otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4, 0, 3) == KEY_NOT_FOUND // true since value 4 is not in half-open range [0,3)
 * ```
 */
export function indexOf(arr: byte[], key: byte, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("indexOf: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find key in arr, starting from index
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function indexOf(arr: byte[], key: byte, fromIndex: int): int throws {
    return indexOf(arr, key, fromIndex, arr.length)
}

/*
 * tries to find key in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4) == 3
 * ```
 */
export function indexOf(arr: byte[], key: byte): int {
    try {
        return indexOf(arr, key, 0, arr.length)
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "indexOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return KEY_NOT_FOUND
}

/*
 * checks whether key is in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns true if key is in arr, false otherwise
 *
 * @example
 * ```
 * includes([1,2,3,4], 4) == true
 * ```
 */
export function includes(arr: byte[], key: byte): boolean {
    return indexOf(arr, key) != KEY_NOT_FOUND;
}

/*
 * tries to find last entry of a key into arr with respect to passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param smallerIndex an index of arr to end search with
 *
 * @param largerIndex a last index to start search in arr
 *
 * @returns index of key if found in arr(smallerIndex; largerIndex], otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: byte[], key: byte, smallerIndex: int, largerIndex: int): int throws {
    if (!checkRange(arr.length, smallerIndex + 1, largerIndex + 1)) {
        throw new ArrayIndexOutOfBoundsException("lastIndexOf: bounds verification failed")
    }

    for (let i: int = largerIndex; i > smallerIndex; i--) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find last entry of key into arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: byte[], key: byte): int {
    try {
        return lastIndexOf(arr, key, arr.length - 1)
    } catch (e) {
        assert false : "can't happen"
    }
}

/*
 * lastIndexOf(arr: byte[], key: byte, fromIndex: int) tries to find entry of key into arr which is not greater than fromIndex
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with (including, search is performed backwards)
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
function lastIndexOf(arr: byte[], key: byte, fromIndex: int): int throws {
    return lastIndexOf(arr, key, -1, fromIndex)
}

// C++ semantics
// (lower|upper)BoundSearch([1, 1, 2, 2, 2, 3, 3], 2, 0, 7)
//                                 ^        ^
//                                 |        |
//                                 |    upper bound
//                             lower bound


/**
 * tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is endIndex
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than endIndex
 */
export function lowerBoundSearch(arr: byte[], key: byte, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("lowerBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] < key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 *  tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is arr.length
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than arr.length
 */
export function lowerBoundSearch(arr: byte[], key: byte): int {
    try {
        return lowerBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "lowerBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than endIndex
 */
export function upperBoundSearch(arr: byte[], key: byte, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("upperBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] <= key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than arr.length
 */
export function upperBoundSearch(arr: byte[], key: byte): int {
    try {
        return upperBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "upperBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}


/**
 * concatenates two arrays into a single one
 *
 * @param a first array to concatenate
 *
 * @param b second array to concatenate
 *
 * @returns concatenation of the arguments
 */
export function concat(a: byte[], b: byte[]): byte[] {
    let res: byte[] = new byte[a.length + b.length]
    try {
        copyTo(a, res, 0, 0, a.length);
        copyTo(b, res, a.length, 0, b.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "concat: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return res;
}

/**
 * takes an array and reverses it in-place
 *
 * @param arr an array to be reversed
 */
export function reverse(arr: byte[]): void {
    let arrLen: int = arr.length;
    for (let i: int = 0; i < arrLen / 2; i++) {
        let tmp: byte = arr[i];
        arr[i] = arr[arrLen - i - 1];
        arr[arrLen - i - 1] = tmp;
    }
}

/**
 * Joins elements with delimeter
 *
 * @param arr array to be joined in String
 *
 * @param delimiter a string to insert between elements of arr
 *
 * @returns joined elements of arr with delimiter between
 */

export function join(arr: byte[], delimiter: String): String {
    let sb: StringBuilder = new StringBuilder();
    let len: int = arr.length;
    for (let i: int = 0; i < len; i++) {
        sb.append(arr[i])
        if (i != len - 1) {
            sb.append(delimiter);
        }
    }
    return sb.toString();
}


/**
 * runs fn for elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function forEach(arr: byte[], fn: (value: byte, index: int, self: byte[]) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i], i, arr);
    }
}

/**
 * runs fn over all elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 */
export function forEach(arr: byte[], fn: (value: byte) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i]);
    }
}

/**
 * updates element arr[i] using fn
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function map(arr: byte[], fn: (value: byte, index: int, self: byte[]) => byte): byte[] {
    let res: byte[] = new byte[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i], i, arr);
    }
    return res;
}

/**
 * creates a new array using fn(arr[i])
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply for each element of arr
 *
 * @returns byte[] - a copy of arr where for each element fn was applied
 */
export function map(arr: byte[], fn: (value: byte) => byte): byte[] {
    let res: byte[] = new byte[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i]);
    }
    return res;
}

/**
 * creates an array only with elements of arr where fn applied to element returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a filter with use of adjacent elements
 *
 * @returns an array of elements from arr where fn returned true
 */
export function filter(arr: byte[], fn: (value: byte, index: int, self: byte[]) => boolean): byte[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i], i, arr)) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: byte[] = new byte[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

/**
 * creates an array only with elements of arr where fn(arr[i]) returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition for value
 *
 * @returns an array of elements from arr where fn(arr[i]) returned true
 */
export function filter(arr: byte[], fn: (value: byte) => boolean): byte[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i])) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: byte[] = new byte[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}


//
// Methods for short[]
//

/**
 * fills arr in-place by specified value with respect to passed indexes
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr, exclusive, i.e. arr[endIndex] is not changed
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value, 0, arr.length)
 * ```
 */
export function fill(arr: short[], value: short, startIndex: int, endIndex: int): void throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("fill: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        arr[i] = value;
    }
}

/**
 * fills arr in-place by specified value
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value)
 * ```
 */
export function fill(arr: short[], value: short): void {
    try {
        fill(arr, value, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "fill: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
}

/**
 * copies src array into dst with respect to passed indexes.
 * dst must have enough space, otherwise out-of-bounds might occur
 *
 * @param src source array to be copied
 *
 * @param dst destination array
 *
 * @param dstStart index of dst to start from
 *
 * @param srcStart index of src to start from
 *
 * @param srcEnd last index of src to copy, exclusive, i.e. src[srcEnd] is not copied
 *
 * @example: copy src to dst
 * ```
 * copyTo(src, dst, 0, 0, src.length)
 * ```
 */
// TODO(petr-shumilov): make intrinsic
export function copyTo(src: short[], dst: short[], dstStart: int, srcStart: int, srcEnd: int): void throws {
    if (!checkRange(src.length, srcStart, srcEnd)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: src bounds verification failed")
    }
    if (!checkRange(dst.length, dstStart, dst.length)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: dst bounds verification failed")
    }
    if (!((srcEnd - srcStart) <= (dst.length - dstStart))) {
        throw new ArrayIndexOutOfBoundsException("Destination array must have enough space")
    }

    let j: int = dstStart;
    for (let i: int = srcStart; i < srcEnd; i++) {
        dst[j] = src[i];
        j++;
    }
}

/**
 * creates a copy of src array with respect to passed indexes.
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr to copy, exclusive, i.e. arr[endIndex] is not copied
 *
 * @returns copy of arr[startIndex; endIndex)
 *
 * @example: create exact copy of arr
 * ```
 * copyOf(arr, 0, arr.length)
 * ```
 */
export function copyOf(arr: short[], startIndex: int, endIndex: int): short[] throws {
    let res: short[] = new short[endIndex - startIndex];
    copyTo(arr, res, 0, startIndex, endIndex);
    return res;
}

/*
 * creates a copy of src array with respect to passed index
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of src to start from
 *
 * @returns copy of arr[startIndex; src.length)
 *
 * @example: create copy of arr without first element
 * ```
 * copyOf(arr, 1)
 * ```
 */
export function copyOf(arr: short[], startIndex: int): short[] throws {
    return copyOf(arr, startIndex, arr.length);
}

/*
 * copyOf(arr: short[]) creates a copy of src array
 *
 * @param arr source array to be copied
 *
 * @returns copy of arr
 */
export function copyOf(arr: short[]): short[] {
    try {
        return copyOf(arr, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "copyOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return new short[0]
}


function bubbleSort(arr: short[], startIndex: int, endIndex: int): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if ((arr[i + 1] < arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: short[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if ((arr[startIndex + idxFromStart] < arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex)
}

function heapSortDown(arr: short[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && (arr[startIndex + best] < arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && (arr[startIndex + best] < arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: short[], startIndex: int, endIndex: int): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i)
    }
}

function quickSortSplit(arr: short[], startIndex: int, endIndex: int): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while ((arr[i] < pivot)) {
            i++
        }
        while ((pivot < arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if ((arr[i] < pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: short[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: short[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex)
}

function quickSort(arr: short[], startIndex: int, endIndex: int): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: short[], startIndex: int, endIndex: int): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex);
}


function bubbleSort(arr: short[], startIndex: int, endIndex: int, mustPrecede: (lhs: short, rhs: short) => boolean): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (mustPrecede(arr[i + 1], arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: short[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: short, rhs: short) => boolean): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if (mustPrecede(arr[startIndex + idxFromStart], arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex, mustPrecede)
}

function heapSortDown(arr: short[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: short, rhs: short) => boolean): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: short[], startIndex: int, endIndex: int, mustPrecede: (lhs: short, rhs: short) => boolean): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1, mustPrecede)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i, mustPrecede)
    }
}

function quickSortSplit(arr: short[], startIndex: int, endIndex: int, mustPrecede: (lhs: short, rhs: short) => boolean): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while (mustPrecede(arr[i], pivot)) {
            i++
        }
        while (mustPrecede(pivot, arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if (mustPrecede(arr[i], pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: short[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: short, rhs: short) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: short[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: short, rhs: short) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex, mustPrecede)
}

function quickSort(arr: short[], startIndex: int, endIndex: int, mustPrecede: (lhs: short, rhs: short) => boolean): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits, mustPrecede)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: short[], startIndex: int, endIndex: int, mustPrecede: (lhs: short, rhs: short) => boolean): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex, mustPrecede);
}



/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 */
export function sort(arr: short[], mustPrecede: (lhs: short, rhs: short) => boolean): void {
    sort(arr, 0, arr.length, mustPrecede);
}

export function sort(arr: short[], startIndex: int, mustPrecede: (lhs: short, rhs: short) => boolean): void {
    sort(arr, startIndex, arr.length, mustPrecede)
}



export function sort(arr: short[], startIndex: int): void {
    sort(arr, startIndex, arr.length)
}

export function sort(arr: short[]): void {
    sort(arr, 0, arr.length)
}

// ======== tests section ========

/** note: used for tests, {@link test_sortAllOn} */
class test_SortDatashort {
    name: string
    arr: short[]

    constructor(name: string, arr: short[]) {
        this.name = name
        this.arr = arr
    }
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortCopy(arr: short[]): short[] {
    let c = new short[arr.length]
    for (let i = 0; i < arr.length; i++) {
        c[i] = arr[i]
    }
    return c
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpFwd(l: short, r: short): boolean {
    return l < r
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpInv(l: short, r: short): boolean {
    return r < l
}

/** note: used for tests, {@link test_sortAllOn} */
function test_printArr(arr: short[], startIndex: int, endIndex: int) {
    for (let i = startIndex; i < endIndex; i++) {
        console.print(arr[i] + ' ')
    }
    console.println('')
}

/**
 * Function used to test sorting in standard library tests.
 * There is only one exported function: `sort`, but for testing
 * we need access to all sub sorts that are used. Hence this part of "tests"
 * is located here. All related entities are prefixed whith `test_` to stand out
 */
export function test_sortAllOn(arr: short[]) {
    // block for comparator ``
    if (true) {
        const sorts: test_SortDatashort[] = new test_SortDatashort[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length)
        sorts[lastSort++] = new test_SortDatashort("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length)
        sorts[lastSort++] = new test_SortDatashort("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length)
        sorts[lastSort++] = new test_SortDatashort("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length)
        sorts[lastSort++] = new test_SortDatashort("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just)
        sorts[lastSort++] = new test_SortDatashort("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts  are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for short')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpFwd`
    if (true) {
        const sorts: test_SortDatashort[] = new test_SortDatashort[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatashort("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatashort("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatashort("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatashort("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatashort("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpFwd are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for short')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpInv`
    if (true) {
        const sorts: test_SortDatashort[] = new test_SortDatashort[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatashort("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatashort("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatashort("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatashort("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatashort("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpInv are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for short')
                }
            }
        }
    }
}
// ======== end of tests section ========


/**
 *  tries to find key in arr with respect of passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index of key if found in arr[startIndex; endIndex), otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4, 0, 3) == KEY_NOT_FOUND // true since value 4 is not in half-open range [0,3)
 * ```
 */
export function indexOf(arr: short[], key: short, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("indexOf: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find key in arr, starting from index
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function indexOf(arr: short[], key: short, fromIndex: int): int throws {
    return indexOf(arr, key, fromIndex, arr.length)
}

/*
 * tries to find key in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4) == 3
 * ```
 */
export function indexOf(arr: short[], key: short): int {
    try {
        return indexOf(arr, key, 0, arr.length)
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "indexOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return KEY_NOT_FOUND
}

/*
 * checks whether key is in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns true if key is in arr, false otherwise
 *
 * @example
 * ```
 * includes([1,2,3,4], 4) == true
 * ```
 */
export function includes(arr: short[], key: short): boolean {
    return indexOf(arr, key) != KEY_NOT_FOUND;
}

/*
 * tries to find last entry of a key into arr with respect to passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param smallerIndex an index of arr to end search with
 *
 * @param largerIndex a last index to start search in arr
 *
 * @returns index of key if found in arr(smallerIndex; largerIndex], otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: short[], key: short, smallerIndex: int, largerIndex: int): int throws {
    if (!checkRange(arr.length, smallerIndex + 1, largerIndex + 1)) {
        throw new ArrayIndexOutOfBoundsException("lastIndexOf: bounds verification failed")
    }

    for (let i: int = largerIndex; i > smallerIndex; i--) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find last entry of key into arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: short[], key: short): int {
    try {
        return lastIndexOf(arr, key, arr.length - 1)
    } catch (e) {
        assert false : "can't happen"
    }
}

/*
 * lastIndexOf(arr: short[], key: short, fromIndex: int) tries to find entry of key into arr which is not greater than fromIndex
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with (including, search is performed backwards)
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
function lastIndexOf(arr: short[], key: short, fromIndex: int): int throws {
    return lastIndexOf(arr, key, -1, fromIndex)
}

// C++ semantics
// (lower|upper)BoundSearch([1, 1, 2, 2, 2, 3, 3], 2, 0, 7)
//                                 ^        ^
//                                 |        |
//                                 |    upper bound
//                             lower bound


/**
 * tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is endIndex
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than endIndex
 */
export function lowerBoundSearch(arr: short[], key: short, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("lowerBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] < key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 *  tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is arr.length
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than arr.length
 */
export function lowerBoundSearch(arr: short[], key: short): int {
    try {
        return lowerBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "lowerBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than endIndex
 */
export function upperBoundSearch(arr: short[], key: short, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("upperBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] <= key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than arr.length
 */
export function upperBoundSearch(arr: short[], key: short): int {
    try {
        return upperBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "upperBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}


/**
 * concatenates two arrays into a single one
 *
 * @param a first array to concatenate
 *
 * @param b second array to concatenate
 *
 * @returns concatenation of the arguments
 */
export function concat(a: short[], b: short[]): short[] {
    let res: short[] = new short[a.length + b.length]
    try {
        copyTo(a, res, 0, 0, a.length);
        copyTo(b, res, a.length, 0, b.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "concat: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return res;
}

/**
 * takes an array and reverses it in-place
 *
 * @param arr an array to be reversed
 */
export function reverse(arr: short[]): void {
    let arrLen: int = arr.length;
    for (let i: int = 0; i < arrLen / 2; i++) {
        let tmp: short = arr[i];
        arr[i] = arr[arrLen - i - 1];
        arr[arrLen - i - 1] = tmp;
    }
}

/**
 * Joins elements with delimeter
 *
 * @param arr array to be joined in String
 *
 * @param delimiter a string to insert between elements of arr
 *
 * @returns joined elements of arr with delimiter between
 */

export function join(arr: short[], delimiter: String): String {
    let sb: StringBuilder = new StringBuilder();
    let len: int = arr.length;
    for (let i: int = 0; i < len; i++) {
        sb.append(arr[i])
        if (i != len - 1) {
            sb.append(delimiter);
        }
    }
    return sb.toString();
}


/**
 * runs fn for elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function forEach(arr: short[], fn: (value: short, index: int, self: short[]) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i], i, arr);
    }
}

/**
 * runs fn over all elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 */
export function forEach(arr: short[], fn: (value: short) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i]);
    }
}

/**
 * updates element arr[i] using fn
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function map(arr: short[], fn: (value: short, index: int, self: short[]) => short): short[] {
    let res: short[] = new short[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i], i, arr);
    }
    return res;
}

/**
 * creates a new array using fn(arr[i])
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply for each element of arr
 *
 * @returns short[] - a copy of arr where for each element fn was applied
 */
export function map(arr: short[], fn: (value: short) => short): short[] {
    let res: short[] = new short[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i]);
    }
    return res;
}

/**
 * creates an array only with elements of arr where fn applied to element returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a filter with use of adjacent elements
 *
 * @returns an array of elements from arr where fn returned true
 */
export function filter(arr: short[], fn: (value: short, index: int, self: short[]) => boolean): short[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i], i, arr)) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: short[] = new short[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

/**
 * creates an array only with elements of arr where fn(arr[i]) returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition for value
 *
 * @returns an array of elements from arr where fn(arr[i]) returned true
 */
export function filter(arr: short[], fn: (value: short) => boolean): short[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i])) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: short[] = new short[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}


//
// Methods for int[]
//

/**
 * fills arr in-place by specified value with respect to passed indexes
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr, exclusive, i.e. arr[endIndex] is not changed
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value, 0, arr.length)
 * ```
 */
export function fill(arr: int[], value: int, startIndex: int, endIndex: int): void throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("fill: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        arr[i] = value;
    }
}

/**
 * fills arr in-place by specified value
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value)
 * ```
 */
export function fill(arr: int[], value: int): void {
    try {
        fill(arr, value, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "fill: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
}

/**
 * copies src array into dst with respect to passed indexes.
 * dst must have enough space, otherwise out-of-bounds might occur
 *
 * @param src source array to be copied
 *
 * @param dst destination array
 *
 * @param dstStart index of dst to start from
 *
 * @param srcStart index of src to start from
 *
 * @param srcEnd last index of src to copy, exclusive, i.e. src[srcEnd] is not copied
 *
 * @example: copy src to dst
 * ```
 * copyTo(src, dst, 0, 0, src.length)
 * ```
 */
// TODO(petr-shumilov): make intrinsic
export function copyTo(src: int[], dst: int[], dstStart: int, srcStart: int, srcEnd: int): void throws {
    if (!checkRange(src.length, srcStart, srcEnd)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: src bounds verification failed")
    }
    if (!checkRange(dst.length, dstStart, dst.length)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: dst bounds verification failed")
    }
    if (!((srcEnd - srcStart) <= (dst.length - dstStart))) {
        throw new ArrayIndexOutOfBoundsException("Destination array must have enough space")
    }

    let j: int = dstStart;
    for (let i: int = srcStart; i < srcEnd; i++) {
        dst[j] = src[i];
        j++;
    }
}

/**
 * creates a copy of src array with respect to passed indexes.
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr to copy, exclusive, i.e. arr[endIndex] is not copied
 *
 * @returns copy of arr[startIndex; endIndex)
 *
 * @example: create exact copy of arr
 * ```
 * copyOf(arr, 0, arr.length)
 * ```
 */
export function copyOf(arr: int[], startIndex: int, endIndex: int): int[] throws {
    let res: int[] = new int[endIndex - startIndex];
    copyTo(arr, res, 0, startIndex, endIndex);
    return res;
}

/*
 * creates a copy of src array with respect to passed index
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of src to start from
 *
 * @returns copy of arr[startIndex; src.length)
 *
 * @example: create copy of arr without first element
 * ```
 * copyOf(arr, 1)
 * ```
 */
export function copyOf(arr: int[], startIndex: int): int[] throws {
    return copyOf(arr, startIndex, arr.length);
}

/*
 * copyOf(arr: int[]) creates a copy of src array
 *
 * @param arr source array to be copied
 *
 * @returns copy of arr
 */
export function copyOf(arr: int[]): int[] {
    try {
        return copyOf(arr, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "copyOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return new int[0]
}


function bubbleSort(arr: int[], startIndex: int, endIndex: int): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if ((arr[i + 1] < arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: int[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if ((arr[startIndex + idxFromStart] < arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex)
}

function heapSortDown(arr: int[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && (arr[startIndex + best] < arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && (arr[startIndex + best] < arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: int[], startIndex: int, endIndex: int): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i)
    }
}

function quickSortSplit(arr: int[], startIndex: int, endIndex: int): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while ((arr[i] < pivot)) {
            i++
        }
        while ((pivot < arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if ((arr[i] < pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: int[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: int[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex)
}

function quickSort(arr: int[], startIndex: int, endIndex: int): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: int[], startIndex: int, endIndex: int): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex);
}


function bubbleSort(arr: int[], startIndex: int, endIndex: int, mustPrecede: (lhs: int, rhs: int) => boolean): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (mustPrecede(arr[i + 1], arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: int[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: int, rhs: int) => boolean): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if (mustPrecede(arr[startIndex + idxFromStart], arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex, mustPrecede)
}

function heapSortDown(arr: int[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: int, rhs: int) => boolean): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: int[], startIndex: int, endIndex: int, mustPrecede: (lhs: int, rhs: int) => boolean): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1, mustPrecede)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i, mustPrecede)
    }
}

function quickSortSplit(arr: int[], startIndex: int, endIndex: int, mustPrecede: (lhs: int, rhs: int) => boolean): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while (mustPrecede(arr[i], pivot)) {
            i++
        }
        while (mustPrecede(pivot, arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if (mustPrecede(arr[i], pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: int[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: int, rhs: int) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: int[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: int, rhs: int) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex, mustPrecede)
}

function quickSort(arr: int[], startIndex: int, endIndex: int, mustPrecede: (lhs: int, rhs: int) => boolean): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits, mustPrecede)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: int[], startIndex: int, endIndex: int, mustPrecede: (lhs: int, rhs: int) => boolean): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex, mustPrecede);
}



/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 */
export function sort(arr: int[], mustPrecede: (lhs: int, rhs: int) => boolean): void {
    sort(arr, 0, arr.length, mustPrecede);
}

export function sort(arr: int[], startIndex: int, mustPrecede: (lhs: int, rhs: int) => boolean): void {
    sort(arr, startIndex, arr.length, mustPrecede)
}



export function sort(arr: int[], startIndex: int): void {
    sort(arr, startIndex, arr.length)
}

export function sort(arr: int[]): void {
    sort(arr, 0, arr.length)
}

// ======== tests section ========

/** note: used for tests, {@link test_sortAllOn} */
class test_SortDataint {
    name: string
    arr: int[]

    constructor(name: string, arr: int[]) {
        this.name = name
        this.arr = arr
    }
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortCopy(arr: int[]): int[] {
    let c = new int[arr.length]
    for (let i = 0; i < arr.length; i++) {
        c[i] = arr[i]
    }
    return c
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpFwd(l: int, r: int): boolean {
    return l < r
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpInv(l: int, r: int): boolean {
    return r < l
}

/** note: used for tests, {@link test_sortAllOn} */
function test_printArr(arr: int[], startIndex: int, endIndex: int) {
    for (let i = startIndex; i < endIndex; i++) {
        console.print(arr[i] + ' ')
    }
    console.println('')
}

/**
 * Function used to test sorting in standard library tests.
 * There is only one exported function: `sort`, but for testing
 * we need access to all sub sorts that are used. Hence this part of "tests"
 * is located here. All related entities are prefixed whith `test_` to stand out
 */
export function test_sortAllOn(arr: int[]) {
    // block for comparator ``
    if (true) {
        const sorts: test_SortDataint[] = new test_SortDataint[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length)
        sorts[lastSort++] = new test_SortDataint("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length)
        sorts[lastSort++] = new test_SortDataint("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length)
        sorts[lastSort++] = new test_SortDataint("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length)
        sorts[lastSort++] = new test_SortDataint("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just)
        sorts[lastSort++] = new test_SortDataint("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts  are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for int')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpFwd`
    if (true) {
        const sorts: test_SortDataint[] = new test_SortDataint[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDataint("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDataint("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDataint("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDataint("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDataint("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpFwd are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for int')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpInv`
    if (true) {
        const sorts: test_SortDataint[] = new test_SortDataint[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDataint("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDataint("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDataint("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDataint("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDataint("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpInv are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for int')
                }
            }
        }
    }
}
// ======== end of tests section ========


/**
 *  tries to find key in arr with respect of passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index of key if found in arr[startIndex; endIndex), otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4, 0, 3) == KEY_NOT_FOUND // true since value 4 is not in half-open range [0,3)
 * ```
 */
export function indexOf(arr: int[], key: int, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("indexOf: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find key in arr, starting from index
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function indexOf(arr: int[], key: int, fromIndex: int): int throws {
    return indexOf(arr, key, fromIndex, arr.length)
}

/*
 * tries to find key in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4) == 3
 * ```
 */
export function indexOf(arr: int[], key: int): int {
    try {
        return indexOf(arr, key, 0, arr.length)
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "indexOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return KEY_NOT_FOUND
}

/*
 * checks whether key is in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns true if key is in arr, false otherwise
 *
 * @example
 * ```
 * includes([1,2,3,4], 4) == true
 * ```
 */
export function includes(arr: int[], key: int): boolean {
    return indexOf(arr, key) != KEY_NOT_FOUND;
}

/*
 * tries to find last entry of a key into arr with respect to passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param smallerIndex an index of arr to end search with
 *
 * @param largerIndex a last index to start search in arr
 *
 * @returns index of key if found in arr(smallerIndex; largerIndex], otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: int[], key: int, smallerIndex: int, largerIndex: int): int throws {
    if (!checkRange(arr.length, smallerIndex + 1, largerIndex + 1)) {
        throw new ArrayIndexOutOfBoundsException("lastIndexOf: bounds verification failed")
    }

    for (let i: int = largerIndex; i > smallerIndex; i--) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find last entry of key into arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: int[], key: int): int {
    try {
        return lastIndexOf(arr, key, arr.length - 1)
    } catch (e) {
        assert false : "can't happen"
    }
}

/*
 * lastIndexOf(arr: int[], key: int, fromIndex: int) tries to find entry of key into arr which is not greater than fromIndex
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with (including, search is performed backwards)
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
function lastIndexOf(arr: int[], key: int, fromIndex: int): int throws {
    return lastIndexOf(arr, key, -1, fromIndex)
}

// C++ semantics
// (lower|upper)BoundSearch([1, 1, 2, 2, 2, 3, 3], 2, 0, 7)
//                                 ^        ^
//                                 |        |
//                                 |    upper bound
//                             lower bound


/**
 * tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is endIndex
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than endIndex
 */
export function lowerBoundSearch(arr: int[], key: int, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("lowerBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] < key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 *  tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is arr.length
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than arr.length
 */
export function lowerBoundSearch(arr: int[], key: int): int {
    try {
        return lowerBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "lowerBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than endIndex
 */
export function upperBoundSearch(arr: int[], key: int, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("upperBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] <= key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than arr.length
 */
export function upperBoundSearch(arr: int[], key: int): int {
    try {
        return upperBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "upperBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}


/**
 * concatenates two arrays into a single one
 *
 * @param a first array to concatenate
 *
 * @param b second array to concatenate
 *
 * @returns concatenation of the arguments
 */
export function concat(a: int[], b: int[]): int[] {
    let res: int[] = new int[a.length + b.length]
    try {
        copyTo(a, res, 0, 0, a.length);
        copyTo(b, res, a.length, 0, b.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "concat: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return res;
}

/**
 * takes an array and reverses it in-place
 *
 * @param arr an array to be reversed
 */
export function reverse(arr: int[]): void {
    let arrLen: int = arr.length;
    for (let i: int = 0; i < arrLen / 2; i++) {
        let tmp: int = arr[i];
        arr[i] = arr[arrLen - i - 1];
        arr[arrLen - i - 1] = tmp;
    }
}

/**
 * Joins elements with delimeter
 *
 * @param arr array to be joined in String
 *
 * @param delimiter a string to insert between elements of arr
 *
 * @returns joined elements of arr with delimiter between
 */

export function join(arr: int[], delimiter: String): String {
    let sb: StringBuilder = new StringBuilder();
    let len: int = arr.length;
    for (let i: int = 0; i < len; i++) {
        sb.append(arr[i])
        if (i != len - 1) {
            sb.append(delimiter);
        }
    }
    return sb.toString();
}


/**
 * runs fn for elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function forEach(arr: int[], fn: (value: int, index: int, self: int[]) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i], i, arr);
    }
}

/**
 * runs fn over all elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 */
export function forEach(arr: int[], fn: (value: int) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i]);
    }
}

/**
 * updates element arr[i] using fn
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function map(arr: int[], fn: (value: int, index: int, self: int[]) => int): int[] {
    let res: int[] = new int[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i], i, arr);
    }
    return res;
}

/**
 * creates a new array using fn(arr[i])
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply for each element of arr
 *
 * @returns int[] - a copy of arr where for each element fn was applied
 */
export function map(arr: int[], fn: (value: int) => int): int[] {
    let res: int[] = new int[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i]);
    }
    return res;
}

/**
 * creates an array only with elements of arr where fn applied to element returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a filter with use of adjacent elements
 *
 * @returns an array of elements from arr where fn returned true
 */
export function filter(arr: int[], fn: (value: int, index: int, self: int[]) => boolean): int[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i], i, arr)) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: int[] = new int[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

/**
 * creates an array only with elements of arr where fn(arr[i]) returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition for value
 *
 * @returns an array of elements from arr where fn(arr[i]) returned true
 */
export function filter(arr: int[], fn: (value: int) => boolean): int[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i])) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: int[] = new int[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}


//
// Methods for long[]
//

/**
 * fills arr in-place by specified value with respect to passed indexes
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr, exclusive, i.e. arr[endIndex] is not changed
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value, 0, arr.length)
 * ```
 */
export function fill(arr: long[], value: long, startIndex: int, endIndex: int): void throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("fill: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        arr[i] = value;
    }
}

/**
 * fills arr in-place by specified value
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value)
 * ```
 */
export function fill(arr: long[], value: long): void {
    try {
        fill(arr, value, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "fill: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
}

/**
 * copies src array into dst with respect to passed indexes.
 * dst must have enough space, otherwise out-of-bounds might occur
 *
 * @param src source array to be copied
 *
 * @param dst destination array
 *
 * @param dstStart index of dst to start from
 *
 * @param srcStart index of src to start from
 *
 * @param srcEnd last index of src to copy, exclusive, i.e. src[srcEnd] is not copied
 *
 * @example: copy src to dst
 * ```
 * copyTo(src, dst, 0, 0, src.length)
 * ```
 */
// TODO(petr-shumilov): make intrinsic
export function copyTo(src: long[], dst: long[], dstStart: int, srcStart: int, srcEnd: int): void throws {
    if (!checkRange(src.length, srcStart, srcEnd)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: src bounds verification failed")
    }
    if (!checkRange(dst.length, dstStart, dst.length)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: dst bounds verification failed")
    }
    if (!((srcEnd - srcStart) <= (dst.length - dstStart))) {
        throw new ArrayIndexOutOfBoundsException("Destination array must have enough space")
    }

    let j: int = dstStart;
    for (let i: int = srcStart; i < srcEnd; i++) {
        dst[j] = src[i];
        j++;
    }
}

/**
 * creates a copy of src array with respect to passed indexes.
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr to copy, exclusive, i.e. arr[endIndex] is not copied
 *
 * @returns copy of arr[startIndex; endIndex)
 *
 * @example: create exact copy of arr
 * ```
 * copyOf(arr, 0, arr.length)
 * ```
 */
export function copyOf(arr: long[], startIndex: int, endIndex: int): long[] throws {
    let res: long[] = new long[endIndex - startIndex];
    copyTo(arr, res, 0, startIndex, endIndex);
    return res;
}

/*
 * creates a copy of src array with respect to passed index
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of src to start from
 *
 * @returns copy of arr[startIndex; src.length)
 *
 * @example: create copy of arr without first element
 * ```
 * copyOf(arr, 1)
 * ```
 */
export function copyOf(arr: long[], startIndex: int): long[] throws {
    return copyOf(arr, startIndex, arr.length);
}

/*
 * copyOf(arr: long[]) creates a copy of src array
 *
 * @param arr source array to be copied
 *
 * @returns copy of arr
 */
export function copyOf(arr: long[]): long[] {
    try {
        return copyOf(arr, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "copyOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return new long[0]
}


function bubbleSort(arr: long[], startIndex: int, endIndex: int): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if ((arr[i + 1] < arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: long[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if ((arr[startIndex + idxFromStart] < arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex)
}

function heapSortDown(arr: long[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && (arr[startIndex + best] < arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && (arr[startIndex + best] < arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: long[], startIndex: int, endIndex: int): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i)
    }
}

function quickSortSplit(arr: long[], startIndex: int, endIndex: int): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while ((arr[i] < pivot)) {
            i++
        }
        while ((pivot < arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if ((arr[i] < pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: long[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: long[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex)
}

function quickSort(arr: long[], startIndex: int, endIndex: int): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: long[], startIndex: int, endIndex: int): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex);
}


function bubbleSort(arr: long[], startIndex: int, endIndex: int, mustPrecede: (lhs: long, rhs: long) => boolean): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (mustPrecede(arr[i + 1], arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: long[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: long, rhs: long) => boolean): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if (mustPrecede(arr[startIndex + idxFromStart], arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex, mustPrecede)
}

function heapSortDown(arr: long[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: long, rhs: long) => boolean): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: long[], startIndex: int, endIndex: int, mustPrecede: (lhs: long, rhs: long) => boolean): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1, mustPrecede)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i, mustPrecede)
    }
}

function quickSortSplit(arr: long[], startIndex: int, endIndex: int, mustPrecede: (lhs: long, rhs: long) => boolean): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while (mustPrecede(arr[i], pivot)) {
            i++
        }
        while (mustPrecede(pivot, arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if (mustPrecede(arr[i], pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: long[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: long, rhs: long) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: long[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: long, rhs: long) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex, mustPrecede)
}

function quickSort(arr: long[], startIndex: int, endIndex: int, mustPrecede: (lhs: long, rhs: long) => boolean): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits, mustPrecede)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: long[], startIndex: int, endIndex: int, mustPrecede: (lhs: long, rhs: long) => boolean): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex, mustPrecede);
}



/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 */
export function sort(arr: long[], mustPrecede: (lhs: long, rhs: long) => boolean): void {
    sort(arr, 0, arr.length, mustPrecede);
}

export function sort(arr: long[], startIndex: int, mustPrecede: (lhs: long, rhs: long) => boolean): void {
    sort(arr, startIndex, arr.length, mustPrecede)
}



export function sort(arr: long[], startIndex: int): void {
    sort(arr, startIndex, arr.length)
}

export function sort(arr: long[]): void {
    sort(arr, 0, arr.length)
}

// ======== tests section ========

/** note: used for tests, {@link test_sortAllOn} */
class test_SortDatalong {
    name: string
    arr: long[]

    constructor(name: string, arr: long[]) {
        this.name = name
        this.arr = arr
    }
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortCopy(arr: long[]): long[] {
    let c = new long[arr.length]
    for (let i = 0; i < arr.length; i++) {
        c[i] = arr[i]
    }
    return c
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpFwd(l: long, r: long): boolean {
    return l < r
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpInv(l: long, r: long): boolean {
    return r < l
}

/** note: used for tests, {@link test_sortAllOn} */
function test_printArr(arr: long[], startIndex: int, endIndex: int) {
    for (let i = startIndex; i < endIndex; i++) {
        console.print(arr[i] + ' ')
    }
    console.println('')
}

/**
 * Function used to test sorting in standard library tests.
 * There is only one exported function: `sort`, but for testing
 * we need access to all sub sorts that are used. Hence this part of "tests"
 * is located here. All related entities are prefixed whith `test_` to stand out
 */
export function test_sortAllOn(arr: long[]) {
    // block for comparator ``
    if (true) {
        const sorts: test_SortDatalong[] = new test_SortDatalong[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length)
        sorts[lastSort++] = new test_SortDatalong("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length)
        sorts[lastSort++] = new test_SortDatalong("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length)
        sorts[lastSort++] = new test_SortDatalong("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length)
        sorts[lastSort++] = new test_SortDatalong("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just)
        sorts[lastSort++] = new test_SortDatalong("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts  are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for long')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpFwd`
    if (true) {
        const sorts: test_SortDatalong[] = new test_SortDatalong[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatalong("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatalong("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatalong("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatalong("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatalong("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpFwd are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for long')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpInv`
    if (true) {
        const sorts: test_SortDatalong[] = new test_SortDatalong[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatalong("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatalong("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatalong("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatalong("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatalong("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpInv are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for long')
                }
            }
        }
    }
}
// ======== end of tests section ========


/**
 *  tries to find key in arr with respect of passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index of key if found in arr[startIndex; endIndex), otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4, 0, 3) == KEY_NOT_FOUND // true since value 4 is not in half-open range [0,3)
 * ```
 */
export function indexOf(arr: long[], key: long, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("indexOf: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find key in arr, starting from index
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function indexOf(arr: long[], key: long, fromIndex: int): int throws {
    return indexOf(arr, key, fromIndex, arr.length)
}

/*
 * tries to find key in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4) == 3
 * ```
 */
export function indexOf(arr: long[], key: long): int {
    try {
        return indexOf(arr, key, 0, arr.length)
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "indexOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return KEY_NOT_FOUND
}

/*
 * checks whether key is in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns true if key is in arr, false otherwise
 *
 * @example
 * ```
 * includes([1,2,3,4], 4) == true
 * ```
 */
export function includes(arr: long[], key: long): boolean {
    return indexOf(arr, key) != KEY_NOT_FOUND;
}

/*
 * tries to find last entry of a key into arr with respect to passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param smallerIndex an index of arr to end search with
 *
 * @param largerIndex a last index to start search in arr
 *
 * @returns index of key if found in arr(smallerIndex; largerIndex], otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: long[], key: long, smallerIndex: int, largerIndex: int): int throws {
    if (!checkRange(arr.length, smallerIndex + 1, largerIndex + 1)) {
        throw new ArrayIndexOutOfBoundsException("lastIndexOf: bounds verification failed")
    }

    for (let i: int = largerIndex; i > smallerIndex; i--) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find last entry of key into arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: long[], key: long): int {
    try {
        return lastIndexOf(arr, key, arr.length - 1)
    } catch (e) {
        assert false : "can't happen"
    }
}

/*
 * lastIndexOf(arr: long[], key: long, fromIndex: int) tries to find entry of key into arr which is not greater than fromIndex
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with (including, search is performed backwards)
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
function lastIndexOf(arr: long[], key: long, fromIndex: int): int throws {
    return lastIndexOf(arr, key, -1, fromIndex)
}

// C++ semantics
// (lower|upper)BoundSearch([1, 1, 2, 2, 2, 3, 3], 2, 0, 7)
//                                 ^        ^
//                                 |        |
//                                 |    upper bound
//                             lower bound


/**
 * tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is endIndex
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than endIndex
 */
export function lowerBoundSearch(arr: long[], key: long, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("lowerBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] < key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 *  tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is arr.length
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than arr.length
 */
export function lowerBoundSearch(arr: long[], key: long): int {
    try {
        return lowerBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "lowerBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than endIndex
 */
export function upperBoundSearch(arr: long[], key: long, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("upperBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] <= key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than arr.length
 */
export function upperBoundSearch(arr: long[], key: long): int {
    try {
        return upperBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "upperBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}


/**
 * concatenates two arrays into a single one
 *
 * @param a first array to concatenate
 *
 * @param b second array to concatenate
 *
 * @returns concatenation of the arguments
 */
export function concat(a: long[], b: long[]): long[] {
    let res: long[] = new long[a.length + b.length]
    try {
        copyTo(a, res, 0, 0, a.length);
        copyTo(b, res, a.length, 0, b.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "concat: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return res;
}

/**
 * takes an array and reverses it in-place
 *
 * @param arr an array to be reversed
 */
export function reverse(arr: long[]): void {
    let arrLen: int = arr.length;
    for (let i: int = 0; i < arrLen / 2; i++) {
        let tmp: long = arr[i];
        arr[i] = arr[arrLen - i - 1];
        arr[arrLen - i - 1] = tmp;
    }
}

/**
 * Joins elements with delimeter
 *
 * @param arr array to be joined in String
 *
 * @param delimiter a string to insert between elements of arr
 *
 * @returns joined elements of arr with delimiter between
 */

export function join(arr: long[], delimiter: String): String {
    let sb: StringBuilder = new StringBuilder();
    let len: int = arr.length;
    for (let i: int = 0; i < len; i++) {
        sb.append(arr[i])
        if (i != len - 1) {
            sb.append(delimiter);
        }
    }
    return sb.toString();
}


/**
 * runs fn for elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function forEach(arr: long[], fn: (value: long, index: int, self: long[]) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i], i, arr);
    }
}

/**
 * runs fn over all elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 */
export function forEach(arr: long[], fn: (value: long) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i]);
    }
}

/**
 * updates element arr[i] using fn
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function map(arr: long[], fn: (value: long, index: int, self: long[]) => long): long[] {
    let res: long[] = new long[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i], i, arr);
    }
    return res;
}

/**
 * creates a new array using fn(arr[i])
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply for each element of arr
 *
 * @returns long[] - a copy of arr where for each element fn was applied
 */
export function map(arr: long[], fn: (value: long) => long): long[] {
    let res: long[] = new long[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i]);
    }
    return res;
}

/**
 * creates an array only with elements of arr where fn applied to element returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a filter with use of adjacent elements
 *
 * @returns an array of elements from arr where fn returned true
 */
export function filter(arr: long[], fn: (value: long, index: int, self: long[]) => boolean): long[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i], i, arr)) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: long[] = new long[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

/**
 * creates an array only with elements of arr where fn(arr[i]) returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition for value
 *
 * @returns an array of elements from arr where fn(arr[i]) returned true
 */
export function filter(arr: long[], fn: (value: long) => boolean): long[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i])) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: long[] = new long[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}


//
// Methods for float[]
//

/**
 * fills arr in-place by specified value with respect to passed indexes
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr, exclusive, i.e. arr[endIndex] is not changed
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value, 0, arr.length)
 * ```
 */
export function fill(arr: float[], value: float, startIndex: int, endIndex: int): void throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("fill: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        arr[i] = value;
    }
}

/**
 * fills arr in-place by specified value
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value)
 * ```
 */
export function fill(arr: float[], value: float): void {
    try {
        fill(arr, value, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "fill: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
}

/**
 * copies src array into dst with respect to passed indexes.
 * dst must have enough space, otherwise out-of-bounds might occur
 *
 * @param src source array to be copied
 *
 * @param dst destination array
 *
 * @param dstStart index of dst to start from
 *
 * @param srcStart index of src to start from
 *
 * @param srcEnd last index of src to copy, exclusive, i.e. src[srcEnd] is not copied
 *
 * @example: copy src to dst
 * ```
 * copyTo(src, dst, 0, 0, src.length)
 * ```
 */
// TODO(petr-shumilov): make intrinsic
export function copyTo(src: float[], dst: float[], dstStart: int, srcStart: int, srcEnd: int): void throws {
    if (!checkRange(src.length, srcStart, srcEnd)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: src bounds verification failed")
    }
    if (!checkRange(dst.length, dstStart, dst.length)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: dst bounds verification failed")
    }
    if (!((srcEnd - srcStart) <= (dst.length - dstStart))) {
        throw new ArrayIndexOutOfBoundsException("Destination array must have enough space")
    }

    let j: int = dstStart;
    for (let i: int = srcStart; i < srcEnd; i++) {
        dst[j] = src[i];
        j++;
    }
}

/**
 * creates a copy of src array with respect to passed indexes.
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr to copy, exclusive, i.e. arr[endIndex] is not copied
 *
 * @returns copy of arr[startIndex; endIndex)
 *
 * @example: create exact copy of arr
 * ```
 * copyOf(arr, 0, arr.length)
 * ```
 */
export function copyOf(arr: float[], startIndex: int, endIndex: int): float[] throws {
    let res: float[] = new float[endIndex - startIndex];
    copyTo(arr, res, 0, startIndex, endIndex);
    return res;
}

/*
 * creates a copy of src array with respect to passed index
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of src to start from
 *
 * @returns copy of arr[startIndex; src.length)
 *
 * @example: create copy of arr without first element
 * ```
 * copyOf(arr, 1)
 * ```
 */
export function copyOf(arr: float[], startIndex: int): float[] throws {
    return copyOf(arr, startIndex, arr.length);
}

/*
 * copyOf(arr: float[]) creates a copy of src array
 *
 * @param arr source array to be copied
 *
 * @returns copy of arr
 */
export function copyOf(arr: float[]): float[] {
    try {
        return copyOf(arr, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "copyOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return new float[0]
}


function bubbleSort(arr: float[], startIndex: int, endIndex: int): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if ((arr[i + 1] < arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: float[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if ((arr[startIndex + idxFromStart] < arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex)
}

function heapSortDown(arr: float[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && (arr[startIndex + best] < arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && (arr[startIndex + best] < arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: float[], startIndex: int, endIndex: int): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i)
    }
}

function quickSortSplit(arr: float[], startIndex: int, endIndex: int): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while ((arr[i] < pivot)) {
            i++
        }
        while ((pivot < arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if ((arr[i] < pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: float[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: float[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex)
}

function quickSort(arr: float[], startIndex: int, endIndex: int): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: float[], startIndex: int, endIndex: int): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex);
}


function bubbleSort(arr: float[], startIndex: int, endIndex: int, mustPrecede: (lhs: float, rhs: float) => boolean): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (mustPrecede(arr[i + 1], arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: float[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: float, rhs: float) => boolean): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if (mustPrecede(arr[startIndex + idxFromStart], arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex, mustPrecede)
}

function heapSortDown(arr: float[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: float, rhs: float) => boolean): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: float[], startIndex: int, endIndex: int, mustPrecede: (lhs: float, rhs: float) => boolean): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1, mustPrecede)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i, mustPrecede)
    }
}

function quickSortSplit(arr: float[], startIndex: int, endIndex: int, mustPrecede: (lhs: float, rhs: float) => boolean): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while (mustPrecede(arr[i], pivot)) {
            i++
        }
        while (mustPrecede(pivot, arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if (mustPrecede(arr[i], pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: float[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: float, rhs: float) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: float[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: float, rhs: float) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex, mustPrecede)
}

function quickSort(arr: float[], startIndex: int, endIndex: int, mustPrecede: (lhs: float, rhs: float) => boolean): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits, mustPrecede)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: float[], startIndex: int, endIndex: int, mustPrecede: (lhs: float, rhs: float) => boolean): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex, mustPrecede);
}



/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 */
export function sort(arr: float[], mustPrecede: (lhs: float, rhs: float) => boolean): void {
    sort(arr, 0, arr.length, mustPrecede);
}

export function sort(arr: float[], startIndex: int, mustPrecede: (lhs: float, rhs: float) => boolean): void {
    sort(arr, startIndex, arr.length, mustPrecede)
}



export function sort(arr: float[], startIndex: int): void {
    sort(arr, startIndex, arr.length)
}

export function sort(arr: float[]): void {
    sort(arr, 0, arr.length)
}

// ======== tests section ========

/** note: used for tests, {@link test_sortAllOn} */
class test_SortDatafloat {
    name: string
    arr: float[]

    constructor(name: string, arr: float[]) {
        this.name = name
        this.arr = arr
    }
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortCopy(arr: float[]): float[] {
    let c = new float[arr.length]
    for (let i = 0; i < arr.length; i++) {
        c[i] = arr[i]
    }
    return c
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpFwd(l: float, r: float): boolean {
    return l < r
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpInv(l: float, r: float): boolean {
    return r < l
}

/** note: used for tests, {@link test_sortAllOn} */
function test_printArr(arr: float[], startIndex: int, endIndex: int) {
    for (let i = startIndex; i < endIndex; i++) {
        console.print(arr[i] + ' ')
    }
    console.println('')
}

/**
 * Function used to test sorting in standard library tests.
 * There is only one exported function: `sort`, but for testing
 * we need access to all sub sorts that are used. Hence this part of "tests"
 * is located here. All related entities are prefixed whith `test_` to stand out
 */
export function test_sortAllOn(arr: float[]) {
    // block for comparator ``
    if (true) {
        const sorts: test_SortDatafloat[] = new test_SortDatafloat[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length)
        sorts[lastSort++] = new test_SortDatafloat("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length)
        sorts[lastSort++] = new test_SortDatafloat("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length)
        sorts[lastSort++] = new test_SortDatafloat("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length)
        sorts[lastSort++] = new test_SortDatafloat("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just)
        sorts[lastSort++] = new test_SortDatafloat("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts  are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for float')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpFwd`
    if (true) {
        const sorts: test_SortDatafloat[] = new test_SortDatafloat[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatafloat("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatafloat("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatafloat("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatafloat("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatafloat("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpFwd are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for float')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpInv`
    if (true) {
        const sorts: test_SortDatafloat[] = new test_SortDatafloat[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatafloat("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatafloat("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatafloat("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatafloat("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatafloat("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpInv are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for float')
                }
            }
        }
    }
}
// ======== end of tests section ========


/**
 *  tries to find key in arr with respect of passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index of key if found in arr[startIndex; endIndex), otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4, 0, 3) == KEY_NOT_FOUND // true since value 4 is not in half-open range [0,3)
 * ```
 */
export function indexOf(arr: float[], key: float, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("indexOf: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find key in arr, starting from index
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function indexOf(arr: float[], key: float, fromIndex: int): int throws {
    return indexOf(arr, key, fromIndex, arr.length)
}

/*
 * tries to find key in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4) == 3
 * ```
 */
export function indexOf(arr: float[], key: float): int {
    try {
        return indexOf(arr, key, 0, arr.length)
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "indexOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return KEY_NOT_FOUND
}

/*
 * checks whether key is in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns true if key is in arr, false otherwise
 *
 * @example
 * ```
 * includes([1,2,3,4], 4) == true
 * ```
 */
export function includes(arr: float[], key: float): boolean {
    return indexOf(arr, key) != KEY_NOT_FOUND;
}

/*
 * tries to find last entry of a key into arr with respect to passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param smallerIndex an index of arr to end search with
 *
 * @param largerIndex a last index to start search in arr
 *
 * @returns index of key if found in arr(smallerIndex; largerIndex], otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: float[], key: float, smallerIndex: int, largerIndex: int): int throws {
    if (!checkRange(arr.length, smallerIndex + 1, largerIndex + 1)) {
        throw new ArrayIndexOutOfBoundsException("lastIndexOf: bounds verification failed")
    }

    for (let i: int = largerIndex; i > smallerIndex; i--) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find last entry of key into arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: float[], key: float): int {
    try {
        return lastIndexOf(arr, key, arr.length - 1)
    } catch (e) {
        assert false : "can't happen"
    }
}

/*
 * lastIndexOf(arr: float[], key: float, fromIndex: int) tries to find entry of key into arr which is not greater than fromIndex
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with (including, search is performed backwards)
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
function lastIndexOf(arr: float[], key: float, fromIndex: int): int throws {
    return lastIndexOf(arr, key, -1, fromIndex)
}

// C++ semantics
// (lower|upper)BoundSearch([1, 1, 2, 2, 2, 3, 3], 2, 0, 7)
//                                 ^        ^
//                                 |        |
//                                 |    upper bound
//                             lower bound


/**
 * tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is endIndex
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than endIndex
 */
export function lowerBoundSearch(arr: float[], key: float, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("lowerBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] < key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 *  tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is arr.length
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than arr.length
 */
export function lowerBoundSearch(arr: float[], key: float): int {
    try {
        return lowerBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "lowerBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than endIndex
 */
export function upperBoundSearch(arr: float[], key: float, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("upperBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] <= key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than arr.length
 */
export function upperBoundSearch(arr: float[], key: float): int {
    try {
        return upperBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "upperBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}


/**
 * concatenates two arrays into a single one
 *
 * @param a first array to concatenate
 *
 * @param b second array to concatenate
 *
 * @returns concatenation of the arguments
 */
export function concat(a: float[], b: float[]): float[] {
    let res: float[] = new float[a.length + b.length]
    try {
        copyTo(a, res, 0, 0, a.length);
        copyTo(b, res, a.length, 0, b.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "concat: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return res;
}

/**
 * takes an array and reverses it in-place
 *
 * @param arr an array to be reversed
 */
export function reverse(arr: float[]): void {
    let arrLen: int = arr.length;
    for (let i: int = 0; i < arrLen / 2; i++) {
        let tmp: float = arr[i];
        arr[i] = arr[arrLen - i - 1];
        arr[arrLen - i - 1] = tmp;
    }
}

/**
 * Joins elements with delimeter
 *
 * @param arr array to be joined in String
 *
 * @param delimiter a string to insert between elements of arr
 *
 * @returns joined elements of arr with delimiter between
 */

export function join(arr: float[], delimiter: String): String {
    let sb: StringBuilder = new StringBuilder();
    let len: int = arr.length;
    for (let i: int = 0; i < len; i++) {
        sb.append(arr[i])
        if (i != len - 1) {
            sb.append(delimiter);
        }
    }
    return sb.toString();
}


/**
 * runs fn for elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function forEach(arr: float[], fn: (value: float, index: int, self: float[]) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i], i, arr);
    }
}

/**
 * runs fn over all elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 */
export function forEach(arr: float[], fn: (value: float) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i]);
    }
}

/**
 * updates element arr[i] using fn
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function map(arr: float[], fn: (value: float, index: int, self: float[]) => float): float[] {
    let res: float[] = new float[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i], i, arr);
    }
    return res;
}

/**
 * creates a new array using fn(arr[i])
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply for each element of arr
 *
 * @returns float[] - a copy of arr where for each element fn was applied
 */
export function map(arr: float[], fn: (value: float) => float): float[] {
    let res: float[] = new float[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i]);
    }
    return res;
}

/**
 * creates an array only with elements of arr where fn applied to element returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a filter with use of adjacent elements
 *
 * @returns an array of elements from arr where fn returned true
 */
export function filter(arr: float[], fn: (value: float, index: int, self: float[]) => boolean): float[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i], i, arr)) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: float[] = new float[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

/**
 * creates an array only with elements of arr where fn(arr[i]) returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition for value
 *
 * @returns an array of elements from arr where fn(arr[i]) returned true
 */
export function filter(arr: float[], fn: (value: float) => boolean): float[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i])) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: float[] = new float[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}


//
// Methods for double[]
//

/**
 * fills arr in-place by specified value with respect to passed indexes
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr, exclusive, i.e. arr[endIndex] is not changed
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value, 0, arr.length)
 * ```
 */
export function fill(arr: double[], value: double, startIndex: int, endIndex: int): void throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("fill: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        arr[i] = value;
    }
}

/**
 * fills arr in-place by specified value
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value)
 * ```
 */
export function fill(arr: double[], value: double): void {
    try {
        fill(arr, value, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "fill: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
}

/**
 * copies src array into dst with respect to passed indexes.
 * dst must have enough space, otherwise out-of-bounds might occur
 *
 * @param src source array to be copied
 *
 * @param dst destination array
 *
 * @param dstStart index of dst to start from
 *
 * @param srcStart index of src to start from
 *
 * @param srcEnd last index of src to copy, exclusive, i.e. src[srcEnd] is not copied
 *
 * @example: copy src to dst
 * ```
 * copyTo(src, dst, 0, 0, src.length)
 * ```
 */
// TODO(petr-shumilov): make intrinsic
export function copyTo(src: double[], dst: double[], dstStart: int, srcStart: int, srcEnd: int): void throws {
    if (!checkRange(src.length, srcStart, srcEnd)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: src bounds verification failed")
    }
    if (!checkRange(dst.length, dstStart, dst.length)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: dst bounds verification failed")
    }
    if (!((srcEnd - srcStart) <= (dst.length - dstStart))) {
        throw new ArrayIndexOutOfBoundsException("Destination array must have enough space")
    }

    let j: int = dstStart;
    for (let i: int = srcStart; i < srcEnd; i++) {
        dst[j] = src[i];
        j++;
    }
}

/**
 * creates a copy of src array with respect to passed indexes.
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr to copy, exclusive, i.e. arr[endIndex] is not copied
 *
 * @returns copy of arr[startIndex; endIndex)
 *
 * @example: create exact copy of arr
 * ```
 * copyOf(arr, 0, arr.length)
 * ```
 */
export function copyOf(arr: double[], startIndex: int, endIndex: int): double[] throws {
    let res: double[] = new double[endIndex - startIndex];
    copyTo(arr, res, 0, startIndex, endIndex);
    return res;
}

/*
 * creates a copy of src array with respect to passed index
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of src to start from
 *
 * @returns copy of arr[startIndex; src.length)
 *
 * @example: create copy of arr without first element
 * ```
 * copyOf(arr, 1)
 * ```
 */
export function copyOf(arr: double[], startIndex: int): double[] throws {
    return copyOf(arr, startIndex, arr.length);
}

/*
 * copyOf(arr: double[]) creates a copy of src array
 *
 * @param arr source array to be copied
 *
 * @returns copy of arr
 */
export function copyOf(arr: double[]): double[] {
    try {
        return copyOf(arr, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "copyOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return new double[0]
}


function bubbleSort(arr: double[], startIndex: int, endIndex: int): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if ((arr[i + 1] < arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: double[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if ((arr[startIndex + idxFromStart] < arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex)
}

function heapSortDown(arr: double[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && (arr[startIndex + best] < arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && (arr[startIndex + best] < arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: double[], startIndex: int, endIndex: int): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i)
    }
}

function quickSortSplit(arr: double[], startIndex: int, endIndex: int): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while ((arr[i] < pivot)) {
            i++
        }
        while ((pivot < arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if ((arr[i] < pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: double[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: double[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex)
}

function quickSort(arr: double[], startIndex: int, endIndex: int): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: double[], startIndex: int, endIndex: int): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex);
}


function bubbleSort(arr: double[], startIndex: int, endIndex: int, mustPrecede: (lhs: double, rhs: double) => boolean): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (mustPrecede(arr[i + 1], arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: double[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: double, rhs: double) => boolean): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if (mustPrecede(arr[startIndex + idxFromStart], arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex, mustPrecede)
}

function heapSortDown(arr: double[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: double, rhs: double) => boolean): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: double[], startIndex: int, endIndex: int, mustPrecede: (lhs: double, rhs: double) => boolean): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1, mustPrecede)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i, mustPrecede)
    }
}

function quickSortSplit(arr: double[], startIndex: int, endIndex: int, mustPrecede: (lhs: double, rhs: double) => boolean): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while (mustPrecede(arr[i], pivot)) {
            i++
        }
        while (mustPrecede(pivot, arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if (mustPrecede(arr[i], pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: double[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: double, rhs: double) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: double[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: double, rhs: double) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex, mustPrecede)
}

function quickSort(arr: double[], startIndex: int, endIndex: int, mustPrecede: (lhs: double, rhs: double) => boolean): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits, mustPrecede)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: double[], startIndex: int, endIndex: int, mustPrecede: (lhs: double, rhs: double) => boolean): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex, mustPrecede);
}



/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 */
export function sort(arr: double[], mustPrecede: (lhs: double, rhs: double) => boolean): void {
    sort(arr, 0, arr.length, mustPrecede);
}

export function sort(arr: double[], startIndex: int, mustPrecede: (lhs: double, rhs: double) => boolean): void {
    sort(arr, startIndex, arr.length, mustPrecede)
}



export function sort(arr: double[], startIndex: int): void {
    sort(arr, startIndex, arr.length)
}

export function sort(arr: double[]): void {
    sort(arr, 0, arr.length)
}

// ======== tests section ========

/** note: used for tests, {@link test_sortAllOn} */
class test_SortDatadouble {
    name: string
    arr: double[]

    constructor(name: string, arr: double[]) {
        this.name = name
        this.arr = arr
    }
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortCopy(arr: double[]): double[] {
    let c = new double[arr.length]
    for (let i = 0; i < arr.length; i++) {
        c[i] = arr[i]
    }
    return c
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpFwd(l: double, r: double): boolean {
    return l < r
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpInv(l: double, r: double): boolean {
    return r < l
}

/** note: used for tests, {@link test_sortAllOn} */
function test_printArr(arr: double[], startIndex: int, endIndex: int) {
    for (let i = startIndex; i < endIndex; i++) {
        console.print(arr[i] + ' ')
    }
    console.println('')
}

/**
 * Function used to test sorting in standard library tests.
 * There is only one exported function: `sort`, but for testing
 * we need access to all sub sorts that are used. Hence this part of "tests"
 * is located here. All related entities are prefixed whith `test_` to stand out
 */
export function test_sortAllOn(arr: double[]) {
    // block for comparator ``
    if (true) {
        const sorts: test_SortDatadouble[] = new test_SortDatadouble[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length)
        sorts[lastSort++] = new test_SortDatadouble("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length)
        sorts[lastSort++] = new test_SortDatadouble("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length)
        sorts[lastSort++] = new test_SortDatadouble("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length)
        sorts[lastSort++] = new test_SortDatadouble("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just)
        sorts[lastSort++] = new test_SortDatadouble("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts  are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for double')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpFwd`
    if (true) {
        const sorts: test_SortDatadouble[] = new test_SortDatadouble[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatadouble("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatadouble("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatadouble("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatadouble("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatadouble("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpFwd are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for double')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpInv`
    if (true) {
        const sorts: test_SortDatadouble[] = new test_SortDatadouble[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatadouble("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatadouble("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatadouble("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatadouble("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatadouble("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpInv are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for double')
                }
            }
        }
    }
}
// ======== end of tests section ========


/**
 *  tries to find key in arr with respect of passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index of key if found in arr[startIndex; endIndex), otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4, 0, 3) == KEY_NOT_FOUND // true since value 4 is not in half-open range [0,3)
 * ```
 */
export function indexOf(arr: double[], key: double, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("indexOf: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find key in arr, starting from index
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function indexOf(arr: double[], key: double, fromIndex: int): int throws {
    return indexOf(arr, key, fromIndex, arr.length)
}

/*
 * tries to find key in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4) == 3
 * ```
 */
export function indexOf(arr: double[], key: double): int {
    try {
        return indexOf(arr, key, 0, arr.length)
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "indexOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return KEY_NOT_FOUND
}

/*
 * checks whether key is in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns true if key is in arr, false otherwise
 *
 * @example
 * ```
 * includes([1,2,3,4], 4) == true
 * ```
 */
export function includes(arr: double[], key: double): boolean {
    return indexOf(arr, key) != KEY_NOT_FOUND;
}

/*
 * tries to find last entry of a key into arr with respect to passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param smallerIndex an index of arr to end search with
 *
 * @param largerIndex a last index to start search in arr
 *
 * @returns index of key if found in arr(smallerIndex; largerIndex], otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: double[], key: double, smallerIndex: int, largerIndex: int): int throws {
    if (!checkRange(arr.length, smallerIndex + 1, largerIndex + 1)) {
        throw new ArrayIndexOutOfBoundsException("lastIndexOf: bounds verification failed")
    }

    for (let i: int = largerIndex; i > smallerIndex; i--) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find last entry of key into arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: double[], key: double): int {
    try {
        return lastIndexOf(arr, key, arr.length - 1)
    } catch (e) {
        assert false : "can't happen"
    }
}

/*
 * lastIndexOf(arr: double[], key: double, fromIndex: int) tries to find entry of key into arr which is not greater than fromIndex
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with (including, search is performed backwards)
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
function lastIndexOf(arr: double[], key: double, fromIndex: int): int throws {
    return lastIndexOf(arr, key, -1, fromIndex)
}

// C++ semantics
// (lower|upper)BoundSearch([1, 1, 2, 2, 2, 3, 3], 2, 0, 7)
//                                 ^        ^
//                                 |        |
//                                 |    upper bound
//                             lower bound


/**
 * tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is endIndex
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than endIndex
 */
export function lowerBoundSearch(arr: double[], key: double, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("lowerBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] < key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 *  tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is arr.length
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than arr.length
 */
export function lowerBoundSearch(arr: double[], key: double): int {
    try {
        return lowerBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "lowerBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than endIndex
 */
export function upperBoundSearch(arr: double[], key: double, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("upperBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] <= key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than arr.length
 */
export function upperBoundSearch(arr: double[], key: double): int {
    try {
        return upperBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "upperBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}


/**
 * concatenates two arrays into a single one
 *
 * @param a first array to concatenate
 *
 * @param b second array to concatenate
 *
 * @returns concatenation of the arguments
 */
export function concat(a: double[], b: double[]): double[] {
    let res: double[] = new double[a.length + b.length]
    try {
        copyTo(a, res, 0, 0, a.length);
        copyTo(b, res, a.length, 0, b.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "concat: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return res;
}

/**
 * takes an array and reverses it in-place
 *
 * @param arr an array to be reversed
 */
export function reverse(arr: double[]): void {
    let arrLen: int = arr.length;
    for (let i: int = 0; i < arrLen / 2; i++) {
        let tmp: double = arr[i];
        arr[i] = arr[arrLen - i - 1];
        arr[arrLen - i - 1] = tmp;
    }
}

/**
 * Joins elements with delimeter
 *
 * @param arr array to be joined in String
 *
 * @param delimiter a string to insert between elements of arr
 *
 * @returns joined elements of arr with delimiter between
 */

export function join(arr: double[], delimiter: String): String {
    let sb: StringBuilder = new StringBuilder();
    let len: int = arr.length;
    for (let i: int = 0; i < len; i++) {
        sb.append(arr[i])
        if (i != len - 1) {
            sb.append(delimiter);
        }
    }
    return sb.toString();
}


/**
 * runs fn for elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function forEach(arr: double[], fn: (value: double, index: int, self: double[]) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i], i, arr);
    }
}

/**
 * runs fn over all elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 */
export function forEach(arr: double[], fn: (value: double) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i]);
    }
}

/**
 * updates element arr[i] using fn
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function map(arr: double[], fn: (value: double, index: int, self: double[]) => double): double[] {
    let res: double[] = new double[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i], i, arr);
    }
    return res;
}

/**
 * creates a new array using fn(arr[i])
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply for each element of arr
 *
 * @returns double[] - a copy of arr where for each element fn was applied
 */
export function map(arr: double[], fn: (value: double) => double): double[] {
    let res: double[] = new double[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i]);
    }
    return res;
}

/**
 * creates an array only with elements of arr where fn applied to element returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a filter with use of adjacent elements
 *
 * @returns an array of elements from arr where fn returned true
 */
export function filter(arr: double[], fn: (value: double, index: int, self: double[]) => boolean): double[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i], i, arr)) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: double[] = new double[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

/**
 * creates an array only with elements of arr where fn(arr[i]) returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition for value
 *
 * @returns an array of elements from arr where fn(arr[i]) returned true
 */
export function filter(arr: double[], fn: (value: double) => boolean): double[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i])) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: double[] = new double[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}


//
// Methods for char[]
//

/**
 * fills arr in-place by specified value with respect to passed indexes
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr, exclusive, i.e. arr[endIndex] is not changed
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value, 0, arr.length)
 * ```
 */
export function fill(arr: char[], value: char, startIndex: int, endIndex: int): void throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("fill: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        arr[i] = value;
    }
}

/**
 * fills arr in-place by specified value
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value)
 * ```
 */
export function fill(arr: char[], value: char): void {
    try {
        fill(arr, value, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "fill: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
}

/**
 * copies src array into dst with respect to passed indexes.
 * dst must have enough space, otherwise out-of-bounds might occur
 *
 * @param src source array to be copied
 *
 * @param dst destination array
 *
 * @param dstStart index of dst to start from
 *
 * @param srcStart index of src to start from
 *
 * @param srcEnd last index of src to copy, exclusive, i.e. src[srcEnd] is not copied
 *
 * @example: copy src to dst
 * ```
 * copyTo(src, dst, 0, 0, src.length)
 * ```
 */
// TODO(petr-shumilov): make intrinsic
export function copyTo(src: char[], dst: char[], dstStart: int, srcStart: int, srcEnd: int): void throws {
    if (!checkRange(src.length, srcStart, srcEnd)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: src bounds verification failed")
    }
    if (!checkRange(dst.length, dstStart, dst.length)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: dst bounds verification failed")
    }
    if (!((srcEnd - srcStart) <= (dst.length - dstStart))) {
        throw new ArrayIndexOutOfBoundsException("Destination array must have enough space")
    }

    let j: int = dstStart;
    for (let i: int = srcStart; i < srcEnd; i++) {
        dst[j] = src[i];
        j++;
    }
}

/**
 * creates a copy of src array with respect to passed indexes.
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr to copy, exclusive, i.e. arr[endIndex] is not copied
 *
 * @returns copy of arr[startIndex; endIndex)
 *
 * @example: create exact copy of arr
 * ```
 * copyOf(arr, 0, arr.length)
 * ```
 */
export function copyOf(arr: char[], startIndex: int, endIndex: int): char[] throws {
    let res: char[] = new char[endIndex - startIndex];
    copyTo(arr, res, 0, startIndex, endIndex);
    return res;
}

/*
 * creates a copy of src array with respect to passed index
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of src to start from
 *
 * @returns copy of arr[startIndex; src.length)
 *
 * @example: create copy of arr without first element
 * ```
 * copyOf(arr, 1)
 * ```
 */
export function copyOf(arr: char[], startIndex: int): char[] throws {
    return copyOf(arr, startIndex, arr.length);
}

/*
 * copyOf(arr: char[]) creates a copy of src array
 *
 * @param arr source array to be copied
 *
 * @returns copy of arr
 */
export function copyOf(arr: char[]): char[] {
    try {
        return copyOf(arr, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "copyOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return new char[0]
}


function bubbleSort(arr: char[], startIndex: int, endIndex: int): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if ((arr[i + 1] < arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: char[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if ((arr[startIndex + idxFromStart] < arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex)
}

function heapSortDown(arr: char[], idxFromStart: int, startIndex: int, endIndex: int): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && (arr[startIndex + best] < arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && (arr[startIndex + best] < arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: char[], startIndex: int, endIndex: int): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i)
    }
}

function quickSortSplit(arr: char[], startIndex: int, endIndex: int): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while ((arr[i] < pivot)) {
            i++
        }
        while ((pivot < arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if ((arr[i] < pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: char[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: char[], startIndex: int, endIndex: int, bits: int): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex)
}

function quickSort(arr: char[], startIndex: int, endIndex: int): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: char[], startIndex: int, endIndex: int): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex);
}


function bubbleSort(arr: char[], startIndex: int, endIndex: int, mustPrecede: (lhs: char, rhs: char) => boolean): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (mustPrecede(arr[i + 1], arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: char[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: char, rhs: char) => boolean): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if (mustPrecede(arr[startIndex + idxFromStart], arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex, mustPrecede)
}

function heapSortDown(arr: char[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: char, rhs: char) => boolean): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: char[], startIndex: int, endIndex: int, mustPrecede: (lhs: char, rhs: char) => boolean): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1, mustPrecede)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i, mustPrecede)
    }
}

function quickSortSplit(arr: char[], startIndex: int, endIndex: int, mustPrecede: (lhs: char, rhs: char) => boolean): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while (mustPrecede(arr[i], pivot)) {
            i++
        }
        while (mustPrecede(pivot, arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if (mustPrecede(arr[i], pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: char[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: char, rhs: char) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: char[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: char, rhs: char) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex, mustPrecede)
}

function quickSort(arr: char[], startIndex: int, endIndex: int, mustPrecede: (lhs: char, rhs: char) => boolean): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits, mustPrecede)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: char[], startIndex: int, endIndex: int, mustPrecede: (lhs: char, rhs: char) => boolean): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex, mustPrecede);
}



/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 */
export function sort(arr: char[], mustPrecede: (lhs: char, rhs: char) => boolean): void {
    sort(arr, 0, arr.length, mustPrecede);
}

export function sort(arr: char[], startIndex: int, mustPrecede: (lhs: char, rhs: char) => boolean): void {
    sort(arr, startIndex, arr.length, mustPrecede)
}



export function sort(arr: char[], startIndex: int): void {
    sort(arr, startIndex, arr.length)
}

export function sort(arr: char[]): void {
    sort(arr, 0, arr.length)
}

// ======== tests section ========

/** note: used for tests, {@link test_sortAllOn} */
class test_SortDatachar {
    name: string
    arr: char[]

    constructor(name: string, arr: char[]) {
        this.name = name
        this.arr = arr
    }
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortCopy(arr: char[]): char[] {
    let c = new char[arr.length]
    for (let i = 0; i < arr.length; i++) {
        c[i] = arr[i]
    }
    return c
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpFwd(l: char, r: char): boolean {
    return l < r
}

/** note: used for tests, {@link test_sortAllOn} */
function test_sortAllOnCmpInv(l: char, r: char): boolean {
    return r < l
}

/** note: used for tests, {@link test_sortAllOn} */
function test_printArr(arr: char[], startIndex: int, endIndex: int) {
    for (let i = startIndex; i < endIndex; i++) {
        console.print(arr[i] + ' ')
    }
    console.println('')
}

/**
 * Function used to test sorting in standard library tests.
 * There is only one exported function: `sort`, but for testing
 * we need access to all sub sorts that are used. Hence this part of "tests"
 * is located here. All related entities are prefixed whith `test_` to stand out
 */
export function test_sortAllOn(arr: char[]) {
    // block for comparator ``
    if (true) {
        const sorts: test_SortDatachar[] = new test_SortDatachar[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length)
        sorts[lastSort++] = new test_SortDatachar("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length)
        sorts[lastSort++] = new test_SortDatachar("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length)
        sorts[lastSort++] = new test_SortDatachar("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length)
        sorts[lastSort++] = new test_SortDatachar("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just)
        sorts[lastSort++] = new test_SortDatachar("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts  are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for char')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpFwd`
    if (true) {
        const sorts: test_SortDatachar[] = new test_SortDatachar[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatachar("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatachar("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatachar("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatachar("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpFwd)
        sorts[lastSort++] = new test_SortDatachar("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpFwd are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for char')
                }
            }
        }
    }
    // block for comparator `est_sortAllOnCmpInv`
    if (true) {
        const sorts: test_SortDatachar[] = new test_SortDatachar[10]
        let lastSort = 0
        const bubbled = test_sortCopy(arr)
        bubbleSort(bubbled, 0, bubbled.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatachar("bubble", bubbled)

        
        const heaped = test_sortCopy(arr)
        heapSort(heaped, 0, heaped.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatachar("heap", heaped)

        const quicked = test_sortCopy(arr)
        quickSortImpl1(quicked, 0, quicked.length, quicked.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatachar("quick", quicked)

        const quickedBuble = test_sortCopy(arr)
        quickSortImpl15(quickedBuble, 0, quickedBuble.length, quickedBuble.length, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatachar("quick with bubble", quickedBuble)

        const just = test_sortCopy(arr)
        sort(just, test_sortAllOnCmpInv)
        sorts[lastSort++] = new test_SortDatachar("sort()", just)

        for (let s = 1; s < lastSort; s++) {
            for (let i = 0; i < arr.length; i++) {
                if (sorts[0].arr[i] != sorts[s].arr[i]) {
                    console.println(sorts[0].name + ': ')
                    test_printArr(sorts[0].arr, 0, arr.length)
                    console.println(sorts[s].name + ': ')
                    test_printArr(sorts[s].arr, 0, arr.length)
                    throw new Error("sorts est_sortAllOnCmpInv are not equal: " + sorts[0].name + ' ' + sorts[s].name + ' for char')
                }
            }
        }
    }
}
// ======== end of tests section ========


/**
 *  tries to find key in arr with respect of passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index of key if found in arr[startIndex; endIndex), otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4, 0, 3) == KEY_NOT_FOUND // true since value 4 is not in half-open range [0,3)
 * ```
 */
export function indexOf(arr: char[], key: char, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("indexOf: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find key in arr, starting from index
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function indexOf(arr: char[], key: char, fromIndex: int): int throws {
    return indexOf(arr, key, fromIndex, arr.length)
}

/*
 * tries to find key in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4) == 3
 * ```
 */
export function indexOf(arr: char[], key: char): int {
    try {
        return indexOf(arr, key, 0, arr.length)
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "indexOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return KEY_NOT_FOUND
}

/*
 * checks whether key is in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns true if key is in arr, false otherwise
 *
 * @example
 * ```
 * includes([1,2,3,4], 4) == true
 * ```
 */
export function includes(arr: char[], key: char): boolean {
    return indexOf(arr, key) != KEY_NOT_FOUND;
}

/*
 * tries to find last entry of a key into arr with respect to passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param smallerIndex an index of arr to end search with
 *
 * @param largerIndex a last index to start search in arr
 *
 * @returns index of key if found in arr(smallerIndex; largerIndex], otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: char[], key: char, smallerIndex: int, largerIndex: int): int throws {
    if (!checkRange(arr.length, smallerIndex + 1, largerIndex + 1)) {
        throw new ArrayIndexOutOfBoundsException("lastIndexOf: bounds verification failed")
    }

    for (let i: int = largerIndex; i > smallerIndex; i--) {
        if (arr[i] == key) {
            return i;
        }
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find last entry of key into arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: char[], key: char): int {
    try {
        return lastIndexOf(arr, key, arr.length - 1)
    } catch (e) {
        assert false : "can't happen"
    }
}

/*
 * lastIndexOf(arr: char[], key: char, fromIndex: int) tries to find entry of key into arr which is not greater than fromIndex
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with (including, search is performed backwards)
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
function lastIndexOf(arr: char[], key: char, fromIndex: int): int throws {
    return lastIndexOf(arr, key, -1, fromIndex)
}

// C++ semantics
// (lower|upper)BoundSearch([1, 1, 2, 2, 2, 3, 3], 2, 0, 7)
//                                 ^        ^
//                                 |        |
//                                 |    upper bound
//                             lower bound


/**
 * tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is endIndex
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than endIndex
 */
export function lowerBoundSearch(arr: char[], key: char, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("lowerBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] < key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 *  tries to find a lower bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Lower bound is an index of a first element, where (element < key) is false. If no such element is found than lower bound is arr.length
 *
 * @param arr array to find a lower bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find lower bound of
 *
 * @returns index such (arr[index] < key) is false. If no such index is found than arr.length
 */
export function lowerBoundSearch(arr: char[], key: char): int {
    try {
        return lowerBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "lowerBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than endIndex
 */
export function upperBoundSearch(arr: char[], key: char, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("upperBoundSearch: bounds verification failed")
    }

    let left: int = startIndex;
    let len: int = endIndex - startIndex;

    while (len > 0) {
        let half: int = len >>> 1;
        let middle: int = left + half;

        if (arr[middle] <= key) {
            left = middle + 1;
            len -= half + 1;
        } else {
            len = half;
        }
    }

    return left;
}

/**
 * tries to find an upper bound of a key in sorted arr.
 * The array has to be sorted before calling this function.
 * Upper bound is an index of a first element, where (key < element) is true. If no such element is found than upper bound is endIndex
 *
 * @param arr array to find a upper bound of a key. Has to be sorted, otherwise the answer is implementation-defined and may be incorrect
 *
 * @param key a value to find upper bound of. It may be not in arr, upper bound will present anyway
 *
 * @returns index such (key < arr[index]) is true. If no such index is found than arr.length
 */
export function upperBoundSearch(arr: char[], key: char): int {
    try {
        return upperBoundSearch(arr, key, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "upperBoundSearch: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return arr.length
}


/**
 * concatenates two arrays into a single one
 *
 * @param a first array to concatenate
 *
 * @param b second array to concatenate
 *
 * @returns concatenation of the arguments
 */
export function concat(a: char[], b: char[]): char[] {
    let res: char[] = new char[a.length + b.length]
    try {
        copyTo(a, res, 0, 0, a.length);
        copyTo(b, res, a.length, 0, b.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "concat: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return res;
}

/**
 * takes an array and reverses it in-place
 *
 * @param arr an array to be reversed
 */
export function reverse(arr: char[]): void {
    let arrLen: int = arr.length;
    for (let i: int = 0; i < arrLen / 2; i++) {
        let tmp: char = arr[i];
        arr[i] = arr[arrLen - i - 1];
        arr[arrLen - i - 1] = tmp;
    }
}

/**
 * Joins elements with delimeter
 *
 * @param arr array to be joined in String
 *
 * @param delimiter a string to insert between elements of arr
 *
 * @returns joined elements of arr with delimiter between
 */

export function join(arr: char[], delimiter: String): String {
    let sb: StringBuilder = new StringBuilder();
    let len: int = arr.length;
    for (let i: int = 0; i < len; i++) {
        sb.append(arr[i])
        if (i != len - 1) {
            sb.append(delimiter);
        }
    }
    return sb.toString();
}


/**
 * runs fn for elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function forEach(arr: char[], fn: (value: char, index: int, self: char[]) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i], i, arr);
    }
}

/**
 * runs fn over all elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 */
export function forEach(arr: char[], fn: (value: char) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i]);
    }
}

/**
 * updates element arr[i] using fn
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function map(arr: char[], fn: (value: char, index: int, self: char[]) => char): char[] {
    let res: char[] = new char[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i], i, arr);
    }
    return res;
}

/**
 * creates a new array using fn(arr[i])
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply for each element of arr
 *
 * @returns char[] - a copy of arr where for each element fn was applied
 */
export function map(arr: char[], fn: (value: char) => char): char[] {
    let res: char[] = new char[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i]);
    }
    return res;
}

/**
 * creates an array only with elements of arr where fn applied to element returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a filter with use of adjacent elements
 *
 * @returns an array of elements from arr where fn returned true
 */
export function filter(arr: char[], fn: (value: char, index: int, self: char[]) => boolean): char[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i], i, arr)) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: char[] = new char[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

/**
 * creates an array only with elements of arr where fn(arr[i]) returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition for value
 *
 * @returns an array of elements from arr where fn(arr[i]) returned true
 */
export function filter(arr: char[], fn: (value: char) => boolean): char[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i])) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: char[] = new char[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}


//
// Methods for Object[]
//

/**
 * fills arr in-place by specified value with respect to passed indexes
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr, exclusive, i.e. arr[endIndex] is not changed
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value, 0, arr.length)
 * ```
 */
export function fill(arr: Object[], value: Object, startIndex: int, endIndex: int): void throws {
    if (!checkRange(arr.length, startIndex, endIndex)){
        throw new ArrayIndexOutOfBoundsException("fill: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        arr[i] = value;
    }
}

/**
 * fills arr in-place by specified value
 *
 * @param arr source array to be filled
 *
 * @param value a value to fill arr with
 *
 * @example: fill all content of arr with value
 * ```
 * fill(arr, value)
 * ```
 */
export function fill(arr: Object[], value: Object): void {
    try {
        fill(arr, value, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "fill: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
}

/**
 * copies src array into dst with respect to passed indexes.
 * dst must have enough space, otherwise out-of-bounds might occur
 *
 * @param src source array to be copied
 *
 * @param dst destination array
 *
 * @param dstStart index of dst to start from
 *
 * @param srcStart index of src to start from
 *
 * @param srcEnd last index of src to copy, exclusive, i.e. src[srcEnd] is not copied
 *
 * @example: copy src to dst
 * ```
 * copyTo(src, dst, 0, 0, src.length)
 * ```
 */
// TODO(petr-shumilov): make intrinsic
export function copyTo(src: Object[], dst: Object[], dstStart: int, srcStart: int, srcEnd: int): void throws {
    if (!checkRange(src.length, srcStart, srcEnd)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: src bounds verification failed")
    }
    if (!checkRange(dst.length, dstStart, dst.length)) {
        throw new ArrayIndexOutOfBoundsException("copyTo: dst bounds verification failed")
    }
    if (!((srcEnd - srcStart) <= (dst.length - dstStart))) {
        throw new ArrayIndexOutOfBoundsException("Destination array must have enough space")
    }

    let j: int = dstStart;
    for (let i: int = srcStart; i < srcEnd; i++) {
        dst[j] = src[i];
        j++;
    }
}

/**
 * creates a copy of src array with respect to passed indexes.
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of arr to start from
 *
 * @param endIndex last index of arr to copy, exclusive, i.e. arr[endIndex] is not copied
 *
 * @returns copy of arr[startIndex; endIndex)
 *
 * @example: create exact copy of arr
 * ```
 * copyOf(arr, 0, arr.length)
 * ```
 */
export function copyOf(arr: Object[], startIndex: int, endIndex: int): Object[] throws {
    let res: Object[] = new Object[endIndex - startIndex];
    copyTo(arr, res, 0, startIndex, endIndex);
    return res;
}

/*
 * creates a copy of src array with respect to passed index
 *
 * @param arr source array to be copied
 *
 * @param startIndex index of src to start from
 *
 * @returns copy of arr[startIndex; src.length)
 *
 * @example: create copy of arr without first element
 * ```
 * copyOf(arr, 1)
 * ```
 */
export function copyOf(arr: Object[], startIndex: int): Object[] throws {
    return copyOf(arr, startIndex, arr.length);
}

/*
 * copyOf(arr: Object[]) creates a copy of src array
 *
 * @param arr source array to be copied
 *
 * @returns copy of arr
 */
export function copyOf(arr: Object[]): Object[] {
    try {
        return copyOf(arr, 0, arr.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "copyOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return new Object[0]
}


function bubbleSort(arr: Object[], startIndex: int, endIndex: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    let was = true
    while (was) {
        was = false
        for (let i = startIndex; i < endIndex - 1; i++) {
            if (mustPrecede(arr[i + 1], arr[i])) {
                const tmp = arr[i + 1]
                arr[i + 1] = arr[i]
                arr[i] = tmp
                was = true
            }
        }
    }
}
function heapSortUp(arr: Object[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    while (idxFromStart != 0) {
        const p = (idxFromStart - 1) / 2
        if (mustPrecede(arr[startIndex + idxFromStart], arr[startIndex + p])) {
            break
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + p]
        arr[startIndex + p] = tmp
        idxFromStart = p
    }
    heapSortDown(arr, idxFromStart, startIndex, endIndex, mustPrecede)
}

function heapSortDown(arr: Object[], idxFromStart: int, startIndex: int, endIndex: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    while (true) {
        const l = idxFromStart * 2 + 1
        const r = idxFromStart * 2 + 2
        let best = idxFromStart
        if (startIndex + l < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + l])) {
            best = l
        }
        if (startIndex + r < endIndex && mustPrecede(arr[startIndex + best], arr[startIndex + r])) {
            best = r
        }
        if (best == idxFromStart) {
            return
        }
        const tmp = arr[startIndex + idxFromStart]
        arr[startIndex + idxFromStart] = arr[startIndex + best]
        arr[startIndex + best] = tmp
        idxFromStart = best
    }
}

function heapSort(arr: Object[], startIndex: int, endIndex: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    let len = endIndex - startIndex
    for (let i = 1; i < len; i++) {
        heapSortUp(arr, i, startIndex, startIndex + i + 1, mustPrecede)
    }
    for (let i = len - 1; i >= 0; i--) {
        const tmp = arr[startIndex + i]
        arr[startIndex + i] = arr[startIndex]
        arr[startIndex] = tmp
        heapSortDown(arr, 0, startIndex, startIndex + i, mustPrecede)
    }
}

function quickSortSplit(arr: Object[], startIndex: int, endIndex: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): int {
    const pivot = arr[startIndex + (endIndex - startIndex) / 2]
    let i = startIndex
    let j = endIndex - 1
    while (i < j) {
        while (mustPrecede(arr[i], pivot)) {
            i++
        }
        while (mustPrecede(pivot, arr[j])) {
            j--
        }
        if (i >= j) {
            break
        }
        let tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
        // we must not leave pivot outside of [i, j] range
        if (mustPrecede(arr[i], pivot)) {
            i++
        } else { // arr[i] == pivot
            j--
        }
    }

    return j
}


function quickSortImpl1(arr: Object[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 1) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl1(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl1(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
}
function quickSortImpl15(arr: Object[], startIndex: int, endIndex: int, bits: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    const bits1 = bits * 3

    let iter = 0
    while (endIndex - startIndex > 15) {
        if (iter++ > bits1) {
            heapSort(arr, startIndex, endIndex, mustPrecede)
            return
        }

        let p = quickSortSplit(arr, startIndex, endIndex, mustPrecede)
        if (p - startIndex < endIndex - p) {
            quickSortImpl15(arr, startIndex, p, bits - 1, mustPrecede)
            startIndex = p
        } else {
            quickSortImpl15(arr, p, endIndex, bits - 1, mustPrecede)
            endIndex = p
        }
    }
    bubbleSort(arr, startIndex, endIndex, mustPrecede)
}

function quickSort(arr: Object[], startIndex: int, endIndex: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    let size = endIndex - startIndex
    if (size == 0) {
        return
    }
    // find log of length to fall back into determenistic O(n logn) sort
    size -= 1
    size |= size >> 1
    size |= size >> 2
    size |= size >> 4
    size |= size >> 8
    size |= size >> 16
    size += 1
    let bits = -1
    for (let i = 0; i < 31; i++) {
        if ((size & 1 << i) != 0) {
            bits = i
            break
        }
    }
    assert bits != -1
    quickSortImpl15(arr, startIndex, endIndex, bits, mustPrecede)
}

/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 *
 * @param startIndex an index to start sorting with, inclusive
 *
 * @param endIndex an index to end sorting, exclusive
 *
 * @example: sort array arr
 * ```
 * sort(arr, 0, arr.length)
 * ```
 */
export function sort(arr: Object[], startIndex: int, endIndex: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsError("sort: bounds verification failed")
    }

    
    
    quickSort(arr, startIndex, endIndex, mustPrecede);
}



/**
 * sorts arr in-place
 *
 * @param arr an array to sort
 */
export function sort(arr: Object[], mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    sort(arr, 0, arr.length, mustPrecede);
}

export function sort(arr: Object[], startIndex: int, mustPrecede: (lhs: Object, rhs: Object) => boolean): void {
    sort(arr, startIndex, arr.length, mustPrecede)
}



/**
 *  tries to find key in arr with respect of passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param startIndex an index of arr to begin search with
 *
 * @param endIndex a last index to stop search in arr, i.e. arr[endIndex] is not checked
 *
 * @returns index of key if found in arr[startIndex; endIndex), otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4, 0, 3) == KEY_NOT_FOUND // true since value 4 is not in half-open range [0,3)
 * ```
 */
export function indexOf(arr: Object[], key: Object, startIndex: int, endIndex: int): int throws {
    if (!checkRange(arr.length, startIndex, endIndex)) {
        throw new ArrayIndexOutOfBoundsException("indexOf: bounds verification failed")
    }

    for (let i: int = startIndex; i < endIndex; i++) {
        
        if (arr[i].equals(key)) {
            return i;
        } 
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find key in arr, starting from index
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function indexOf(arr: Object[], key: Object, fromIndex: int): int throws {
    return indexOf(arr, key, fromIndex, arr.length)
}

/*
 * tries to find key in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns index of key if found in arr, otherwise KEY_NOT_FOUND
 *
 * @example
 * ```
 * indexOf([1,2,3,4], 4) == 3
 * ```
 */
export function indexOf(arr: Object[], key: Object): int {
    try {
        return indexOf(arr, key, 0, arr.length)
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "indexOf: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return KEY_NOT_FOUND
}

/*
 * checks whether key is in arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns true if key is in arr, false otherwise
 *
 * @example
 * ```
 * includes([1,2,3,4], 4) == true
 * ```
 */
export function includes(arr: Object[], key: Object): boolean {
    return indexOf(arr, key) != KEY_NOT_FOUND;
}

/*
 * tries to find last entry of a key into arr with respect to passed indexes
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param smallerIndex an index of arr to end search with
 *
 * @param largerIndex a last index to start search in arr
 *
 * @returns index of key if found in arr(smallerIndex; largerIndex], otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: Object[], key: Object, smallerIndex: int, largerIndex: int): int throws {
    if (!checkRange(arr.length, smallerIndex + 1, largerIndex + 1)) {
        throw new ArrayIndexOutOfBoundsException("lastIndexOf: bounds verification failed")
    }

    for (let i: int = largerIndex; i > smallerIndex; i--) {
        
        if (arr[i].equals(key)) {
            return i;
        } 
    }
    return KEY_NOT_FOUND;
}

/*
 * tries to find last entry of key into arr
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
export function lastIndexOf(arr: Object[], key: Object): int {
    try {
        return lastIndexOf(arr, key, arr.length - 1)
    } catch (e) {
        assert false : "can't happen"
    }
}

/*
 * lastIndexOf(arr: Object[], key: Object, fromIndex: int) tries to find entry of key into arr which is not greater than fromIndex
 *
 * @param arr array to find a key
 *
 * @param key a value to find
 *
 * @param fromIndex an index of arr to begin search with (including, search is performed backwards)
 *
 * @returns last index of key if found in arr, otherwise KEY_NOT_FOUND
 */
function lastIndexOf(arr: Object[], key: Object, fromIndex: int): int throws {
    return lastIndexOf(arr, key, -1, fromIndex)
}


/**
 * concatenates two arrays into a single one
 *
 * @param a first array to concatenate
 *
 * @param b second array to concatenate
 *
 * @returns concatenation of the arguments
 */
export function concat(a: Object[], b: Object[]): Object[] {
    let res: Object[] = new Object[a.length + b.length]
    try {
        copyTo(a, res, 0, 0, a.length);
        copyTo(b, res, a.length, 0, b.length);
    } catch (e) {
        // TODO(ivan-tyulyandin): code below is an overcheck, but will be helpful in case of strange exceptions
        assert false : "concat: should be unreacheable since indicies have to be correct by design, " + e.toString()
    }
    return res;
}

/**
 * takes an array and reverses it in-place
 *
 * @param arr an array to be reversed
 */
export function reverse(arr: Object[]): void {
    let arrLen: int = arr.length;
    for (let i: int = 0; i < arrLen / 2; i++) {
        let tmp: Object = arr[i];
        arr[i] = arr[arrLen - i - 1];
        arr[arrLen - i - 1] = tmp;
    }
}

/**
 * Joins elements with delimeter
 *
 * @param arr array to be joined in String
 *
 * @param delimiter a string to insert between elements of arr
 *
 * @returns joined elements of arr with delimiter between
 */

export function join(arr: Object[], delimiter: String): String {
    let sb: StringBuilder = new StringBuilder();
    let len: int = arr.length;
    for (let i: int = 0; i < len; i++) {
        sb.append(arr[i])
        if (i != len - 1) {
            sb.append(delimiter);
        }
    }
    return sb.toString();
}


/**
 * runs fn for elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function forEach(arr: Object[], fn: (value: Object, index: int, self: Object[]) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i], i, arr);
    }
}

/**
 * runs fn over all elements of arr
 *
 * @param arr array to run fn over
 *
 * @param fn a function to apply
 */
export function forEach(arr: Object[], fn: (value: Object) => void): void {
    for (let i: int = 0; i < arr.length; i++) {
        fn(arr[i]);
    }
}

/**
 * updates element arr[i] using fn
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply
 * fn takes a value, index of some array and reference to it. Can be used, e.g., to write a function with use of adjacent elements
 */
export function map(arr: Object[], fn: (value: Object, index: int, self: Object[]) => Object): Object[] {
    let res: Object[] = new Object[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i], i, arr);
    }
    return res;
}

/**
 * creates a new array using fn(arr[i])
 *
 * @param arr an array to run fn over
 *
 * @param fn a function to apply for each element of arr
 *
 * @returns Object[] - a copy of arr where for each element fn was applied
 */
export function map(arr: Object[], fn: (value: Object) => Object): Object[] {
    let res: Object[] = new Object[arr.length];
    for (let i: int = 0; i < arr.length; i++) {
        res[i] = fn(arr[i]);
    }
    return res;
}

/**
 * creates an array only with elements of arr where fn applied to element returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition
 * fn takes a value to test, index of some array and reference to it. Can be used, e.g., to write a filter with use of adjacent elements
 *
 * @returns an array of elements from arr where fn returned true
 */
export function filter(arr: Object[], fn: (value: Object, index: int, self: Object[]) => boolean): Object[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i], i, arr)) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: Object[] = new Object[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

/**
 * creates an array only with elements of arr where fn(arr[i]) returned true
 *
 * @param arr array to be filtered
 *
 * @param fn a filter condition for value
 *
 * @returns an array of elements from arr where fn(arr[i]) returned true
 */
export function filter(arr: Object[], fn: (value: Object) => boolean): Object[] {
    let resNum: int = 0;
    let indicator: boolean[] = new boolean[arr.length];

    for (let i: int = 0; i < arr.length; i++) {
        if (fn(arr[i])) {
            indicator[i] = true;
            resNum++;
        }
    }

    let res: Object[] = new Object[resNum];
    let j: int = 0;
    for (let i: int = 0; i < arr.length; i++) {
        if (indicator[i]) {
            res[j] = arr[i];
            j++;
        }
    }

    return res;
}

