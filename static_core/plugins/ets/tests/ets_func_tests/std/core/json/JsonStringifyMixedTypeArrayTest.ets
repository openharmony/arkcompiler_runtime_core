/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

// Test correctness of JSON.stringify for arrays with mixed types.
// Scenarios: Array<Any> where first elements are primitives but later elements are objects/arrays/strings.

class Inner {
    name: string
    constructor(n: string) {
        this.name = n
    }
}

class Container {
    items: Array<Any>
    constructor(arr: Array<Any>) {
        this.items = arr
    }
}

// --- Bug 1: Mixed primitive + object in single array ---
// [Int, Int, Object] - probeNestedArrayType returns FVT_INT for first elem,
// writeLeafPrimitiveArray casts Object as int → incorrect

function testMixedIntAndObject(): void {
    const arr: Array<Any> = [1, 2, new Inner("hello")]
    const result = JSON.stringify(arr)
    // Expected: [1,2,{"name":"hello"}]
    arktest.assertTrue(result === '[1,2,{"name":"hello"}]', `Expected '[1,2,{"name":"hello"}]', got '${result}'`)
}

// --- Bug 1b: Mixed primitive + object via class field ---
// Same as Bug 1 but through a class field with CachedObjectLayoutEntry

function testMixedIntAndObjectViaField(): void {
    const c = new Container([1, 2, new Inner("world")])
    const result = JSON.stringify(c)
    // Expected: {"items":[1,2,{"name":"world"}]}
    arktest.assertTrue(result === '{"items":[1,2,{"name":"world"}]}', `Expected '{"items":[1,2,{"name":"world"}]}', got '${result}'`)
}

// --- Bug 2: cachedNestedInfo persists across instances ---
// Serialize obj with Int array first, then with Object array on same field.
// cachedNestedInfo from first serialization leaks into second.

function testCachedNestedInfoLeakIntToObject(): void {
    const c1 = new Container([10, 20, 30])
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"items":[10,20,30]}', `First: expected '{"items":[10,20,30]}', got '${r1}'`)

    // Same class, different instance, different array type
    const c2 = new Container([new Inner("a"), new Inner("b")])
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"items":[{"name":"a"},{"name":"b"}]}', `Second: expected '{"items":[{"name":"a"},{"name":"b"}]}', got '${r2}'`)
}

// --- Bug 3: Mixed Int + String in same array ---
// probeNestedArrayType returns FVT_INT for first elem,
// writeLeafPrimitiveArray with FVT_INT encounters String → incorrect

function testMixedIntAndString(): void {
    const arr: Array<Any> = [1, 2, "hello"]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1,2,"hello"]', `Expected '[1,2,"hello"]', got '${result}'`)
}

// --- Bug 4: Mixed Int + nested Array ---
// [1, 2, [3, 4]] - probeNestedArrayType finds Int first,
// but third element is Array

function testMixedIntAndArray(): void {
    const arr: Array<Any> = [1, 2, [3, 4]]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1,2,[3,4]]', `Expected '[1,2,[3,4]]', got '${result}'`)
}

// --- Bug 5: null first, then Int, then Object ---
// probeNestedArrayType skips null, finds Int, returns FVT_INT.
// Later Object element gets cast as int.

