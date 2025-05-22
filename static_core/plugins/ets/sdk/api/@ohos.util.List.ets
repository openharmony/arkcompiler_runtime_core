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

import { LinkedList, ListNode } from './@ohos.util.LinkedList';

export class List<T> implements Iterable<T> {
    private buffer: LinkedList<T>;

    /**
     * Constructs an empty list.
     */
    public constructor() {
        this.buffer = new LinkedList<T>();
    }

    /**
     * Gets the number of elements in the list.
     *
     * @returns The number of elements in the list.
     */
    public get length(): number {
        return this.buffer.length;
    }

    /**
     * Adds an element to the end of the list.
     *
     * @param element The element to add.
     * @returns True if the element was added successfully.
     */
    public add(element: T): boolean {
        return this.buffer.add(element);
    }

    /**
     * Clears all elements from the list.
     */
    public clear(): void {
        this.buffer.clear();
    }

    /**
     * Converts the list to an array.
     *
     * @returns An array containing all elements of the list.
     */
    public convertToArray(): Array<T> {
        return this.buffer.convertToArray();
    }

    /**
     * Checks if the list is equal to another object.
     *
     * @param obj The object to compare with.
     * @returns True if the list is equal to the object.
     */
    public equal(obj: Object): boolean {
        if (!(obj instanceof Iterable)) {
            return false;
        }

        let iterator = obj.$_iterator();
        let element = iterator.next();
        let currentNode: ListNode<T> | undefined = this.buffer.head
        while (!element.done && currentNode !== undefined) {
            if (currentNode!.element !== element.value!) {
                return false;
            }

            element = iterator.next();
            currentNode = currentNode!.next;
        }

        if (element.done && currentNode === undefined) {
            return true;
        }

        return false;
    }

    /**
     * Executes a provided function once for each list element.
     *
     * @param callbackfn - The function to execute on each element, taking the value, index, and List as arguments.
     *
     */
    public forEach(callbackfn: (value: T, index: number, list: List<T>) => void): void {
        this.buffer.forEach((value: T, index: number) => {
            callbackfn(value, index, this);
        })
    }

    /**
     * Gets the element at the specified index.
     *
     * @param index The index of the element to get.
     * @returns The element at the specified index, or undefined if the index is out of range.
     */
    public get(index: number): T | undefined {
        return this.buffer.get(index);
    }

    /**
     * Gets the first element of the list.
     *
     * @returns The first element of the list.
     * @throws RangeError if the list is empty.
     */
    public getFirst(): T | undefined {
        return this.buffer.getFirst();
    }

    /**
     * Gets the index of the first occurrence of an element.
     *
     * @param element The element to search for.
     * @returns The index of the first occurrence of the element, or -1 if not found.
     */
    public getIndexOf(element: T): number {
        return this.buffer.getIndexOf(element);
    }

    /**
     * Gets the last element of the list.
     *
     * @returns The last element of the list.
     * @throws Error if the list is empty.
     */
    public getLast(): T | undefined {
        return this.buffer.getLast();
    }

    /**
     * Gets the index of the last occurrence of an element.
     *
     * @param element The element to search for.
     * @returns The index of the last occurrence of the element, or -1 if not found.
     */
    public getLastIndexOf(element: T): number {
        return this.buffer.getLastIndexOf(element);
    }

    /**
     * Gets a sub list from the list.
     *
     * @param fromIndex The starting index of the sub list.
     * @param toIndex The ending index of the sub list.
     * @returns A new list containing the elements from the specified range.
     * @throws RangeError if the indices are out of range.
     */
    public getSubList(fromIndex: number, toIndex: number): List<T> {
        if (fromIndex > toIndex || fromIndex < 0 || toIndex > this.length - 1) {
            throw new RangeError('The value of fromIndex or toIndex is out of range.')
        }
        let subList: List<T> = new List<T>()
        let i: number = 0
        let currentNode: ListNode<T> | undefined = this.buffer.head
        while (currentNode !== undefined) {
            if (i >= fromIndex && i < toIndex) {
                subList.add(currentNode!.element)
            }
            currentNode = currentNode!.next
            i++
        }
        return subList;
    }

    /**
     * Checks if the list contains an element.
     *
     * @param element The element to check for.
     * @returns True if the list contains the element.
     */
    public has(element: T): boolean {
        return this.buffer.has(element);
    }

    /**
     * Inserts an element at the specified index.
     *
     * @param element The element to insert.
     * @param index The index at which to insert the element.
     * @throws RangeError if the index is out of range.
     */
    public insert(element: T, index: number): void {
        this.buffer.insert(index, element);
    }

    /**
     * Checks if the list is empty.
     *
     * @returns True if the list is empty.
     */
    public isEmpty(): boolean {
        return this.length == 0;
    }

