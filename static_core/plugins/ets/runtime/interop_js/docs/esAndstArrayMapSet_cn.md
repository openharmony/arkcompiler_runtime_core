# 一、es.Array/Map/Set

## 说明

`es.Array/Map/Set`是静态中定义的动态`Array`/`Map`/`Set`，即静态在访问动态的`Array`/`Map`/`Set`时所使用的类型。
`es.Array/Map/Set`暂时不支持通过new关键字来新建`Array`/`Map`/`Set`，目前只支持通过动态内导出`Array`/`Map`/`Set`或者导出返回函数等方式来获取动态的`Array`/`Map`/`Set`。
示例如下：
```ts
// arkts_sta.ets
import {arr, returnArray} from 'arkts_dyn.ets';
import es from '@ohos.lang.interop';

function foo(){
    let a = arr[1]; // a == 2
    let esArray: es.Array<number> = returnArray<number>(1, 2);
    let b = esArray[1]; // b == 2
}

// arkts_dyn.ets
export let arr: Array<number> = [1, 2];

export function returnArray<T>(...values: T[]): T[]{
    retrun [...values];
}

```
同时如果要调用入参为`Array`/`Map`/`Set`的动态函数等，也需要传入类型为`es.Array/Map/Set`。

## 1. es.Array<T>

### 1.1 属性与元素访问

| 方法/属性 | 参数 | 返回值 | 描述 |
| :--- | :--- | :--- | :--- |
| **`length`** | (属性) | `number` | 获取数组的长度。 |
| **`$_get()`** | `index`: `number` | `T \| undefined` |获取指定索引处的元素。 |
| **`$_set()`** | `index`: `number`, `val`: `T` | `void` |设置指定索引处的元素，不支持扩大。 |
| **`at()`** | `index`: `number` | `T \| undefined` | 接收一个整数值并返回该索引对应的元素。 |

### 1.2 添加与删除元素

| 方法 | 参数 | 返回值 | 描述 |
| :--- | :--- | :--- | :--- |
| **`push()`** | `val`: `T` | `number` | 将一个元素添加到数组的末尾，并返回新的长度。 |
| **`pop()`** | 无 | `T \| undefined` | 从数组中删除最后一个元素，并返回该元素的值。 |
| **`unshift()`** | `val`: `T` | `number` | 将一个元素添加到数组的开头，并返回新的长度。 |
| **`shift()`** | 无 | `T \| undefined` | 从数组中删除第一个元素，并返回该元素的值。 |
| **`splice()`** | `start`: `number`, `deleteCount?`: `number`, `...items`: `StdArray<T>` | `Array<T>` | 通过删除或替换现有元素或者原地添加新的元素来修改数组。 |
| **`toSpliced()`** | 同上 | `Array<T>` | 类似 `splice()`，但不修改原数组，而是返回一个新数组。 |

### 1.3 遍历、映射与过滤

| 方法 | 参数 | 返回值 | 描述 |
| :--- | :--- | :--- | :--- |
| **`forEach()`** | `callbackfn`, `thisArg?`: `Object` | `void` | 对数组的每个元素执行一次给定的函数。 |
| **`map<U>()`** | `callbackfn` | `Array<U>` | 创建一个新数组，其结果是该数组中的每个元素调用一次提供的函数后的返回值。 |
| **`filter()`** | `predicate` | `Array<T>` | 创建一个新数组，包含所有通过所提供函数测试的元素。 |
| **`reduce()`** | `callbackfn`, `initialValue?`: `U` | `T \| U` | 对数组中的每个元素按序执行一个 reducer 函数，将其结果汇总为单个返回值。 |
| **`reduceRight()`** | `callbackfn`, `initialValue?`: `U` | `T \| U` | 接受一个函数作为累加器，从右向左遍历数组，将其减少为单个值。 |
| **`flatMap<U>()`** | `fn` | `Array<U>` | 对数组中的每个元素应用给定回调，然后将结果展开一级，返回一个新数组。 |

### 1.4 查找与判断