function testNullFirstMixedType(): void {
    const arr: Array<Any> = [null, 1, new Inner("x")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[null,1,{"name":"x"}]', `Expected '[null,1,{"name":"x"}]', got '${result}'`)
}

// --- Bug 6: Mixed Double + Object ---

function testMixedDoubleAndObject(): void {
    const arr: Array<Any> = [1.5, 2.5, new Inner("d")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1.5,2.5,{"name":"d"}]', `Expected '[1.5,2.5,{"name":"d"}]', got '${result}'`)
}

// --- Bug 7: Mixed Boolean + Object ---

function testMixedBooleanAndObject(): void {
    const arr: Array<Any> = [true, false, new Inner("b")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[true,false,{"name":"b"}]', `Expected '[true,false,{"name":"b"}]', got '${result}'`)
}

// --- Bug 8: Mixed Long + Object ---

function testMixedLongAndObject(): void {
    const arr: Array<Any> = [10000000000 as long, 20000000000 as long, new Inner("L")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[10000000000,20000000000,{"name":"L"}]', `Expected '[10000000000,20000000000,{"name":"L"}]', got '${result}'`)
}

// --- Bug 9: cachedType field value type leak ---
// Field `items: Any` cached as FVT_ARRAY from first call,
// then field value changes to String

class FlexibleContainer {
    value: Any
    constructor(v: Any) {
        this.value = v
    }
}

function testCachedTypeLeakArrayToString(): void {
    const c1 = new FlexibleContainer([1, 2, 3])
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"value":[1,2,3]}', `First: expected '{"value":[1,2,3]}', got '${r1}'`)

    const c2 = new FlexibleContainer("hello")
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"value":"hello"}', `Second: expected '{"value":"hello"}', got '${r2}'`)
}

// --- Bug 10: cachedType field value type leak Int to Object ---

function testCachedTypeLeakIntToObject(): void {
    const c1 = new FlexibleContainer(42)
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"value":42}', `First: expected '{"value":42}', got '${r1}'`)

    const c2 = new FlexibleContainer(new Inner("obj"))
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"value":{"name":"obj"}}', `Second: expected '{"value":{"name":"obj"}}', got '${r2}'`)
}

// --- Additional: Nested mixed array ---
// Array<Array<Any>> where outer array has mixed content: [[1,2], {name:"x"}]

function testNestedMixedArray(): void {
    const arr: Array<Any> = [[1, 2], new Inner("nested")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[[1,2],{"name":"nested"}]', `Expected '[[1,2],{"name":"nested"}]', got '${result}'`)
}

// --- Additional: Array field with empty array first, then mixed ---

function testEmptyThenMixedArray(): void {
    const c1 = new Container([])
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"items":[]}', `First: expected '{"items":[]}', got '${r1}'`)

    const c2 = new Container([1, new Inner("after-empty")])
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"items":[1,{"name":"after-empty"}]}', `Second: expected '{"items":[1,{"name":"after-empty"}]}', got '${r2}'`)
}

// ============================================================
// Tests for issues identified in optimization commit analysis
// ============================================================

// --- Issue: cachedType oscillation across instances of same class ---
// Same field alternates between Int and String across instances.
// cachedType should be re-validated each time, not stuck on stale value.

function testCachedTypeOscillationIntString(): void {
    // Serialize alternating types in sequence
    const c1 = new FlexibleContainer(1)
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"value":1}', `c1: expected '{"value":1}', got '${r1}'`)

    const c2 = new FlexibleContainer("a")
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"value":"a"}', `c2: expected '{"value":"a"}', got '${r2}'`)

    const c3 = new FlexibleContainer(2)
    const r3 = JSON.stringify(c3)
    arktest.assertTrue(r3 === '{"value":2}', `c3: expected '{"value":2}', got '${r3}'`)

    const c4 = new FlexibleContainer("b")
    const r4 = JSON.stringify(c4)
    arktest.assertTrue(r4 === '{"value":"b"}', `c4: expected '{"value":"b"}', got '${r4}'`)
}

// --- Issue: cachedType oscillation Int → Array → Int ---

function testCachedTypeOscillationIntArrayInt(): void {
    const c1 = new FlexibleContainer(42)
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"value":42}', `c1: expected '{"value":42}', got '${r1}'`)

    const c2 = new FlexibleContainer([1, 2])
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"value":[1,2]}', `c2: expected '{"value":[1,2]}', got '${r2}'`)

    const c3 = new FlexibleContainer(99)
    const r3 = JSON.stringify(c3)
    arktest.assertTrue(r3 === '{"value":99}', `c3: expected '{"value":99}', got '${r3}'`)
}

// --- Issue: cachedType oscillation Double ↔ null ---

function testCachedTypeOscillationDoubleNull(): void {
    const c1 = new FlexibleContainer(3.14)
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"value":3.14}', `c1: expected '{"value":3.14}', got '${r1}'`)

    const c2 = new FlexibleContainer(null)
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"value":null}', `c2: expected '{"value":null}', got '${r2}'`)

    const c3 = new FlexibleContainer(2.71)
    const r3 = JSON.stringify(c3)
    arktest.assertTrue(r3 === '{"value":2.71}', `c3: expected '{"value":2.71}', got '${r3}'`)

    const c4 = new FlexibleContainer(null)
    const r4 = JSON.stringify(c4)
    arktest.assertTrue(r4 === '{"value":null}', `c4: expected '{"value":null}', got '${r4}'`)
}

