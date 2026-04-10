# 1. es.Array/Map/Set

## Description

`es.Array/Map/Set` are dynamic `Array`/`Map`/`Set` defined in static code, i.e., the types used by static code to access dynamic `Array`/`Map`/`Set`.

Currently, dynamic `Array`/`Map`/`Set` can only be obtained by retrieving the dynamic `Array` class to initialize, exporting `Array`/`Map`/`Set` from dynamic code, or exporting return functions.

Examples are as follows:
```ts
// arkts_sta.ets
import {arr, returnArray} from 'arkts_dyn.ets';
import es from '@ohos.lang.interop';

function foo(){
    // Initialize by obtaining the Array class
    let global = ESValue.getGlobal();
    let arrayClass = global.getProperty(`Array`);
    let esArray1 = arrayClass.instantiate().unwrap() as es.Array<string>; // Create an empty es array with an initial size of 0
    
    // Initialize by obtaining an Array from dynamic internal exports
    let a = esArray2[1]; // a == 2
    esArray1.push(a);

    // Initialize by acquiring a dynamically exported function that returns an array
    let esArray3: es.Array<number> = returnArray<number>(1, 2);
    let b = esArray3[1]; // b == 2
    esArray1.push(b);

    let c = esArray1.length; // c == 2
}

// arkts_dyn.ets
export let arr: Array<number> = [1, 2];

export function returnArray<T>(...values: T[]): T[]{
    return [...values];
}

```
Meanwhile, if you need to call dynamic functions with `Array`/`Map`/`Set` as parameters, you must also pass arguments of type `es.Array/Map/Set`.


## 1.1 es.Array<T>

### 1.1 Properties and Element Access

| Method/Property | Parameters | Return Value | Description |
| :--- | :--- | :--- | :--- |
| **`length`** | (property) | `number` | Gets the length of the array. |
| **`$_get()`** | `index`: `number` | `T \| undefined` |  Gets the element at the specified index. |
| **`$_set()`** | `index`: `number`, `val`: `T` | `void` |  Sets the element at the specified index. |
| **`at()`** | `index`: `number` | `T \| undefined` | Accepts an integer value and returns the element at the corresponding index. Supports negative indices and returns `undefined` if the index is out of bounds. |

### 1.2 Adding and Removing Elements

| Method | Parameters | Return Value | Description |
| :--- | :--- | :--- | :--- |
| **`push()`** | `val`: `T` | `number` | Adds an element to the end of the array and returns the new length. |
| **`pop()`** | None | `T \| undefined` | Removes the last element from the array and returns its value. |
| **`unshift()`** | `val`: `T` | `number` | Adds an element to the beginning of the array and returns the new length. |
| **`shift()`** | None | `T \| undefined` | Removes the first element from the array and returns its value. |
| **`splice()`** | `start`: `number`, `deleteCount?`: `number`, `...items`: `StdArray<T>` | `Array<T>` | Modifies the array by deleting or replacing existing elements or adding new elements in place. |
| **`toSpliced()`** | Same as above | `Array<T>` | Similar to `splice()`, but does not modify the original array and returns a new array. |

### 1.3 Iteration, Mapping, and Filtering

| Method | Parameters | Return Value | Description |
| :--- | :--- | :--- | :--- |
| **`forEach()`** | `callbackfn`, `thisArg?`: `Object` | `void` | Executes a given function once for each element of the array. |
| **`map<U>()`** | `callbackfn` | `Array<U>` | Creates a new array with the results of calling a provided function on every element in the array. |
| **`filter()`** | `predicate` | `Array<T>` | Creates a new array with all elements that pass the test implemented by the provided function. |
| **`reduce()`** | `callbackfn`, `initialValue?`: `U` | `T \| U` | Executes a reducer function on each element of the array, resulting in a single output value. |
| **`reduceRight()`** | `callbackfn`, `initialValue?`: `U` | `T \| U` | Accepts a function as an accumulator and iterates the array from right to left, reducing it to a single value. |
| **`flatMap<U>()`** | `fn` | `Array<U>` | Applies a given callback to each element of the array, then flattens the result by one level and returns a new array. |