| 方法 | 参数 | 返回值 | 描述 |
| :--- | :--- | :--- | :--- |
| **`indexOf()`** | `val`: `T`, `fromIndex?`: `number` | `number` | 返回在数组中可以找到一个给定元素的第一个索引，如果不存在，则返回 -1。 |
| **`lastIndexOf()`** | `val`: `T`, `fromIndex?`: `number` | `number` | 返回指定元素在数组中的最后一个的索引。 |
| **`includes()`** | `val`: `T`, `fromIndex?`: `number` | `boolean` | 判断一个数组是否包含一个指定的值。 |
| **`find()`** | `predicate` | `T \| undefined` | 返回数组中满足提供的测试函数的第一个元素的值。 |
| **`findIndex()`** | `predicate` | `number` | 返回数组中满足提供的测试函数的第一个元素的索引。 |
| **`findLast()`** | `predicate` | `T \| undefined` | 逆向迭代数组，并返回满足测试函数的第一个元素的值。 |
| **`findLastIndex()`**| `predicate` | `number` | 逆向迭代数组，并返回满足测试函数的第一个元素的索引。 |
| **`some()`** | `predicate` | `boolean` | 测试数组中是不是至少有 1 个元素通过了被提供的函数测试。 |
| **`every()`** | `predicate` | `boolean` | 测试一个数组内的所有元素是否都能通过某个指定函数的测试。 |

### 1.5 变形、排序与合并

| 方法 | 参数 | 返回值 | 描述 |
| :--- | :--- | :--- | :--- |
| **`concat()`** | `val`: `T` | `Array<T>` | 用于合并当前数组与给定值，返回一个新数组。 |
| **`slice()`** | `start?`: `number`, `end?`: `number` | `Array<T>` | 返回一个新的数组对象，这一对象是一个由 `start` 和 `end` 决定的原数组的浅拷贝。 |
| **`reverse()`** | 无 | `Array<T>` | 反转数组中的元素（原地修改）。 |
| **`toReversed()`** | 无 | `Array<T>` | `reverse()` 的非破坏性版本，返回一个反转后的新数组。 |
| **`sort()`** | `comparator?` | `Array<T>` | 用原地算法对数组的元素进行排序，并返回数组。 |
| **`toSorted()`** | `comparator?` | `Array<T>` | `sort()` 的非破坏性版本，返回一个排序后的新数组。 |
| **`fill()`** | `value`: `T`, `start?`: `number`, `end?`: `number` | `Array<T>` | 用一个固定值填充一个数组中从起始索引到终止索引内的全部元素。 |
| **`copyWithin()`** | `target`, `start?`, `end?` | `Array<T>` | 浅复制数组的一部分到同一数组中的另一个位置。 |
| **`flat<U>()`** | `depth?`: `number` | `Array<U>` | 按照一个可指定的深度递归遍历数组，并将所有元素与遍历到的子数组中的元素合并为一个新数组返回。 |
| **`with()`** | `index`: `number`, `value`: `T` | `Array<T>` | 返回一个新数组，其指定索引处的值被替换为给定值。 |
| **`join()`** | `sep?`: `String` | `string` | 将数组的所有元素连接成一个字符串。 |
| **`toString()`** | 无 | `string` | 返回一个字符串，表示指定的数组及其元素。 |

---

# 2. es.Map<K, V>


### 2.1 属性

| 属性名 | 类型 | 说明 |
| :--- | :--- | :--- |
| **`size`** | `number` | **(只读)** 返回 `Map` 对象中当前存在的键值对的总数。 |

-----

### 2.2 增删改查

以下方法用于管理 `Map` 实例中的数据条目。