// --- Issue: cachedNestedInfo leaks across instances ---
// First instance has nested Array<Array<Int>>, second has flat Array<Inner>

class NestedContainer {
    data: Array<Any>
    constructor(arr: Array<Any>) {
        this.data = arr
    }
}

function testCachedNestedInfoLeakNestedToFlat(): void {
    const c1 = new NestedContainer([[1, 2], [3, 4]])
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"data":[[1,2],[3,4]]}', `c1: expected '{"data":[[1,2],[3,4]]}', got '${r1}'`)

    // Same class, different structure
    const c2 = new NestedContainer([new Inner("flat1"), new Inner("flat2")])
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"data":[{"name":"flat1"},{"name":"flat2"}]}', `c2: expected '{"data":[{"name":"flat1"},{"name":"flat2"}]}', got '${r2}'`)
}

// --- Issue: cachedNestedInfo from Int flat array leaks to nested Array<Array<Int>> ---

function testCachedNestedInfoLeakFlatToNested(): void {
    const c1 = new NestedContainer([10, 20, 30])
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"data":[10,20,30]}', `c1: expected '{"data":[10,20,30]}', got '${r1}'`)

    const c2 = new NestedContainer([[100, 200], [300, 400]])
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"data":[[100,200],[300,400]]}', `c2: expected '{"data":[[100,200],[300,400]]}', got '${r2}'`)
}

// --- Issue: cachedArrayElementType leaks across instances ---
// First array is all Int, second array on same field is all String

function testCachedArrayElementTypeLeakIntToString(): void {
    const c1 = new Container([1, 2, 3])
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"items":[1,2,3]}', `c1: expected '{"items":[1,2,3]}', got '${r1}'`)

    const c2 = new Container(["x", "y", "z"])
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"items":["x","y","z"]}', `c2: expected '{"items":["x","y","z"]}', got '${r2}'`)
}

// --- Issue: cachedArrayElementType leaks from Boolean to mixed ---

function testCachedArrayElementTypeLeakBooleanToMixed(): void {
    const c1 = new Container([true, false, true])
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"items":[true,false,true]}', `c1: expected '{"items":[true,false,true]}', got '${r1}'`)

    const c2 = new Container([1, "hello", true])
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"items":[1,"hello",true]}', `c2: expected '{"items":[1,"hello",true]}', got '${r2}'`)
}

// --- Issue: cyclic reference detection in fast path ---
// writeClassInline uses serialDepth instead of Set, which may miss true cycles.

class SelfRef {
    name: string
    ref: Any
    constructor(n: string) {
        this.name = n
        this.ref = null
    }
}

function testCyclicReferenceDirectSelfRef(): void {
    const obj = new SelfRef("cycle")
    obj.ref = obj  // direct self-reference
    let caught = false
    try {
        JSON.stringify(obj)
    } catch (e) {
        if (e instanceof TypeError) {
            caught = true
        }
    }
    arktest.assertTrue(caught, "Direct self-reference should throw TypeError")
}

function testCyclicReferenceMutualRef(): void {
    const a = new SelfRef("a")
    const b = new SelfRef("b")
    a.ref = b
    b.ref = a
    let caught = false
    try {
        JSON.stringify(a)
    } catch (e) {
        if (e instanceof TypeError) {
            caught = true
        }
    }
    arktest.assertTrue(caught, "Mutual reference should throw TypeError")
}

// --- Issue: cyclic array reference in fast path ---

class ArrayHolder {
    arr: Array<Any>
    constructor() {
        this.arr = []
    }
}

function testCyclicArrayReference(): void {
    const holder = new ArrayHolder()
    holder.arr.push(holder)  // array contains its parent object
    let caught = false
    try {
        JSON.stringify(holder)
    } catch (e) {
        if (e instanceof TypeError) {
            caught = true
        }
    }
    arktest.assertTrue(caught, "Cyclic array reference should throw TypeError")
}

// --- Issue: cyclic reference via Array field in Container ---