### 1.4 Searching and Testing

| Method | Parameters | Return Value | Description |
| :--- | :--- | :--- | :--- |
| **`indexOf()`** | `val`: `T`, `fromIndex?`: `number` | `number` | Returns the first index at which a given element can be found in the array, or -1 if it is not present. |
| **`lastIndexOf()`** | `val`: `T`, `fromIndex?`: `number` | `number` | Returns the last index at which a given element can be found in the array. |
| **`includes()`** | `val`: `T`, `fromIndex?`: `number` | `boolean` | Determines whether an array includes a certain value. |
| **`find()`** | `predicate` | `T \| undefined` | Returns the value of the first element in the array that satisfies the provided testing function. |
| **`findIndex()`** | `predicate` | `number` | Returns the index of the first element in the array that satisfies the provided testing function. |
| **`findLast()`** | `predicate` | `T \| undefined` | Iterates the array in reverse and returns the value of the first element that satisfies the testing function. |
| **`findLastIndex()`**| `predicate` | `number` | Iterates the array in reverse and returns the index of the first element that satisfies the testing function. |
| **`some()`** | `predicate` | `boolean` | Tests whether at least one element in the array passes the test implemented by the provided function. |
| **`every()`** | `predicate` | `boolean` | Tests whether all elements in the array pass the test implemented by the provided function. |

### 1.5 Transformation, Sorting, and Merging

| Method | Parameters | Return Value | Description |
| :--- | :--- | :--- | :--- |
| **`concat()`** | `val`: `T` | `Array<T>` | Merges the current array with the given value and returns a new array. |
| **`slice()`** | `start?`: `number`, `end?`: `number` | `Array<T>` | Returns a new array object that is a shallow copy of a portion of the original array determined by `start` and `end`. |
| **`reverse()`** | None | `Array<T>` | Reverses the elements in the array (in-place modification). |
| **`toReversed()`** | None | `Array<T>` | Non-destructive version of `reverse()`, returns a new reversed array. |
| **`sort()`** | `comparator?` | `Array<T>` | Sorts the elements of the array in place and returns the array. |
| **`toSorted()`** | `comparator?` | `Array<T>` | Non-destructive version of `sort()`, returns a new sorted array. |
| **`fill()`** | `value`: `T`, `start?`: `number`, `end?`: `number` | `Array<T>` | Fills all elements in an array from a start index to an end index with a static value. |
| **`copyWithin()`** | `target`, `start?`, `end?` | `Array<T>` | Shallow copies part of an array to another location in the same array. |
| **`flat<U>()`** | `depth?`: `number` | `Array<U>` | Creates a new array with all sub-array elements concatenated into it recursively up to the specified depth. |
| **`with()`** | `index`: `number`, `value`: `T` | `Array<T>` | Returns a new array with the element at the specified index replaced with the given value. |
| **`join()`** | `sep?`: `String` | `string` | Joins all elements of an array into a string. |
| **`toString()`** | None | `string` | Returns a string representing the specified array and its elements. |

---

# 2. es.Map<K, V>


### 2.1 Properties

| Property Name | Type | Description |
| :--- | :--- | :--- |
| **`size`** | `number` | **(Read-only)** Returns the total number of key-value pairs currently in the `Map` object. |

-----

### 2.2 Add, Delete, Modify, Query

The following methods are used to manage data entries in `Map` instances.

| Method Name | Parameters | `Return Value` | Description |
| :--- | :--- | :--- | :--- |
| **`set()`** | `key: K`<br>`value: V` | `this` | Adds or updates a specified key-value pair in the `Map`. **Returns the `Map` object itself, so it supports method chaining.** |
| **`get()`** | `key: K` | `V \| undefined` | Reads the value corresponding to the specified key. If the key does not exist in the `Map`, returns `undefined`. |
| **`has()`** | `key: K` | `boolean` | Determines whether the `Map` contains the specified key. Returns `true` if it exists, otherwise returns `false`. |
| **`delete()`** | `key: K` | `boolean` | Deletes the key-value pair corresponding to the specified key. If the element exists and is successfully removed, returns `true`; if the element does not exist, returns `false`. |
| **`clear()`** | None | `void` | Clears all key-value pairs in the `Map`. After calling, `size` will become 0. |