| 方法名 | 入参 | 返回值 | 说明 |
| :--- | :--- | :--- | :--- |
| **`set()`** | `key: K`<br>`value: V` | `this` | 向 `Map` 中添加或更新一个指定的键值对。**返回 `Map` 对象自身，因此支持链式调用。** |
| **`get()`** | `key: K` | `V \| undefined` | 读取指定键对应的值。如果该键在 `Map` 中不存在，则返回 `undefined`。 |
| **`has()`** | `key: K` | `boolean` | 判断 `Map` 中是否包含指定的键。存在返回 `true`，否则返回 `false`。 |
| **`delete()`** | `key: K` | `boolean` | 删除指定键对应的键值对。如果该元素存在并被成功移除，返回 `true`；如果元素不存在，返回 `false`。 |
| **`clear()`** | 无 | `void` | 清空 `Map` 中的所有键值对。调用后 `size` 将变为 0。 |

### 2.3 遍历与转换

以下方法用于遍历 `Map` 中的数据或将其转换为其它格式。

| 方法名 | 入参 | 返回值 | 说明 |
| :--- | :--- | :--- | :--- |
| **`forEach()`** | `callbackfn: (value: V, key: K, map: Map<K, V>) => void`<br>`thisArg?: Object` | `void` | 按照插入顺序，为 `Map` 中的每个键值对执行一次提供的回调函数。可通过 `thisArg` 绑定回调函数内部的 `this` 指向。 |
| **`toString()`** | 无 | `string` | 返回该 `Map` 对象的字符串表示形式。 |

-----

## 3. es.Set<T>

### 3.1 属性

| 属性名 | 类型 | 说明 |
| :--- | :--- | :--- |
| **`size`** | `number` | **(只读)** 返回 `Set` 对象中当前存在的元素总数。 |

-----

### 3.2 增删查

以下方法用于管理 `Set` 实例中的元素。

| 方法名 | 入参 | 返回值 | 说明 |
| :--- | :--- | :--- | :--- |
| **`add()`** | `value: T` | `this` | 在 `Set` 对象的尾部添加一个指定的值。**返回 `Set` 对象自身，支持链式调用。** |
| **`has()`** | `value: T` | `boolean` | 判断 `Set` 中是否包含指定的值。存在返回 `true`，否则返回 `false`。 |
| **`delete()`** | `value: T` | `boolean` | 移除 `Set` 中与指定值相等的元素。如果该元素存在并被成功移除，返回 `true`；如果元素不存在，返回 `false`。 |
| **`clear()`** | 无 | `void` | 移除 `Set` 对象内的所有元素。调用后 `size` 将变为 0。 |

### 3.3 遍历与转换

以下方法用于遍历 `Set` 中的数据或将其转换为字符串。

| 方法名 | 入参 | 返回值 | 说明 |
| :--- | :--- | :--- | :--- |
| **`forEach()`** | `callbackfn: (value: T, value2: T, set: Set<T>) => void`<br>`thisArg?: Object` | `void` | 按照插入顺序，为 `Set` 中的每个元素执行一次提供的回调函数。\*注意：为了与标准 `Map` 保持回调函数签名的一致性，回调的前两个参数（`value` 和 `value2`）都会传入相同的当前元素值。\*可通过 `thisArg` 绑定 `this`。 |
| **`toString()`** | 无 | `string` | 返回该 `Set` 对象的字符串表示形式。 |

-----

# 二、st.Array/Map/Set

## 说明

`st.Array/Map/Set`是动态中定义的静态`Array`/`Map`/`Set`，即动态在访问静态的`Array`/`Map`/`Set`时所使用的类型。
`st.Array/Map/Set`暂时不支持通过new关键字来新建`Array`/`Map`/`Set`，目前支持通过静态内导出`Array`/`Map`/`Set`，导出返回函数等方式来获取动静态的`Array`/`Map`/`Set`或者通过`STValue`的`newSTArray`/`newSTMap`/`newSTSet`方法来新建`Array`/`Map`/`Set`。此外对于`st`和`STValue`的引入，目前需要配置相关文件等，详见：[STValue](./stvalue_cn.md#引用stvalue)。
示例如下：
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
    retrun [3, 4];
}