function testCyclicArraySelfReference(): void {
    const arr: Array<Any> = [1, 2]
    arr.push(arr)  // array contains itself
    let caught = false
    try {
        JSON.stringify(arr)
    } catch (e) {
        if (e instanceof TypeError) {
            caught = true
        }
    }
    arktest.assertTrue(caught, "Array self-reference should throw TypeError")
}

// --- Issue: nested array with mixed types at leaf level ---
// [[1, 2], [3, "four"]] - first inner array is all Int, second has String

function testNestedArrayMixedLeafTypes(): void {
    const arr: Array<Any> = [[1, 2], [3, "four"]]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[[1,2],[3,"four"]]', `Expected '[[1,2],[3,"four"]]', got '${result}'`)
}

// --- Issue: nested array with mixed types at non-leaf level ---
// [[1, 2], new Inner("mid")] - first element is Array, second is Object

function testNestedArrayMixedNonLeafTypes(): void {
    const arr: Array<Any> = [[10, 20], new Inner("mid")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[[10,20],{"name":"mid"}]', `Expected '[[10,20],{"name":"mid"}]', got '${result}'`)
}

// --- Issue: triple-nested array with type change at cachedNestedInfo ---
// First call: [[[1,2],[3,4]]] depth=2 leaf=Int
// Second call on same field: flat [10,20]

class DeepContainer {
    nested: Array<Any>
    constructor(arr: Array<Any>) {
        this.nested = arr
    }
}

function testCachedNestedInfoLeakTripleToFlat(): void {
    const c1 = new DeepContainer([[[1, 2], [3, 4]], [[5, 6], [7, 8]]])
    const r1 = JSON.stringify(c1)
    arktest.assertTrue(r1 === '{"nested":[[[1,2],[3,4]],[[5,6],[7,8]]]}', `c1: got '${r1}'`)

    const c2 = new DeepContainer([100, 200])
    const r2 = JSON.stringify(c2)
    arktest.assertTrue(r2 === '{"nested":[100,200]}', `c2: expected '{"nested":[100,200]}', got '${r2}'`)
}

// --- Issue: Double NaN and Infinity handling ---

function testDoubleSpecialValues(): void {
    const arr: Array<Any> = [1.0, Double.NaN, 2.0, Double.POSITIVE_INFINITY, 3.0, Double.NEGATIVE_INFINITY]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1,null,2,null,3,null]', `Expected '[1,null,2,null,3,null]', got '${result}'`)
}

// --- Issue: Float special values ---

function testFloatSpecialValues(): void {
    const arr: Array<Any> = [1.0 as float, new Float(Float.NaN), 2.0 as float]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1,null,2]', `Expected '[1,null,2]', got '${result}'`)
}

// --- Issue: Mixed Byte + Object via cached field ---

function testMixedByteAndObject(): void {
    const arr: Array<Any> = [1 as byte, 2 as byte, new Inner("b")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1,2,{"name":"b"}]', `Expected '[1,2,{"name":"b"}]', got '${result}'`)
}

// --- Issue: Mixed Short + Object via cached field ---

function testMixedShortAndObject(): void {
    const arr: Array<Any> = [100 as short, 200 as short, new Inner("s")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[100,200,{"name":"s"}]', `Expected '[100,200,{"name":"s"}]', got '${result}'`)
}

// --- Issue: Mixed Float + Object ---

function testMixedFloatAndObject(): void {
    const arr: Array<Any> = [1.5 as float, 2.5 as float, new Inner("f")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1.5,2.5,{"name":"f"}]', `Expected '[1.5,2.5,{"name":"f"}]', got '${result}'`)
}

// --- Issue: array.join output format consistency ---
// Verify that join-based fast path produces same output as element-by-element path

function testJoinFastPathConsistency(): void {
    // Large uniform Int array that triggers join fast path
    const arr: Array<Any> = []
    for (let i = 0; i < 100; i++) {
        arr.push(i)
    }
    const result = JSON.stringify(arr)

    // Build expected result manually
    let expected = '['
    for (let i = 0; i < 100; i++) {
        if (i > 0) expected = expected.concat(',')
        expected = expected.concat(i.toString())
    }
    expected = expected.concat(']')

    arktest.assertTrue(result === expected, `Large Int array: result length ${result.length}, expected length ${expected.length}`)
}

// --- Issue: array.join output format for Long ---

function testJoinFastPathLongConsistency(): void {
    const arr: Array<Any> = []
    for (let i = 0; i < 50; i++) {
        arr.push(new Long(i) * 10000000000 as long)
    }
    const result = JSON.stringify(arr)
    arktest.assertTrue(result.length > 0, "Long array join should produce output")
    // Spot check first and last elements
    arktest.assertTrue(result.startsWith('[0,'), `Expected to start with '[0,', got '${result.substring(0, 10)}'`)
    arktest.assertTrue(result.endsWith('490000000000]'), `Expected to end with '490000000000]', got '${result.substring(result.length - 20)}'`)
}

// --- Issue: mixed array via writeStdCoreArrayFast (no cachedEntry) ---
// When Array<Any> is the top-level argument to JSON.stringify,
// writeStdCoreArrayFast is used which has no cache entry.

function testTopLevelMixedArray(): void {
    const arr: Array<Any> = [1, "hello", true, null, new Inner("obj")]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1,"hello",true,null,{"name":"obj"}]',
        `Expected '[1,"hello",true,null,{"name":"obj"}]', got '${result}'`)
}