### 2.3 Iteration and Conversion

The following methods are used to iterate over data in the `Map` or convert it to other formats.

| Method Name | Parameters | Return Value | Description |
| :--- | :--- | :--- | :--- |
| **`forEach()`** | `callbackfn: (value: V, key: K, map: Map<K, V>) => void`<br>`thisArg?: Object` | `void` | Executes the provided callback function once for each key-value pair in the `Map` in insertion order. You can bind the `this` pointer inside the callback function through `thisArg`. |
| **`toString()`** | None | `string` | Returns the string representation of this `Map` object. |

-----

## 3. es.Set<T>

### 3.1 Properties

| Property Name | Type | Description |
| :--- | :--- | :--- |
| **`size`** | `number` | **(Read-only)** Returns the total number of elements currently in the `Set` object. |

-----

### 3.2 Add, Delete, Query

The following methods are used to manage elements in `Set` instances.

| Method Name | Parameters | Return Value | Description |
| :--- | :--- | :--- | :--- |
| **`add()`** | `value: T` | `this` | Adds a specified value to the end of the `Set` object. **Returns the `Set` object itself, supports method chaining.** |
| **`has()`** | `value: T` | `boolean` | Determines whether the `Set` contains the specified value. Returns `true` if it exists, otherwise returns `false`. |
| **`delete()`** | `value: T` | `boolean` | Removes the element equal to the specified value from the `Set`. If the element exists and is successfully removed, returns `true`; if the element does not exist, returns `false`. |
| **`clear()`** | None | `void` | Removes all elements in the `Set` object. After calling, `size` will become 0. |

### 3.3 Iteration and Conversion

The following methods are used to iterate over data in the `Set` or convert it to a string.

| Method Name | Parameters | Return Value | Description |
| :--- | :--- | :--- | :--- |
| **`forEach()`** | `callbackfn: (value: T, value2: T, set: Set<T>) => void`<br>`thisArg?: Object` | `void` | `Executes the provided callback function once for each element in the `Set` in insertion order. *Note: To maintain consistency with the `Map` callback function signature, both of the first two parameters (`value` and `value2`) will be passed the same current element value.* You can bind `this` through `thisArg`. |
| **`toString()`** | None | `string` | Returns the string representation of this `Set` object. |

-----

# 2. st.Array/Map/Set

## Description

`st.Array/Map/Set` are static `Array`/`Map`/`Set` defined in dynamic code, i.e., the types used by dynamic code to access static `Array`/`Map`/`Set`.

`st.Array/Map/Set` does not temporarily support creating new `Array`/`Map`/`Set` instances via the `new` keyword. Currently, static and dynamic `Array`/`Map`/`Set` can be obtained by exporting `Array`/`Map`/`Set` from static code, exporting return functions, etc., or new `Array`/`Map`/`Set` instances can be created via the `newSTArray`/`newSTMap`/`newSTSet` methods of `STValue`. In addition, for the import of `st` and `STValue`, relevant files need to be configured at present. For details, see: [STValue](./stvalue_en.md).

Examples are as follows:
```ts
// arkts_dyn.ets
import {sta_arr, returnArray} from 'arkts_sta.ets';
import st from 'static@ohos.lang.interop';
import STValue from 'static@ohos.lang.interop';

function foo(){
    let arr: st.Array<number> = STValue.newSTArray();
    arr.push(1, 2, 3);
    let a = arr[1]; // a == 2
    let b = sta_arr[0]; // b == 1
    let arr2 = returnArray(); 
    let c = arr2[1]; // c == 4
}

// arkts_sta.ets
export let sta_arr: Array<number> = [1, 2];