```
同时如果要调用入参为`Array`/`Map`/`Set`的静态函数等，也需要传入类型为`st.Array/Map/Set`。

## 1. st.Array<T>

### 1.1 属性
| 属性 | 类型 | 说明 |
|------|------|------|
| `length` (get) | `number` | 获取数组的长度（元素个数），返回非负整数 |
| `length` (set) | `(newLength: number) => void` | 设置数组的长度，不支持扩大。 |
| `[index: number]` | `T` | 数组索引访问器，通过数字索引（从 0 开始）读取/设置指定位置的元素 |

### 1.2 元素增删
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `shift` | `() => T \| undefined` | `T \| undefined` | 删除数组**第一个**元素，返回被删除的元素；若数组为空，返回 `undefined` |
| `unshift` | `(...val: T[]) => number` | `number` | 向数组**开头**添加一个/多个元素，返回修改后的数组长度 |
| `pop` | `() => T \| undefined` | `T \| undefined` | 删除数组**最后一个**元素，返回被删除的元素；若数组为空，返回 `undefined` |
| `push` | `(...val: T[]) => number` | `number` | 向数组**末尾**添加一个/多个元素，返回修改后的数组长度 |
| `splice` | 重载1：`(start: number) => Array<T>`<br>重载2：`(start: number, deleteCount: number, ...val: T[]) => Array<T>` | `Array<T>` | 灵活的数组修改方法：<br>- 重载1：从 `start` 索引开始删除**所有剩余元素**，返回被删除的元素数组<br>- 重载2：从 `start` 索引开始，删除 `deleteCount` 个元素，并插入 `val` 中的元素，返回被删除的元素数组 |

### 1.3 遍历与迭代
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `forEach` | `(callbackfn: (value: T, index: number, array: Array<T>) => void) => void` | `void` | 遍历数组每个元素并执行回调函数：<br>- `value`：当前遍历的元素<br>- `index`：当前元素的索引<br>- `array`：原数组本身<br> |

### 1.4 数组筛选与查找
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `filter` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => Array<T>` | `Array<T>` | 根据断言函数筛选元素，返回**新数组**（包含所有断言返回 `true` 的元素），原数组不变 |
| `find` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => T \| undefined` | `T \| undefined` | 查找第一个满足断言函数的元素，找到则返回该元素，否则返回 `undefined` |
| `findIndex` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => number` | `number` | 查找第一个满足断言函数的元素索引，找到则返回索引，否则返回 `-1` |
| `findLast` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => T \| undefined` | `T \| undefined` | 从数组末尾反向查找第一个满足断言函数的元素，找到则返回该元素，否则返回 `undefined` |
| `findLastIndex` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => number` | `number` | 从数组末尾反向查找第一个满足断言函数的元素索引，找到则返回索引，否则返回 `-1` |
| `includes` | `(val: T, fromIndex?: number) => boolean` | `boolean` | 判断数组是否包含指定元素 `val`：<br>- `fromIndex`：可选，指定开始查找的索引（默认 0）<br>返回 `true`（包含）/ `false`（不包含） |
| `indexOf` | `(val: T, fromIndex?: number) => number` | `number` | 查找第一个等于 `val` 的元素索引，找到则返回索引，否则返回 `-1`；`fromIndex` 指定开始查找的索引 |
| `lastIndexOf` | `(searchElement: T, fromIndex?: number) => number` | `number` | 从数组末尾反向查找第一个等于 `searchElement` 的元素索引，找到则返回索引，否则返回 `-1`；`fromIndex` 指定反向查找的起始位置 |