// --- Issue: top-level array with Double special values ---

function testTopLevelDoubleSpecialArray(): void {
    const arr: Array<Any> = [1.5, Double.NaN, Double.POSITIVE_INFINITY, 2.5]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1.5,null,null,2.5]',
        `Expected '[1.5,null,null,2.5]', got '${result}'`)
}

// --- Issue: object array with uniform type in writeStdCoreArrayInline FVT_COMPLEX path ---
// Array<Inner> where all elements are the same class

function testUniformObjectArrayInline(): void {
    const c = new Container([new Inner("a"), new Inner("b"), new Inner("c")])
    const result = JSON.stringify(c)
    arktest.assertTrue(result === '{"items":[{"name":"a"},{"name":"b"},{"name":"c"}]}',
        `Expected '{"items":[{"name":"a"},{"name":"b"},{"name":"c"}]}', got '${result}'`)
}

// --- Issue: interleaved type changes on same class field across multiple serializations ---

function testRepeatedFieldTypeChanges(): void {
    for (let round = 0; round < 5; round++) {
        const cInt = new FlexibleContainer(round)
        const rInt = JSON.stringify(cInt)
        arktest.assertTrue(rInt === '{"value":' + round.toString() + '}',
            `Round ${round} Int: expected '{"value":${round}}', got '${rInt}'`)

        const cStr = new FlexibleContainer("s" + round.toString())
        const rStr = JSON.stringify(cStr)
        arktest.assertTrue(rStr === '{"value":"s' + round.toString() + '"}',
            `Round ${round} String: expected '{"value":"s${round}"}', got '${rStr}'`)
    }
}

// --- Issue: FixedArray<Any> with mixed types ---
// writeFixedArrayObjectFast fast path must validate each element type,
// not blindly cast based on first element's type.

function testFixedArrayMixedTypes(): void {
    const fa = new FixedArray<Any>(4)
    fa[0] = 1
    fa[1] = "hello"
    fa[2] = true
    fa[3] = new Inner("fa")
    const result = JSON.stringify(fa)
    arktest.assertTrue(result === '[1,"hello",true,{"name":"fa"}]',
        `Expected '[1,"hello",true,{"name":"fa"}]', got '${result}'`)
}

// --- Issue: undefined in array should be null ---

function testUndefinedInArray(): void {
    const arr: Array<Any> = [1, undefined, 3]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1,null,3]',
        `Expected '[1,null,3]', got '${result}'`)
}

// --- Issue: Function in array should be null ---

function testFunctionInArray(): void {
    const arr: Array<Any> = [1, (() => { return 0 }), 3]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1,null,3]',
        `Expected '[1,null,3]', got '${result}'`)
}

// --- Issue: all-null array ---

function testAllNullArray(): void {
    const arr: Array<Any> = [null, null, null]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[null,null,null]',
        `Expected '[null,null,null]', got '${result}'`)
}

// --- Issue: empty objects inside mixed array ---

class EmptyClass {
}

function testEmptyObjectInArray(): void {
    const arr: Array<Any> = [1, new EmptyClass(), 3]
    const result = JSON.stringify(arr)
    arktest.assertTrue(result === '[1,{},3]',
        `Expected '[1,{},3]', got '${result}'`)
}