export function returnArray(): number[]{
    return [3, 4];
}

```
Meanwhile, if you need to call static functions with `Array`/`Map`/`Set` as parameters, you must also pass arguments of type `st.Array/Map/Set`.

## 1. st.Array<T>

### 1.1 Properties
| Property | Type | Description |
|----------|------|-------------|
| `length` (get) | `number` | Gets the length of the array (number of elements), returns a non-negative integer |
| `length` (set) | `(newLength: number) => void` | Sets the length of the array:<br>- If the new length is less than the original length, truncates the array (deletes elements beyond the new length)<br>- If the new length is greater than the original length, expands the array (fills new positions with `undefined`) |
| `[index: number]` | `T` | Array index accessor, reads/sets the element at the specified position through a numeric index (starting from 0) |

### 1.2 Element Addition and Deletion
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `shift` | `() => T \| undefined` | `T \| undefined` | Deletes the **first** element of the array and returns the deleted element; if the array is empty, returns `undefined` |
| `unshift` | `(...val: T[]) => number` | `number` | Adds one or more elements to the **beginning** of the array and returns the modified array length |
| `pop` | `() => T \| undefined` | `T \| undefined` | Deletes the **last** element of the array and returns the deleted element; if the array is empty, returns `undefined` |
| `push` | `(...val: T[]) => number` | `number` | Adds one or more elements to the **end** of the array and returns the modified array length |
| `splice` | Overload 1: `(start: number) => Array<T>`<br>Overload 2: `(start: number, deleteCount: number, ...val: T[]) => Array<T>` | `Array<T>` | Flexible array modification method:<br>- Overload 1: Deletes **all remaining elements** starting from the `start` index, returns the array of deleted elements<br>- Overload 2: Starting from the `start` index, deletes `deleteCount` elements and inserts elements from `val`, returns the array of deleted elements |

### 1.3 Iteration and Traversal
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `forEach` | `(callbackfn: (value: T, index: number, array: Array<T>) => void) => void` | `void` | Traverses each element of the array and executes a callback function:<br>- `value`: the current traversed element<br>- `index`: the index of the current element<br>- `array`: the original array itself<br> |

### 1.4 Array Filtering and Searching
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `filter` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => Array<T>` | `Array<T>` | Filters elements based on a predicate function, returns a **new array** (containing all elements where the predicate returns `true`), original array unchanged |
| `find` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => T \| undefined` | `T \| undefined` | Finds the first element that satisfies the predicate function, returns that element if found, otherwise returns `undefined` |
| `findIndex` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => number` | `number` | Finds the index of the first element that satisfies the predicate function, returns the index if found, otherwise returns `-1` |
| `findLast` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => T \| undefined` | `T \| undefined` | Searches from the end of the array in reverse for the first element that satisfies the predicate function, returns that element if found, otherwise returns `undefined` |
| `findLastIndex` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => number` | `number` | Searches from the end of the array in reverse for the index of the first element that satisfies the predicate function, returns the index if found, otherwise returns `-1`` |
| `includes` | `(val: T, fromIndex?: number) => boolean` | `boolean` | Determines whether the array contains the specified element `val`:<br>- `fromIndex`: optional, specifies the index to start searching from (default 0)<br>Returns `true` (contains) / `false` (does not contain) |
| `indexOf` | `(val: T, fromIndex?: number) => number` | ``number` | Finds the index of the first element equal to `val`, returns the index if found, otherwise returns `-1`; `fromIndex` specifies the index to start searching from |
| `lastIndexOf` | `(searchElement: T, fromIndex?: number) => number` | `number` | Searches from the end of the array in reverse for the index of the first element equal to `searchElement`, returns the index if found, otherwise returns `-1`; `fromIndex` specifies the starting position for reverse search |