### 1.5 数组转换与计算
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `map` | `(callbackfn: (value: T, index: number, array: Array<T>) => T) => Array<T>` | `Array<T>` | 遍历数组并对每个元素执行回调函数，返回由回调结果组成的**新数组**，原数组不变 |
| `reduce` | `(callback: (previousVal: T, curVal: T, idx: number, arr: Array<T>) => T, initVal?: T) => T` | `T` | 从左到右遍历数组，通过回调函数累积计算最终值：<br>- `previousVal`：上一次回调的返回值（初始值为 `initVal`，无则取第一个元素）<br>- `curVal`：当前元素<br>返回最终累积值 |
| `reduceRight` | `(callback: (previousVal: T, curVal: T, idx: number, arr: Array<T>) => T, initVal?: T) => T` | `T` | 与 `reduce` 逻辑一致，但**从右到左**遍历数组 |
| `every` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => boolean` | `boolean` | 判断数组**所有元素**是否都满足断言函数：<br>- 全部满足返回 `true`<br>- 只要有一个不满足返回 `false`（立即终止遍历） |
| `some` | `(predicate: (value: T, index: number, array: Array<T>) => boolean) => boolean` | `boolean` | 判断数组**是否存在至少一个**元素满足断言函数：<br>- 存在返回 `true`（立即终止遍历）<br>- 不存在返回 `false` |

### 1.6 数组操作（非破坏性）

| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `slice` | `(start: number, end?: number) => Array<T>` | `Array<T>` | 截取数组的一部分，返回**新数组**：<br>- `start`：起始索引（包含）<br>- `end`：可选，结束索引（不包含，默认到数组末尾）<br>支持负索引（如 `-1` 表示最后一个元素） |
| `concat` | `(...val: Array<T>[]) => Array<T>` | `Array<T>` | 拼接当前数组与一个/多个数组，返回**新数组**，原数组不变 |
| `flat` | `(depth?: number) => Array<T>` | `Array<T>` | 扁平化数组：<br>- `depth`：可选，扁平化深度（默认 1）<br>将嵌套数组展开为一维/指定深度数组，返回新数组 |
| `toSpliced` | `(start?: number, deleteCount?: number, ...items: T[]) => Array<T>` | `Array<T>` | `splice` 的非破坏性版本：执行与 `splice` 相同的操作，但返回新数组，原数组不变 |
| `toSorted` | `(comparator?: (a: T, b: T) => number) => Array<T>` | `Array<T>` | `sort` 的非破坏性版本：对数组排序并返回新数组，原数组不变 |
| `toReversed` | `() => Array<T>` | `Array<T>` | `reverse` 的非破坏性版本：反转数组并返回新数组，原数组不变 |
| `with` | `(index: number, val: T) => Array<T>` | `Array<T>` | 修改指定索引的元素并返回新数组，原数组不变（等效于 `arr[index] = val` 的非破坏性版本） |

### 1.7 其它操作
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `sort` | `(comparator?: (a: T, b: T) => number) => Array<T>` | `Array<T>` | 对数组进行排序（**修改原数组**）：<br>- `comparator`：可选比较函数，返回值规则：<br>  - 负数：`a` 排在 `b` 前面<br>  - 0：位置不变<br>  - 正数：`b` 排在 `a` 前面<br>无比较函数时，默认按字符串 Unicode 码点排序 |
| `reverse` | `() => Array<T>` | `Array<T>` | 反转数组元素顺序（**修改原数组**），返回修改后的原数组 |
| `at` | `(key: number) => T` | `T` | 通过索引获取元素，支持负索引（如 `at(-1)` 获取最后一个元素），超出索引范围返回 `undefined` |
| `copyWithin` | `(target: number, start?: number, end?: number) => Array<T>` | `Array<T>` | 复制数组的一部分到同一数组的另一个位置（**修改原数组**），返回修改后的原数组：<br>- `target`：目标位置索引<br>- `start`：可选，复制起始索引（默认 0）<br>- `end`：可选，复制结束索引（默认数组长度） |
| `fill` | `(value: T, start?: number, end?: number) => Array<T>` | `Array<T>` | 用指定值填充数组（**修改原数组**），返回修改后的原数组：<br>- `value`：填充值<br>- `start`：可选，填充起始索引（默认 0）<br>- `end`：可选，填充结束索引（默认数组长度） |
| `join` | `(sep?: string) => string` | `string` | 将数组元素拼接为字符串：<br>- `sep`：可选分隔符（默认逗号 `,`）<br>空数组返回空字符串，`undefined/null` 元素转为空字符串 |
| `toString` | `() => string` | `string` | 将数组转为字符串，等效于 `join(',')` |
| `toLocaleString` | `() => string` | `string` | 根据本地语言环境规则将数组转为字符串，元素会调用 `toLocaleString()` |

---

## 2. st.Map<K, V> 

### 2.1 属性
| 属性 | 类型 | 说明 |
|------|------|------|
| `size` (get) | `number` | 获取映射中键值对的数量，返回非负整数（只读属性，无 setter） |

### 2.2 基础操作（增/查/删/清空）
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `set` | `(key: K, value: V) => this` | `this` | 向映射中添加/更新键值对：<br>- 若 `key` 已存在，覆盖其对应的值<br>- 若 `key` 不存在，新增该键值对<br>返回映射本身（`this`），支持链式调用（如 `map.set(k1, v1).set(k2, v2)`） |
| `get` | 重载1：`(key: K) => V \| undefined`<br>重载2：`(key: K, def: V) => V` | 重载1：`V \| undefined`<br>重载2：`V` | 获取指定键对应的值：<br>- 重载1：若键不存在，返回 `undefined`<br>- 重载2：若键不存在，返回传入的默认值 `def` |
| `has` | `(key: K) => boolean` | `boolean` | 判断映射中是否包含指定键：<br>- 存在返回 `true`，不存在返回 `false` |
| `delete` | `(key: K) => boolean` | `boolean` | 删除指定键对应的键值对：<br>- 删除成功（键存在）返回 `true`<br>- 删除失败（键不存在）返回 `false` |
| `clear` | `() => void` | `void` | 清空映射中**所有**键值对，执行后 `size` 变为 0 |

### 2.3 遍历与迭代
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `forEach` | `(callbackfn: (value: V, key: K, map: Map<K, V>) => void) => void` | `void` | 遍历映射中所有键值对并执行回调函数：<br>- 回调参数顺序：`value`（值）→ `key`（键）→ `map`（原映射）<br>- 遍历顺序为键值对的插入顺序<br> |

### 2.4 其它操作
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `toString` | `() => string` | `string` | 将映射转换为字符串表示 |

## 3. st.Set<T>

### 3.1 属性
| 属性 | 类型 | 说明 |
|------|------|------|
| `size` (get) | `number` | 获取集合中元素的数量，返回非负整数（只读属性，无 setter） |
| `[index: number]` | `T \| undefined` | 索引访问器：通过数字索引（0 开始）读取集合中的元素，超出索引范围返回 `undefined`；<br> 集合本身是无序的，索引仅为遍历便利，不保证元素顺序固定 |

### 3.2 基础操作（增/查/删/清空）
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `add` | `(value: T) => this` | `this` | 向集合中添加元素：<br>- 若元素已存在，集合无变化<br>- 若元素不存在，新增该元素<br>返回集合本身（`this`），支持链式调用（如 `set.add(1).add(2)`） |
| `has` | `(value: T) => boolean` | `boolean` | 判断集合中是否包含指定元素：<br>- 存在返回 `true`，不存在返回 `false` |
| `delete` | `(value: T) => boolean` | `boolean` | 从集合中删除指定元素：<br>- 删除成功（元素存在）返回 `true`<br>- 删除失败（元素不存在）返回 `false` |
| `clear` | `() => void` | `void` | 清空集合中**所有**元素，执行后 `size` 变为 0 |

### 3.3 遍历与迭代
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `forEach` | `(callbackfn: (value: T, value2: T, set: Set<T>) => void) => void` | `void` | 遍历集合中所有元素并执行回调函数：<br>- 回调参数说明：<br>  - `value`：当前遍历的元素<br>  - `value2`：与 `value` 相同 <br>  - `set`：原集合本身<br>- 遍历顺序为元素的插入顺序<br>  |

### 3.4 其它操作
| 方法 | 签名 | 返回值 | 说明 |
|------|------|--------|------|
| `toString` | `() => string` | `string` | 将集合转换为字符串表示 |