function main(): int {
    const suite = new arktest.ArkTestsuite("JSON.stringify mixed type array tests")

    // Original tests
    suite.addTest("Mixed Int and Object in array", testMixedIntAndObject)
    suite.addTest("Mixed Int and Object via field", testMixedIntAndObjectViaField)
    suite.addTest("Cached nested info leak Int to Object", testCachedNestedInfoLeakIntToObject)
    suite.addTest("Mixed Int and String", testMixedIntAndString)
    suite.addTest("Mixed Int and Array", testMixedIntAndArray)
    suite.addTest("Null first then mixed", testNullFirstMixedType)
    suite.addTest("Mixed Double and Object", testMixedDoubleAndObject)
    suite.addTest("Mixed Boolean and Object", testMixedBooleanAndObject)
    suite.addTest("Mixed Long and Object", testMixedLongAndObject)
    suite.addTest("Cached type leak Array to String", testCachedTypeLeakArrayToString)
    suite.addTest("Cached type leak Int to Object", testCachedTypeLeakIntToObject)
    suite.addTest("Nested mixed array", testNestedMixedArray)
    suite.addTest("Empty then mixed array", testEmptyThenMixedArray)

    // Cache oscillation tests
    suite.addTest("Cached type oscillation Int/String", testCachedTypeOscillationIntString)
    suite.addTest("Cached type oscillation Int/Array/Int", testCachedTypeOscillationIntArrayInt)
    suite.addTest("Cached type oscillation Double/null", testCachedTypeOscillationDoubleNull)
    suite.addTest("Repeated field type changes", testRepeatedFieldTypeChanges)

    // Cache leak across instances tests
    suite.addTest("Cached nested info leak nested to flat", testCachedNestedInfoLeakNestedToFlat)
    suite.addTest("Cached nested info leak flat to nested", testCachedNestedInfoLeakFlatToNested)
    suite.addTest("Cached array elem type leak Int to String", testCachedArrayElementTypeLeakIntToString)
    suite.addTest("Cached array elem type leak Boolean to mixed", testCachedArrayElementTypeLeakBooleanToMixed)
    suite.addTest("Cached nested info leak triple to flat", testCachedNestedInfoLeakTripleToFlat)

    // Cyclic reference detection tests
    suite.addTest("Cyclic reference direct self-ref", testCyclicReferenceDirectSelfRef)
    suite.addTest("Cyclic reference mutual ref", testCyclicReferenceMutualRef)
    suite.addTest("Cyclic array reference", testCyclicArrayReference)
    suite.addTest("Cyclic array self-reference", testCyclicArraySelfReference)

    // Nested array mixed type tests
    suite.addTest("Nested array mixed leaf types", testNestedArrayMixedLeafTypes)
    suite.addTest("Nested array mixed non-leaf types", testNestedArrayMixedNonLeafTypes)

    // Special value tests
    suite.addTest("Double special values NaN/Inf", testDoubleSpecialValues)
    suite.addTest("Float special values NaN", testFloatSpecialValues)

    // Additional mixed type tests for Byte/Short/Float
    suite.addTest("Mixed Byte and Object", testMixedByteAndObject)
    suite.addTest("Mixed Short and Object", testMixedShortAndObject)
    suite.addTest("Mixed Float and Object", testMixedFloatAndObject)

    // Join fast path consistency tests
    suite.addTest("Join fast path consistency Int", testJoinFastPathConsistency)
    suite.addTest("Join fast path consistency Long", testJoinFastPathLongConsistency)

    // Top-level mixed array tests
    suite.addTest("Top-level mixed array", testTopLevelMixedArray)
    suite.addTest("Top-level Double special array", testTopLevelDoubleSpecialArray)

    // Uniform object array test
    suite.addTest("Uniform object array inline", testUniformObjectArrayInline)

    // FixedArray mixed types test
    suite.addTest("FixedArray mixed types", testFixedArrayMixedTypes)

    // Edge case tests
    suite.addTest("Undefined in array", testUndefinedInArray)
    suite.addTest("Function in array", testFunctionInArray)
    suite.addTest("All-null array", testAllNullArray)
    suite.addTest("Empty object in array", testEmptyObjectInArray)

    return suite.run()
}