### 1.5 Array Transformation and Computation
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `map` | `(callbackfn: (value: T, index: number, array: Array<T>) => T) => Array<T>` | `Array<T>` | Traverses the array and executes a callback function on each element, returns a **new array** composed of the callback results, original array unchanged |
| `reduce` | `(callback: (previousVal: T, curVal: T, idx: number, arr: Array<T>) => T, initVal?: T) => T` | `T` | Traverses the array from left to right, accumulating the final value through a callback function:<br>- `previousVal`: the return value of the previous callback (initial value is `initVal`, or the first element if not provided)<br>- `curVal`: the current element<br>Returns the final accumulated value |
| `reduceRight` | `(callback: (previousVal: T, curVal: T, idx: number, arr: Array<T>) => T, initVal?: T) => T` | `T` | Same logic as `reduce`, but traverses the array **from right to left** |
| `every` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => boolean` | `boolean` | Determines whether **all elements** of the array satisfy the predicate function:<br>- Returns `true` if all satisfy<br>- Returns `false` immediately if any element does not satisfy (immediately terminates traversal) |
| `some` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => boolean` | `boolean` | Determines whether **at least one element** of the array satisfies the predicate function:<br>- Returns `true` if one exists (immediately terminates traversal)<br>- Returns `false` if none exist |

### 1.6 Array Operations (Non-destructive)

| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `slice` | `(start: number, end?: number) => Array<T>` | `Array<T>` | Extracts a portion of the array and returns a **new array**:<br>- `start`: starting index (inclusive)<br>- `end`: optional, ending index (exclusive, defaults to end of array)<br>Supports negative indices (e.g., `-1` represents the last element) |
| `concat` | `(...val: Array<T>[]) => Array<T>` | `Array<T>` | Concatenates the current array with one or more arrays, returns a **new array**, original array unchanged |
| `flat` | `(depth?: number) => Array<T>` | `Array<T>` | Flattens the array:<br>- `depth`: optional, flattening depth (default 1)<br>Expands nested arrays into one-dimensional or specified depth arrays, returns a new array |
| `toSpliced` | `(start?: number, deleteCount?: number, ...items: T[]) => Array<T>` | `Array<T>` | Non-destructive version of `splice`: performs the same operation as `splice` but returns a new array, original array unchanged |
| `toSorted` | `(comparator?: (a: T, b: T) => number) => Array<T>` | `Array<T>` | Non-destructive version of `sort`: sorts the array and returns a new array, original array unchanged |
| `toReversed` | `() => Array<T>` | `Array<T>` | Non-destructive version of `reverse`: reverses the array and returns a new array, original array unchanged |
| `with` | `(index: number, val: T) => Array<T>` | `Array<T>` | Modifies the element at the specified index and returns a new array, original array unchanged (equivalent to the non-destructive version of `arr[index] = val`) |

### 1.7 Other Operations
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `sort` | `(comparator?: (a: T, b: T) => number) => Array<T>` | `Array<T>` | Sorts the array (**modifies the original array**):<br>- `comparator`: optional comparison function, return value rules:<br>  - Negative: `a` comes before `b`<br>  - 0: position unchanged<br>  - Positive: `b` comes before `a`<br>Without a comparison function, defaults to sorting by string Unicode code points |
| `reverse` | `() => Array<T>` | `Array<T>` | Reverses the order of array elements (**modifies the original array**), returns the modified original array |
| `at` | `(key: number) => T` | `T` | Accepts an integer and returns the element at the corresponding index. Supports negative indices, and throws an **"Illegal index"** error if the index is out of range. |
| `copyWithin` | `(target: number, start?: number, end?: number) => Array<T>` | `Array<T>` | Copies a portion of the array to another position in the same array (**modifies the original array**), returns the modified original array:<br>- `target`: target position index<br>- `start`: optional, copy start index (default 0)<br>- `end`: optional, copy end index (default array length) |
| `fill` | `(value: T, start?: number, end?: number) => Array<T>` | `Array<T>` | Fills the array with a specified value (**modifies the original array**), returns the modified original array:<br>- `value`: fill value<br>- `start`: optional, fill start index (default 0)<br>- `end`: optional, fill end index (default array length) |
| `join` | `(sep?: string) => string` | `string` | Joins array elements into a string:<br>- `sep`: optional separator (default comma `,`)<br>Empty array returns empty string, `undefined/null` elements are converted to empty strings |
| `toString` | `() => string` | `string` | Converts the array to a string, equivalent to `join(',')` |
| `toLocaleString` | `() => string` | `string` | Converts the array to a string according to local language environment rules, elements will call `toLocaleString()` |

