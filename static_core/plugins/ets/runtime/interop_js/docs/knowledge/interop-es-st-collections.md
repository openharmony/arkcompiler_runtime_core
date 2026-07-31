# Interop es/st.Array/Map/Set Collections Knowledge

This document records the specification rules for cross-context collection types — `es.Array/Map/Set` (Sta accessing Dyn collections) and `st.Array/Map/Set` (Dyn accessing Sta collections).

## es.Array/Map/Set (Sta Context, Dyn Collections)

Defined in static code as dynamic `Array/Map/Set` types — the types used when Sta accesses Dyn collections.

### Acquisition Methods

1. **Via Dyn Array class instantiation**:
   ```typescript
   let global = ESValue.getGlobal();
   let arrayClass = global.getProperty('Array');
   let esArray1 = arrayClass.instantiate().unwrap() as es.Array<string>;
   ```
2. **Via Dyn export**:
   ```typescript
   import { esArray2 } from 'arkts_dyn.ets';
   ```
3. **Via function returning Array**:
   ```typescript
   let esArray3: es.Array<number> = returnArray<number>(1, 2);
   ```

**Note**: When calling Dyn functions with Array/Map/Set parameters, must pass `es.Array/Map/Set` type arguments.

### es.Array\<T\> Interface

**Property & Element Access:**
- `length` (property): number
- `$_get(index: number)`: T | undefined
- `$_set(index: number, val: T)`: void
- `at(index: number)`: T | undefined

**Add/Remove:**
- `push(val: T)`: number
- `pop()`: T | undefined
- `unshift(val: T)`: number
- `shift()`: T | undefined
- `splice(start, deleteCount?, ...items)`: Array\<T\>
- `toSpliced(...)`: Array\<T\>

**Iteration/Mapping/Filtering:**
- `forEach(callbackfn, thisArg?)`: void
- `map\<U\>(callbackfn)`: Array\<U\>
- `filter(predicate)`: Array\<T\>
- `reduce(callbackfn, initialValue?)`: T | U
- `reduceRight(callbackfn, initialValue?)`: T | U
- `flatMap\<U\>(fn)`: Array\<U\>

**Search/Test:**
- `indexOf(val, fromIndex?)`: number
- `lastIndexOf(val, fromIndex?)`: number
- `includes(val, fromIndex?)`: boolean
- `find(predicate)`: T | undefined
- `findIndex(predicate)`: number
- `findLast(predicate)`: T | undefined
- `findLastIndex(predicate)`: number
- `some(predicate)`: boolean
- `every(predicate)`: boolean

**Transform/Sort/Merge:**
- `concat(val)`: Array\<T\>
- `slice(start?, end?)`: Array\<T\>
- `reverse()`: Array\<T\>
- `toReversed()`: Array\<T\>
- `sort(comparator?)`: Array\<T\>
- `toSorted(comparator?)`: Array\<T\>
- `fill(value, start?, end?)`: Array\<T\>
- `copyWithin(target, start?, end?)`: Array\<T\>
- `flat\<U\>(depth?)`: Array\<U\>
- `with(index, value)`: Array\<T\>
- `join(sep?)`: string
- `toString()`: string

### es.Map\<K,V\> Interface

- `size` (property): number
- `set(key: K, value: V)`: this
- `get(key: K)`: V | undefined
- `has(key: K)`: boolean
- `delete(key: K)`: boolean
- `clear()`: void
- `forEach(callbackfn, thisArg?)`: void
- `toString()`: string

### es.Set\<T\> Interface

- `size` (property): number
- `add(value: T)`: this
- `has(value: T)`: boolean
- `delete(value: T)`: boolean
- `clear()`: void
- `forEach(callbackfn, thisArg?)`: void
- `toString()`: string

## st.Array/Map/Set (Dyn Context, Sta Collections)

Defined in dynamic code as static `Array/Map/Set` types — the types used when Dyn accesses Sta collections.

### Acquisition Methods

1. **Via Sta export**: `import { sta_arr } from 'arkts_sta.ets'`
2. **Via function returning Array**: `let arr2 = returnArray()`
3. **Via STValue**: `STValue.newSTArray()` / `newSTMap()` / `newSTSet()`

**Note**: `st.Array/Map/Set` currently does not support `new` keyword creation. When calling Sta functions with Array/Map/Set parameters, must pass `st.Array/Map/Set` type arguments.

### st.Array\<T\> Interface

**Properties:**
- `length` (get/set): number
- `[index: number]`: T (index accessor)

**Add/Remove:**
- `push(...val: T[])`: number
- `pop()`: T | undefined
- `unshift(...val: T[])`: number
- `shift()`: T | undefined
- `splice(start)`: Array\<T\>
- `splice(start, deleteCount, ...val)`: Array\<T\>
- `toSpliced(start?, deleteCount?, ...items)`: Array\<T\>

**Iteration:**
- `forEach(callbackfn)`: void

**Search/Test:**
- `indexOf(val, fromIndex?)`: number
- `lastIndexOf(searchElement, fromIndex?)`: number
- `includes(val, fromIndex?)`: boolean
- `find(predicate)`: T | undefined
- `findIndex(predicate)`: number
- `findLast(predicate)`: T | undefined
- `findLastIndex(predicate)`: number
- `every(predicate)`: boolean
- `some(predicate)`: boolean

**Transform/Compute:**
- `map(callbackfn)`: Array\<T\>
- `reduce(callback, initVal?)`: T
- `reduceRight(callback, initVal?)`: T
- `filter(predicate)`: Array\<T\>

**Non-destructive ops:**
- `slice(start, end?)`: Array\<T\>
- `concat(...val)`: Array\<T\>
- `flat(depth?)`: Array\<T\>
- `toSpliced(...)`: Array\<T\>
- `toSorted(comparator?)`: Array\<T\>
- `toReversed()`: Array\<T\>
- `with(index, val)`: Array\<T\>

**Destructive ops:**
- `sort(comparator?)`: Array\<T\>
- `reverse()`: Array\<T\>
- `at(key)`: T (throws error on out-of-range)
- `copyWithin(target, start?, end?)`: Array\<T\>
- `fill(value, start?, end?)`: Array\<T\>
- `join(sep?)`: string
- `toString()`: string
- `toLocaleString()`: string

### st.Map\<K,V\> Interface

- `size` (property): number
- `set(key, value)`: this
- `get(key)`: V | undefined
- `get(key, def)`: V (with default)
- `has(key)`: boolean
- `delete(key)`: boolean
- `clear()`: void
- `forEach(callbackfn)`: void
- `toString()`: string

### st.Set\<T\> Interface

- `size` (property): number
- `add(value)`: this
- `has(value)`: boolean
- `delete(value)`: boolean
- `clear()`: void
- `forEach(callbackfn)`: void
- `toString()`: string
