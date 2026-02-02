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
 * @file Defines the ReadonlyArray for ArkTS
 * @kit ArkTS
 */


package std.core;

/**
 * Describes a read-only array that does not allow modifying elements by index or changing the array length.
 *
 * @extends ConcatArray<T>
 * @interface ReadonlyArray<out T>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export interface ReadonlyArray<out T> extends ConcatArray<T> {
    /**
     * Merges the current read-only array with one or more arrays and returns a new array.
     *
     * @param {...FixedArray<ConcatArray<@types.SkipVarianceCheckDontUseItWillBeRemoved T>>}
     *     items Arrays or values to be merged into the current read-only array.
     * @returns { Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T> }
     *     A new array containing the merged elements of the current read-only array and all specified arrays.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    concat(...items: FixedArray<ConcatArray<@types.SkipVarianceCheckDontUseItWillBeRemoved T>>): Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T>;
    // Typing and previous overload have to cover it

    /**
     * Returns an iterator that contains key-value pairs (index and value) for each element in the array.
     *
     * @returns { IterableIterator<[number, @types.SkipVarianceCheckDontUseItWillBeRemoved T]> }
     *     A new array iterator object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    entries(): IterableIterator<[number, @types.SkipVarianceCheckDontUseItWillBeRemoved T]>;

    /**
     * Tests whether all elements in the array pass the test implemented by the specified function.
     *
     * @param { Function } predicate A function used to test each element.
     *     The function receives three parameters:
     *     value (the value of the current element), index (the index of the current element),
     *     and obj (the current array object being traversed).
     * @returns { boolean } Returns true if the callback function returns true for every element;
     *     otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    every(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): boolean;

    /**
     * Creates a new array containing all elements that pass the test implemented by the provided function.
     *
     * @param { Function } predicate A function used to test each element of the array.
     *     Returns true to keep the element, false to discard it. The function receives three parameters:
     *     value (the value of the current element), index (the index of the current element),
     *     and obj (the current array object being traversed).
     * @returns { Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T> }
     *     A new array containing all elements that pass the test.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    filter(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T>;

    /**
     * Returns the value of the first element in the array that satisfies the provided testing function,
     * or undefined if no element satisfies it.
     *
     * @param { Function } predicate A function used to test each element of the array.
     *     The function receives three parameters:
     *     value (the value of the current element), index (the index of the current element),
     *     and obj (the current array object being traversed).
     * @returns { T | undefined } The value of the first element in the array that satisfies the testing function,
     *     or undefined if none.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    find(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): T | undefined;

    /**
     * Returns the value of the last element in the array that satisfies the provided testing function,
     * or undefined if no element satisfies it.
     *
     * @param { Function } predicate A function used to test each element of the array.
     *     The function receives three parameters:
     *     value (the value of the current element), index (the index of the current element),
     *     and obj (the current array object being traversed).
     * @returns { T | undefined } The value of the last element in the array that satisfies the testing function,
     *     or undefined if none.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    findLast(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): T | undefined;

    /**
     * Returns the index of the first element in the array that satisfies the provided testing function,
     * or -1 if no element satisfies it.
     *
     * @param { Function } predicate A function used to test each element of the array.
     *     The function receives three parameters:
     *     value (the value of the current element), index (the index of the current element),
     *     and obj (the current array object being traversed).
     * @returns { int } The index of the first element in the array that satisfies the testing function,
     *     or -1 if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    findIndex(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): int;

    /**
     * Returns the index of the last element in the array that satisfies the provided testing function,
     * or -1 if no element satisfies it.
     *
     * @param { Function } predicate A function used to test each element of the array.
     *     The function receives three parameters:
     *     value (the value of the current element), index (the index of the current element),
     *     and obj (the current array object being traversed).
     * @returns { int } The index of the last element in the array that satisfies the testing function,
     *     or -1 if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    findLastIndex(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): int;

    /**
     * Executes a provided function once for each element in the array.
     *
     * @param { Function } action A function to execute for each element in the array.
     *     The function receives three parameters:
     *     value (the value of the current element), index (the index of the current element),
     *     and array (the current array object being traversed).
     * @returns { void } No return value.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    forEach(action: (value: T, index: int, array: ReadonlyArray<T>) => void): void;

    /**
     * Joins all elements of an array (or an array-like object) into a string and returns this string.
     *
     * @param { string } separator Specifies a string to separate each element of the array.
     * @returns { string } A string formed by joining all array elements.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    join(separator?: string): string;

    /**
     * Determines whether an array contains a specified value, returning true if it contains it,
     * otherwise false.
     *
     * @param { @types.SkipVarianceCheckDontUseItWillBeRemoved T } searchElement The element to search for.
     * @param { int } fromIndex The index position to start searching.
     *     If this index is greater than or equal to the array length, returns -1.
     *     If negative, the offset is calculated from the end of the array.
     *     If -array.length <= fromIndex < 0, fromIndex + array.length is used.
     *     However, the array is still searched from front to back in this case.
     *     If fromIndex is less than -array.length or fromIndex is omitted, 0 is used,
     *     causing the entire array to be searched.
     * @returns { boolean } Returns true if the specified value is found, otherwise false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    includes(searchElement: @types.SkipVarianceCheckDontUseItWillBeRemoved T, fromIndex?: int): boolean;

    /**
     * Returns the first index at which a given element can be found in the array, or -1 if it does not exist.
     *
     * @param { @types.SkipVarianceCheckDontUseItWillBeRemoved T } searchElement The element to search for.
     * @param { int } fromIndex The index position to start searching.
     *     If this index is greater than or equal to the array length, returns -1.
     *     If negative, the offset is calculated from the end of the array.
     *     if -array.length <= fromIndex < 0, fromIndex + array.length is used.
     *     However, the array is still searched from front to back in this case.
     *     If fromIndex is less than -array.length or fromIndex is omitted, 0 is used,
     *     causing the entire array to be searched.
     * @returns { int } The index of the first found element in the array, or -1 if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    indexOf(searchElement: @types.SkipVarianceCheckDontUseItWillBeRemoved T, fromIndex?: int): int;

    /**
     * Returns an iterator that contains each index key in the array.
     *
     * @returns { IterableIterator<number> } A new array iterator object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    keys(): IterableIterator<number>;

    /**
     * Returns the last index of the specified element in the array, or -1 if it does not exist.
     * The array is searched backwards.
     *
     * @param { @types.SkipVarianceCheckDontUseItWillBeRemoved T } searchElement The element to search for.
     *     Start searching backward from the end of the array, the entire array is searched.
     * @returns { int } The index of the last specified element in the array, or -1 if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    lastIndexOf(searchElement: @types.SkipVarianceCheckDontUseItWillBeRemoved T): int;

    /**
     * Returns the last index of the specified element in the array, or -1 if it does not exist.
     * The array is searched backwards, starting at fromIndex.
     *
     * @param { @types.SkipVarianceCheckDontUseItWillBeRemoved T } searchElement The element to search for.
     * @param { int } fromIndex Position to start searching backward from.
     *     Defaults to the array length minus 1, i.e., the entire array is searched.
     *     If this value is greater than or equal to the array length, the entire array will be searched.
     *     If negative, it is treated as an offset from the end of the array.
     *     If fromIndex < -array.length, the array is not searched and -1 is returned.
     * @returns { int } The index of the last specified element in the array, or -1 if not found.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    lastIndexOf(searchElement: @types.SkipVarianceCheckDontUseItWillBeRemoved T, fromIndex?: int): int;

    /**
     * Creates a new array with the result of calling a provided function on every element in this array.
     *
     * @param { Function } mapper Function
     *     that produces an element of the new array. The function receives three parameters:
     *     value (the value of the current element), index (the index of the current element),
     *     array (the current array object being traversed).
     * @returns { Array<U> } A new array where each element is the result of the callback function.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    map<U>(mapper: (value: T, index: int, array: ReadonlyArray<T>) => U): Array<U>;

    /**
     * Executes a user-supplied reducer function on each element of the array (processed in ascending order),
     * and aggregates the results into a single return value.
     *
     * @param { Function } reducer A "reducer" function with four parameters:
     *     previousValue (the value resulting from the previous call to "reducer"),
     *     currentValue (the current element), currentIndex (the index of the current element),
     *     and array (the array itself).
     * @returns { T } The accumulated result of the function's processing.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    reduce(reducer: (previousValue: T, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => @types.SkipVarianceCheckDontUseItWillBeRemoved T): T;

    /**
     * Executes a user-supplied reducer function on each element of the array (in ascending order), 
     * aggregating the results into a single return value.
     *
     * @param { Function } reducer A "reducer" function with four parameters:
     *     previousValue (the value resulting from the previous call to "reducer"),
     *     currentValue (the current element), currentIndex (the index of the current element),
     *     and array (the array itself).
     * @param { U } initialValue The value to use as the first argument to the first call of the callback function.
     *     If no initial value is supplied, the first element of the array will be used.
     * @returns { U } The accumulated result of the reducer function processing.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    reduce<U>(reducer: (previousValue: U, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => U, initialValue: U): U;

    /**
     * Accepts a function as an accumulator and processes each value of the array (from right to left)
     * to reduce it to a single value.
     *
     * @param { Function } reducer A "reducer" function with four parameters:
     *     previousValue (the value resulting from the previous call to "reducer"),
     *     currentValue (the current element), currentIndex (the index of the current element),
     *     and array (the array itself).
     * @returns { T } The accumulated result of the function's processing.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    reduceRight(reducer: (previousValue: T, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => @types.SkipVarianceCheckDontUseItWillBeRemoved T): T;

    /**
     * Accepts a function as an accumulator and processes each value of the array (from right to left)
     * to reduce it to a single value.
     *
     * @param { (previousValue: U, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => U }
     *     reducer A "reducer" function with four parameters:
     *     previousValue (the value resulting from the previous call to "reducer"),
     *     currentValue (the current element), currentIndex (the index of the current element),
     *     and array (the array itself).
     * @param { U } initialValue The value to use as the first argument
     *     when the callback function is called for the first time.
     *     If no initial value is provided, the last element of the array will be used.
     * @returns { U } The accumulated result of the function's processing.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    reduceRight<U>(reducer: (previousValue: U, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => U, initialValue: U): U;

    /**
     * The method returns a new array object that is a shallow copy of the original array determined by begin and end
     * (including begin, excluding end). The original array will not be modified.
     *
     * @param { int } start The index at which to begin extraction (starting from 0).
     *     If this parameter is negative, it indicates extraction starting from the specified element
     *     from the end of the original array.
     * @param { int } end The index at which to end extraction
     *     (ending extraction of original array elements at this index).
     *     If this parameter is negative, it indicates ending extraction 
     *     at the specified element from the end of the original array.
     *     If omitted, slice will extract to the end of the original array.
     * @returns { Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T> }
     *     A new array containing the extracted elements.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    slice(start?: int, end?: int): Array<@types.SkipVarianceCheckDontUseItWillBeRemoved T>;

    /**
     * Tests whether at least one element in the array passes the test implemented by the provided function.
     * It returns a Boolean value.
     *
     * @param { Function }
     *     predicate A function used to test each element of the array.
     *     The function receives three parameters: value (the value of the current element),
     *     index (the index of the current element), and array (the current array object being traversed).
     * @returns { boolean } Returns true if the callback function returns true for at least one element;
     *     otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    some(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): boolean;

    /**
     * Returns a new array iterator object that contains the values for each index in the array.
     *
     * @returns { IterableIterator<T> } A new array iterator object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    values(): IterableIterator<T>;
}