---

## 2. st.Map<K, V> 

### 2.1 Properties
| Property | Type | Description |
|----------|------|-------------|
| `size` (get) | `number` | Gets the number of key-value pairs in the map, returns a non-negative integer (read-only property, no setter) |

### 2.2 Basic Operations (Add/Query/Delete/Clear)
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `set` | `(key: K, value: V) => this` | `this` | Adds/updates a key-value pair in the map:<br>- If `key` already exists, overwrites its corresponding value<br>- If `key` does not exist, adds the new key-value pair<br>Returns the map itself (`this`), supports method chaining (e.g., `map.set(k1, v1).set(k2, v2)`) |
| `get` | Overload 1: `(key: K) => V \| undefined`<br>Overload 2: `(key: K, def: V) => V` | Overload 1: `V \| undefined`<br>Overload 2: `V` | Gets the value corresponding to the specified key:<br>- Overload 1: If the key does not exist, returns `undefined`<br>- Overload 2: If the key does not exist, returns the passed default value `def` |
| `has` | `(key: K) => boolean` | `boolean` | Determines whether the map contains the specified key:<br>- Returns `true` if it exists, `false` if it does not |
| `delete` | `(key: K) => boolean` | `boolean` | Deletes the key-value pair corresponding to the specified key:<br>- Returns `true` if deletion succeeds (key exists)<br>- Returns `false` if deletion fails (key does not exist) |
| `clear` | `() => void` | `void` | Clears **all** key-value pairs in the map, `size` becomes 0 after execution |

### 2.3 Iteration and Traversal
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `forEach` | `(callbackfn: (value: V, key: K, map: Map<K, V>) => void) => void` | `void` | Traverses all key-value pairs in the map and executes a callback function:<br>- Callback parameter order: `value` (value) → `key`` (key) → `map` (original map)<br>- Traversal order is the insertion order of key-value pairs<br> |

### 2.4 Other Operations
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `toString` | `() => string` | `string` | Converts the map to a string representation |

## 3. st.Set<T>

### 3.1 Properties
| Property | Type | Description |
|----------|------|-------------|
| `size` (get) | `number` | Gets the number of elements in the set, returns a non-negative integer (read-only property, no setter) |

### 3.2 Basic Operations (Add/Query/Delete/Clear)
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `add` | `(value: T) => this` | `this` | Adds an element to the set:<br>- If the element already exists, the set remains unchanged<br>- If the element does not exist, adds the new element<br>Returns the set itself (`this`), supports method chaining (e.g., `set.add(1).add(2)`) |
| `has` | `(value: T) => boolean` | `boolean` | Determines whether the set contains the specified element:<br>- Returns `true` if it exists, `false` if it does not |
| `delete` | `(value: T) => boolean` | `boolean` | Deletes the specified element from the set:<br>- Returns `true` if deletion succeeds (element exists)<br>- Returns `false` if deletion fails (element does not exist) |
| `clear` | `() => void` | `void` | Clears **all** elements in the set, `size` becomes 0 after execution |

### 3.3 Iteration and Traversal
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `forEach` | `(callbackfn: (value: T, value2: T, set: Set<T>) => void) => void` | `void` | Traverses all elements in the set and executes a callback function:<br>- Callback parameter description:<br>  - `value`: the current traversed element<br>  - `value2`: same as `value` <br>  - `set`: the original set itself<br>- Traversal order is the insertion order of elements<br>  |

### 3.4 Other Operations
| Method | Signature | Return Value | Description |
|--------|-----------|--------------|-------------|
| `toString` | `() => string` | `string` | Converts the set to a string representation |