    /**
     * Removes the first occurrence of an element from the list.
     *
     * @param element The element to remove.
     * @returns True if the element was removed successfully.
     */
    public remove(element: T): boolean {
        return this.buffer.remove(element);
    }

    /**
     * Removes the element at the specified index.
     *
     * @param index The index of the element to remove.
     * @returns The removed element, or undefined if the index is out of range.
     * @throws RangeError if the index is out of range.
     */
    public removeByIndex(index: number): T | undefined {
        return this.buffer.removeByIndex(index);
    }

    /**
     * Replaces all elements in the List using a callback function that takes the value and index, and itself.
     *
     * @param callbackfn - The callback function taking the value, index, and the List as arguments.
     *
     */
    public replaceAllElements(callbackfn: (value: T, index: number, list: List<T>) => T): void {
        let currentNode: ListNode<T> | undefined = this.buffer.head
        let index: number = 0;
        while (currentNode !== undefined) {
            currentNode!.element = callbackfn(currentNode!.element, index++, this)
            currentNode = currentNode!.next
        }
    }

    /**
     * Sets the element at the specified index.
     *
     * @param index The index of the element to set.
     * @param element The new element.
     * @returns The previous element at the specified index, or undefined if the index is out of range.
     * @throws RangeError if the index is out of range.
     */
    public set(index: number, element: T): T | undefined {
        return this.buffer.set(index, element);
    }

    /**
     * Sorts the elements of the list using a comparator function.
     *
     * @param comparator The function to compare elements.
     */
    public sort(comparator: (firstValue: T, secondValue: T) => number): void {
        if (this.length == 0) {
            return
        } else {
            this.quickSort(0, this.length - 1, comparator)
        }
    }

    /**
     * Sorts the list using the quicksort algorithm.
     *
     * @param list The list to sort.
     * @param low The starting index of the sort.
     * @param high The ending index of the sort.
     * @param comparator The function to compare elements.
     */
    private quickSort(low: number, high: number, comparator: (firstValue: T, secondValue: T) => number): void {
        if (low < high) {
            let pivot = this.partition(low, high, comparator);
            this.quickSort(low, pivot - 1, comparator);
            this.quickSort(pivot + 1, high, comparator);
        }
    }

    /**
     * Partitions the list for quicksort.
     *
     * @param list The list to partition.
     * @param low The starting index of the partition.
     * @param high The ending index of the partition.
     * @param comparator The function to compare elements.
     * @returns The index of the pivot element.
     */
    private partition(low: number, high: number, comparator: (firstValue: T, secondValue: T) => number): number {
        let compareNode: ListNode<T> = this.getNode(low)!
        let currentNode: ListNode<T> | undefined = compareNode.next!
        let pivotNode: ListNode<T> = compareNode
        let pivotNum: number = low
        let compareElement: T = compareNode.element;

        for (let j = low + 1; j <= high; j++) {
            if (comparator(currentNode!.element, compareElement) < 0) {
                pivotNode = pivotNode.next!
                this.swap(pivotNode, currentNode!)
                pivotNum++
            }
            currentNode = currentNode!.next
        }
        this.swap(compareNode, pivotNode)
        return pivotNum
    }

    /**
     * swaps the elements of two nodes.
     * @param list1 The first node.
     * @param list2 The second node.
     */
    private swap(list1: ListNode<T>, list2: ListNode<T>) {
        let temp = list1.element
        list1.element = list2.element
        list2.element = temp
    }

    /**
     * Returns an iterator over the elements in the list.
     *
     * @returns An iterator over the elements in the list.
     */
    public override $_iterator(): IterableIterator<T> {
        return this.buffer.$_iterator();
    }

    /**
     * Gets the node at the specified index.
     *
     * @param index The index of the node to get.
     * @returns The node at the specified index, or undefined if the index is out of range.
     */
    private getNode(index: number): ListNode<T> | undefined {
        if (index >= 0 && index < this.length) {
            let currentNode: ListNode<T> = this.buffer.head!
            for (let i = 0; i < index; i++) {
                currentNode = currentNode.next!
            }
            return currentNode;
        }
        return undefined;
    }

    /**
     * Gets the element at the specified index.
     *
     * @param index - The index of the element to retrieve.
     *
     * @returns The element at the specified index. If the index is out of range, returns undefined.
     */
    public $_get(index: number): T | undefined {
        return this.buffer.get(index);
    }

    /**
     * Sets the element at the specified index.
     *
     * @param index - The index of the element to set.
     *
     * @param val - The value to set at the specified index.
     */
    public $_set(index: number, val: T): void {
        this.buffer.set(index, val);
    }
}
