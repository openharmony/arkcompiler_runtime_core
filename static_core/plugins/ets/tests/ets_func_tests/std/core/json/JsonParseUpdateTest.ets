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

// ---------------------------------------------------------------------------
// Test classes
// ---------------------------------------------------------------------------

class Config {
    host: string = "localhost"
    port: double = 8080
    debug: boolean = false
}

class Point {
    x: double = 0
    y: double = 0
}

class Named {
    name: string = "default"
    score: double = 0
}

class NoDefaultCtor {
    id: double
    label: string = ""

    constructor(id: double) {
        this.id = id
    }
}

class WithAnnotations {
    @JSONParseIgnore
    secret: string = "hidden"
    @JSONRename("alias")
    value: string = "original"
}

class Nested {
    meta: string = "meta"
    inner: Point = new Point()
}

class InnerNoDefaultCtor {
    value: string = ""

    constructor(value: string) {
        this.value = value
    }
}

class OuterWithInnerNoDefaultCtor {
    tag: string = ""
    inner: InnerNoDefaultCtor = new InnerNoDefaultCtor("initial")
    count: double = 0

    constructor(tag: string, inner: InnerNoDefaultCtor, count: double) {
        this.tag = tag
        this.inner = inner
        this.count = count
    }
}

// ---------------------------------------------------------------------------
// Test: full population — all fields present in JSON
// ---------------------------------------------------------------------------

function testParseUpdateFullPopulation(): void {
    const cfg = new Config()
    const result = JSON.parseUpdate<Config>('{"host":"example.com","port":443,"debug":true}', cfg)

    arktest.assertEQ(result.host, "example.com")
    arktest.assertEQ(result.port, 443)
    arktest.assertTrue(result.debug)
    // parseUpdate returns the same instance
    arktest.assertTrue(result === cfg)
}

// ---------------------------------------------------------------------------
// Test: merge semantics — absent JSON keys retain existing field values
// ---------------------------------------------------------------------------

function testParseUpdateMergeSemantics(): void {
    const cfg = new Config()
    cfg.port = 9090
    cfg.debug = true

    JSON.parseUpdate<Config>('{"host":"example.com"}', cfg)

    arktest.assertEQ(cfg.host, "example.com")
    arktest.assertEQ(cfg.port, 9090)   // unchanged
    arktest.assertTrue(cfg.debug)       // unchanged
}

// ---------------------------------------------------------------------------
// Test: class without a default constructor
// ---------------------------------------------------------------------------

function testParseUpdateNoDefaultCtor(): void {
    const obj = new NoDefaultCtor(42)
    JSON.parseUpdate<NoDefaultCtor>('{"label":"hello"}', obj)

    arktest.assertEQ(obj.id, 42)        // preserved from construction
    arktest.assertEQ(obj.label, "hello")
}

// ---------------------------------------------------------------------------
// Test: reviver overload
// ---------------------------------------------------------------------------

function testParseUpdateWithReviver(): void {
    const pt = new Point()

    const reviver = (key: string, val: Any): Any => {
        if (key == "x") {
            return (val as Double) * 10
        }
        return val
    }

    JSON.parseUpdate<Point>('{"x":3,"y":5}', reviver, pt)

    arktest.assertEQ(pt.x, 30)
    arktest.assertEQ(pt.y, 5)
}

// ---------------------------------------------------------------------------
// Test: reviver returning undefined suppresses the field (existing value kept)
// ---------------------------------------------------------------------------

function testParseUpdateReviverUndefined(): void {
    const n = new Named()
    n.name = "keep"
    n.score = 99

    const reviver = (key: string, val: Any): Any => {
        if (key == "name") {
            return undefined
        }
        return val
    }

    JSON.parseUpdate<Named>('{"name":"new","score":1}', reviver, n)

    arktest.assertEQ(n.score, 1)
    // name was suppressed by reviver; existing value on instance is preserved
    arktest.assertEQ(n.name, "keep")
}

// ---------------------------------------------------------------------------
// Test: JSONParseIgnore annotation — annotated field not overwritten
// ---------------------------------------------------------------------------

function testParseUpdateParseIgnore(): void {
    const w = new WithAnnotations()
    w.secret = "still-hidden"

    // @JSONParseIgnore on 'secret' → even if present in JSON it is skipped
    // @JSONRename("alias") on 'value' → key "alias" maps to field 'value'
    JSON.parseUpdate<WithAnnotations>('{"secret":"leaked","alias":"renamed"}', w)

    arktest.assertEQ(w.secret, "still-hidden")
    arktest.assertEQ(w.value, "renamed")
}

// ---------------------------------------------------------------------------
// Test: nested object — inner object populated recursively
// ---------------------------------------------------------------------------

function testParseUpdateNested(): void {
    const n = new Nested()
    JSON.parseUpdate<Nested>('{"meta":"updated","inner":{"x":7,"y":8}}', n)

    arktest.assertEQ(n.meta, "updated")
    arktest.assertEQ(n.inner.x, 7)
    arktest.assertEQ(n.inner.y, 8)
}

// ---------------------------------------------------------------------------
// Test: bigIntMode option
// ---------------------------------------------------------------------------

function testParseUpdateWithBigIntModeOption(): void {
    const pt = new Point()
    const opts = {bigIntMode: jsonx.BigIntMode.DEFAULT} as jsonx.ParseOptions

    JSON.parseUpdate<Point>('{"x":1,"y":2}', undefined, pt, opts)

    arktest.assertEQ(pt.x, 1)
    arktest.assertEQ(pt.y, 2)
}

// ---------------------------------------------------------------------------
// Test: nested object field without default constructor — updated in-place
// ---------------------------------------------------------------------------

function testParseUpdateNestedNoDefaultCtor(): void {
    const inner = new InnerNoDefaultCtor("original")
    const outer = new OuterWithInnerNoDefaultCtor("t", inner, 5)

    const json = JSON.stringify(outer)
    const updated = JSON.parseUpdate<OuterWithInnerNoDefaultCtor>(json, outer)

    arktest.assertEQ(updated.tag, "t")
    arktest.assertEQ(updated.count, 5)
    arktest.assertEQ(updated.inner.value, "original")
    arktest.assertTrue(updated === outer)
    arktest.assertTrue(updated.inner === inner)
}

// ---------------------------------------------------------------------------
// Test: nested no-default-ctor object — JSON changes inner field value
// ---------------------------------------------------------------------------

function testParseUpdateNestedNoDefaultCtorChanged(): void {
    const outer = new OuterWithInnerNoDefaultCtor("old", new InnerNoDefaultCtor("before"), 1)
    const savedInner = outer.inner

    JSON.parseUpdate<OuterWithInnerNoDefaultCtor>(
        '{"tag":"new","inner":{"value":"after"},"count":99}', outer
    )

    arktest.assertEQ(outer.tag, "new")
    arktest.assertEQ(outer.count, 99)
    arktest.assertEQ(outer.inner.value, "after")
    arktest.assertTrue(outer.inner === savedInner)
}

// ---------------------------------------------------------------------------
// Test: nested no-default-ctor object — merge semantics preserves absent fields
// ---------------------------------------------------------------------------

function testParseUpdateNestedNoDefaultCtorMerge(): void {
    const outer = new OuterWithInnerNoDefaultCtor("keep", new InnerNoDefaultCtor("keep-inner"), 42)

    JSON.parseUpdate<OuterWithInnerNoDefaultCtor>('{"tag":"changed"}', outer)

    arktest.assertEQ(outer.tag, "changed")
    arktest.assertEQ(outer.count, 42)
    arktest.assertEQ(outer.inner.value, "keep-inner")
}

// ---------------------------------------------------------------------------
// Test: reviver handles type mismatch — converts string to Map
// ---------------------------------------------------------------------------

class WithMap {
    label: string = ""
    data: Map<string, string> = new Map<string, string>()
}

function testParseUpdateReviverTypeMismatch(): void {
    const obj = new WithMap()
    obj.label = "old"

    const reviver = (key: string, value: Any): Any => {
        if (key == "data") {
            const m = new Map<string, string>()
            m.set("converted", "true")
            return m
        }
        return value
    }

    JSON.parseUpdate<WithMap>('{"label":"new","data":"raw-string"}', reviver, obj)

    arktest.assertEQ(obj.label, "new")
    arktest.assertTrue(obj.data instanceof Map)
    arktest.assertEQ(obj.data.get("converted"), "true")
}

// ---------------------------------------------------------------------------
// Test: reviver receives raw value on type mismatch
// ---------------------------------------------------------------------------

function testParseUpdateReviverReceivesRawValue(): void {
    const obj = new WithMap()

    let capturedValue: Any = undefined
    const reviver = (key: string, value: Any): Any => {
        if (key == "data") {
            capturedValue = value
            return obj.data
        }
        return value
    }

    JSON.parseUpdate<WithMap>('{"data":"MAP[1,2]"}', reviver, obj)

    arktest.assertTrue(capturedValue instanceof String)
    arktest.assertEQ(capturedValue as string, "MAP[1,2]")
}

// ---------------------------------------------------------------------------
// Test: reviver with JSONObject mismatch — re-throws original parse error
// ---------------------------------------------------------------------------

function testParseUpdateReviverJsonObjectMismatchThrows(): void {
    const cfg = new Config()
    const reviver = (key: string, value: Any): Any => { return value }

    let caught = false
    try {
        JSON.parseUpdate<Config>(
            '{"host":{"nested":"object"},"port":443,"debug":true}', reviver, cfg
        )
    } catch (e) {
        arktest.assertTrue(e instanceof JSONTypeError)
        caught = true
    }
    arktest.assertTrue(caught)
}

// ---------------------------------------------------------------------------
// Test: reviver with JSONArray mismatch — re-throws original parse error
// ---------------------------------------------------------------------------

function testParseUpdateReviverJsonArrayMismatchThrows(): void {
    const cfg = new Config()
    const reviver = (key: string, value: Any): Any => { return value }

    let caught = false
    try {
        JSON.parseUpdate<Config>(
            '{"host":["array","value"],"port":443,"debug":true}', reviver, cfg
        )
    } catch (e) {
        arktest.assertTrue(e instanceof JSONTypeError)
        caught = true
    }
    arktest.assertTrue(caught)
}

// ---------------------------------------------------------------------------
// Test: reviver with JSONObject mismatch on numeric field — throws JSONTypeError
// ---------------------------------------------------------------------------

function testParseUpdateReviverNumericFieldJsonObjectThrowsJSONTypeError(): void {
    const cfg = new Config()
    const reviver = (key: string, value: Any): Any => { return value }

    let caught = false
    try {
        JSON.parseUpdate<Config>(
            '{"port":{"nested":"object"}}', reviver, cfg
        )
    } catch (e) {
        arktest.assertTrue(e instanceof JSONTypeError)
        caught = true
    }
    arktest.assertTrue(caught)
}

// ---------------------------------------------------------------------------
// Test: object with array fields — throws error due to type erasure
// ---------------------------------------------------------------------------

class WithArrays {
    name: string = ""
    scores: double[] = []
    tags: string[] = []
}

function testParseUpdateArrayFieldsThrows(): void {
    const obj = new WithArrays()
    obj.name = "old"
    obj.scores = [1, 2, 3]
    obj.tags = ["a"]

    arktest.expectError(() => {
        JSON.parseUpdate<WithArrays>('{"name":"new","scores":[10,20],"tags":["x","y","z"]}', obj)
    }, new Error("std.core.Array is expected, but get std.core.JSONArray"))
}

// ---------------------------------------------------------------------------
// Test: object with array fields — merge preserves arrays when absent
// ---------------------------------------------------------------------------

function testParseUpdateArrayFieldsMerge(): void {
    const obj = new WithArrays()
    obj.name = "keep"
    obj.scores = [99, 100]
    obj.tags = ["preserved"]

    JSON.parseUpdate<WithArrays>('{"name":"updated"}', obj)

    arktest.assertEQ(obj.name, "updated")
    arktest.assertEQ(obj.scores.length, 2)
    arktest.assertEQ(obj.scores[0], 99)
    arktest.assertEQ(obj.scores[1], 100)
    arktest.assertEQ(obj.tags.length, 1)
    arktest.assertEQ(obj.tags[0], "preserved")
}

// ---------------------------------------------------------------------------
// Test: object with empty array field — throws error due to type erasure
// ---------------------------------------------------------------------------

function testParseUpdateEmptyArrayFieldThrows(): void {
    const obj = new WithArrays()

    arktest.expectError(() => {
        JSON.parseUpdate<WithArrays>('{"name":"test","scores":[5,6,7,8]}', obj)
    }, new Error("std.core.Array is expected, but get std.core.JSONArray"))
}

// ---------------------------------------------------------------------------
// Test: object with FixedArray-of-objects field
// ---------------------------------------------------------------------------

class WithFixedObjectArray {
    label: string = ""
    points: FixedArray<Point> = []
}

function testParseUpdateFixedArrayOfObjects(): void {
    const obj = new WithFixedObjectArray()
    obj.label = "old"

    JSON.parseUpdate<WithFixedObjectArray>(
        '{"label":"new","points":[{"x":1,"y":2},{"x":3,"y":4},{"x":5,"y":6}]}', obj
    )

    arktest.assertEQ(obj.label, "new")
    arktest.assertEQ(obj.points.length, 3)
    arktest.assertEQ(obj.points[0].x, 1)
    arktest.assertEQ(obj.points[0].y, 2)
    arktest.assertEQ(obj.points[1].x, 3)
    arktest.assertEQ(obj.points[1].y, 4)
    arktest.assertEQ(obj.points[2].x, 5)
    arktest.assertEQ(obj.points[2].y, 6)
}

// ---------------------------------------------------------------------------
// Test: FixedArray-of-objects field — roundtrip via stringify + parseUpdate
// ---------------------------------------------------------------------------

function testParseUpdateFixedArrayOfObjectsRoundtrip(): void {
    const obj = new WithFixedObjectArray()
    obj.label = "trip"
    const p1 = new Point()
    p1.x = 10
    p1.y = 20
    const p2 = new Point()
    p2.x = 30
    p2.y = 40
    obj.points = [p1, p2]

    const json = JSON.stringify(obj)
    const target = new WithFixedObjectArray()
    JSON.parseUpdate<WithFixedObjectArray>(json, target)

    arktest.assertEQ(target.label, "trip")
    arktest.assertEQ(target.points.length, 2)
    arktest.assertEQ(target.points[0].x, 10)
    arktest.assertEQ(target.points[0].y, 20)
    arktest.assertEQ(target.points[1].x, 30)
    arktest.assertEQ(target.points[1].y, 40)
}

// ---------------------------------------------------------------------------
// Test: FixedArray of primitive values — int
// ---------------------------------------------------------------------------

class WithFixedIntArray {
    label: string = ""
    values: FixedArray<int> = []
}

function testParseUpdateFixedArrayOfInts(): void {
    const obj = new WithFixedIntArray()
    obj.label = "old"

    JSON.parseUpdate<WithFixedIntArray>(
        '{"label":"new","values":[10,20,30,40]}', obj
    )

    arktest.assertEQ(obj.label, "new")
    arktest.assertEQ(obj.values.length, 4)
    arktest.assertEQ(obj.values[0], 10)
    arktest.assertEQ(obj.values[1], 20)
    arktest.assertEQ(obj.values[2], 30)
    arktest.assertEQ(obj.values[3], 40)
}

// ---------------------------------------------------------------------------
// Test: FixedArray<Int> (boxed alias) — behaves identically to FixedArray<int>
// ---------------------------------------------------------------------------

class WithFixedBoxedIntArray {
    label: string = ""
    values: FixedArray<Int> = []
}

function testParseUpdateFixedArrayOfBoxedInts(): void {
    const obj = new WithFixedBoxedIntArray()
    obj.label = "old"
    obj.values = [new Int(99)]

    JSON.parseUpdate<WithFixedBoxedIntArray>(
        '{"label":"new","values":[10,20,30]}', obj
    )

    arktest.assertEQ(obj.label, "new")
    arktest.assertEQ(obj.values.length, 3)
    arktest.assertEQ(obj.values[0], 10)
    arktest.assertEQ(obj.values[1], 20)
    arktest.assertEQ(obj.values[2], 30)
}

// ---------------------------------------------------------------------------
// Test: FixedArray<Object> with Int values — throws on parseUpdate
// ---------------------------------------------------------------------------

class WithFixedObjectIntArray {
    label: string = ""
    items: FixedArray<Object> = []
}

function testParseUpdateFixedArrayOfObjectIntsThrows(): void {
    const obj = new WithFixedObjectIntArray()
    obj.label = "old"
    obj.items = [new Int(1), new Int(2)]

    arktest.expectError(() => {
        JSON.parseUpdate<WithFixedObjectIntArray>(
            '{"label":"new","items":[10,20,30]}', obj
        )
    }, new Error("std.core.Object is expected, but get std.core.Double\n                 with bigIntMode=0"))
}

// ---------------------------------------------------------------------------
// Test: FixedArray of primitive values — double
// ---------------------------------------------------------------------------

class WithFixedDoubleArray {
    label: string = ""
    values: FixedArray<double> = []
}

function testParseUpdateFixedArrayOfDoubles(): void {
    const obj = new WithFixedDoubleArray()
    obj.label = "old"

    JSON.parseUpdate<WithFixedDoubleArray>(
        '{"label":"new","values":[1.5,2.7,3.14]}', obj
    )

    arktest.assertEQ(obj.label, "new")
    arktest.assertEQ(obj.values.length, 3)
    arktest.assertEQ(obj.values[0], 1.5)
    arktest.assertEQ(obj.values[1], 2.7)
    arktest.assertEQ(obj.values[2], 3.14)
}

// ---------------------------------------------------------------------------
// Test: FixedArray of primitive values — boolean
// ---------------------------------------------------------------------------

class WithFixedBooleanArray {
    name: string = ""
    flags: FixedArray<boolean> = []
}

function testParseUpdateFixedArrayOfBooleans(): void {
    const obj = new WithFixedBooleanArray()
    obj.name = "old"

    JSON.parseUpdate<WithFixedBooleanArray>(
        '{"name":"new","flags":[true,false,true,false]}', obj
    )

    arktest.assertEQ(obj.name, "new")
    arktest.assertEQ(obj.flags.length, 4)
    arktest.assertTrue(obj.flags[0])
    arktest.assertFalse(obj.flags[1])
    arktest.assertTrue(obj.flags[2])
    arktest.assertFalse(obj.flags[3])
}

// ---------------------------------------------------------------------------
// Test: FixedArray of primitive values — char
// ---------------------------------------------------------------------------

class WithFixedCharArray {
    label: string = ""
    chars: FixedArray<char> = []
}

function testParseUpdateFixedArrayOfChars(): void {
    const obj = new WithFixedCharArray()
    obj.label = "old"

    JSON.parseUpdate<WithFixedCharArray>(
        '{"label":"new","chars":["a","b","c"]}', obj
    )

    arktest.assertEQ(obj.label, "new")
    arktest.assertEQ(obj.chars.length, 3)
    arktest.assertEQ(obj.chars[0], c'a')
    arktest.assertEQ(obj.chars[1], c'b')
    arktest.assertEQ(obj.chars[2], c'c')
}

// ---------------------------------------------------------------------------
// Test: FixedArray of chars — roundtrip via stringify + parseUpdate
// ---------------------------------------------------------------------------

function testParseUpdateFixedCharArrayRoundtrip(): void {
    const obj = new WithFixedCharArray()
    obj.label = "trip"
    obj.chars = [c'x', c'y', c'z']

    const json = JSON.stringify(obj)
    const target = new WithFixedCharArray()
    JSON.parseUpdate<WithFixedCharArray>(json, target)

    arktest.assertEQ(target.label, "trip")
    arktest.assertEQ(target.chars.length, 3)
    arktest.assertEQ(target.chars[0], c'x')
    arktest.assertEQ(target.chars[1], c'y')
    arktest.assertEQ(target.chars[2], c'z')
}

// ---------------------------------------------------------------------------
// Test: FixedArray of primitive values — string
// ---------------------------------------------------------------------------

class WithFixedStringArray {
    tag: string = ""
    items: FixedArray<string> = []
}

function testParseUpdateFixedArrayOfStrings(): void {
    const obj = new WithFixedStringArray()
    obj.tag = "old"

    JSON.parseUpdate<WithFixedStringArray>(
        '{"tag":"new","items":["alpha","beta","gamma"]}', obj
    )

    arktest.assertEQ(obj.tag, "new")
    arktest.assertEQ(obj.items.length, 3)
    arktest.assertEQ(obj.items[0], "alpha")
    arktest.assertEQ(obj.items[1], "beta")
    arktest.assertEQ(obj.items[2], "gamma")
}

// ---------------------------------------------------------------------------
// Test: FixedArray of primitives — merge preserves arrays when absent
// ---------------------------------------------------------------------------

function testParseUpdateFixedPrimitiveArrayMerge(): void {
    const obj = new WithFixedIntArray()
    obj.label = "keep"
    obj.values = [100, 200]

    JSON.parseUpdate<WithFixedIntArray>('{"label":"updated"}', obj)

    arktest.assertEQ(obj.label, "updated")
    arktest.assertEQ(obj.values.length, 2)
    arktest.assertEQ(obj.values[0], 100)
    arktest.assertEQ(obj.values[1], 200)
}

// ---------------------------------------------------------------------------
// Test: FixedArray of primitives — roundtrip via stringify + parseUpdate
// ---------------------------------------------------------------------------

function testParseUpdateFixedPrimitiveArrayRoundtrip(): void {
    const obj = new WithFixedIntArray()
    obj.label = "roundtrip"
    obj.values = [7, 14, 21]

    const json = JSON.stringify(obj)
    const target = new WithFixedIntArray()
    JSON.parseUpdate<WithFixedIntArray>(json, target)

    arktest.assertEQ(target.label, "roundtrip")
    arktest.assertEQ(target.values.length, 3)
    arktest.assertEQ(target.values[0], 7)
    arktest.assertEQ(target.values[1], 14)
    arktest.assertEQ(target.values[2], 21)
}

// ---------------------------------------------------------------------------
// Test: FixedArray of primitives — empty array in JSON
// ---------------------------------------------------------------------------

function testParseUpdateFixedPrimitiveArrayEmpty(): void {
    const obj = new WithFixedIntArray()
    obj.label = "test"
    obj.values = [1, 2, 3]

    JSON.parseUpdate<WithFixedIntArray>(
        '{"label":"empty","values":[]}', obj
    )

    arktest.assertEQ(obj.label, "empty")
    arktest.assertEQ(obj.values.length, 0)
}

// ---------------------------------------------------------------------------
// Test: Array-of-objects — throws error due to type erasure
// ---------------------------------------------------------------------------

class WithObjectArray {
    label: string = ""
    points: Array<Point> = new Array<Point>()
}

function testParseUpdateArrayOfObjectsThrows(): void {
    const obj = new WithObjectArray()

    arktest.expectError(() => {
        JSON.parseUpdate<WithObjectArray>(
            '{"label":"new","points":[{"x":1,"y":2},{"x":3,"y":4}]}', obj
        )
    }, new Error("std.core.Array is expected, but get std.core.JSONArray"))
}

// ---------------------------------------------------------------------------
// Test: reviver rescues Array field type mismatch
// ---------------------------------------------------------------------------

class WithResizableArray {
    b: Array<string>

    constructor(a: int) {
        this.b = new Array<string>()
    }
}

function testParseUpdateReviverRescuesArrayField(): void {
    const obj = new WithResizableArray(132)
    obj.b.push("original")

    const reviver = (key: string, value: Any): Any => {
        if (key == "b") {
            const arr = new Array<string>()
            arr.push("123")
            return arr
        }
        return value
    }

    JSON.parseUpdate<WithResizableArray>('{"b":"daflfjkdaskg"}', reviver, obj)

    arktest.assertTrue(obj.b instanceof Array)
    arktest.assertEQ(obj.b.length, 1)
    arktest.assertEQ(obj.b[0], "123")
}

// ---------------------------------------------------------------------------
// Test classes for complex nested structures
// ---------------------------------------------------------------------------

class Address {
    city: string = ""
    zip: string = ""
}

class Company {
    name: string = ""
    address: Address = new Address()
}

class Employee {
    name: string = ""
    age: int = 0
    company: Company = new Company()
}

class Dimensions {
    width: double = 0
    height: double = 0
}

class Product {
    label: string = ""
    dims: Dimensions = new Dimensions()
    tags: FixedArray<string> = []
}

class Order {
    id: int = 0
    product: Product = new Product()
    customer: string = ""
}

// ---------------------------------------------------------------------------
// Test: deeply nested object — 3 levels deep
// ---------------------------------------------------------------------------

function testParseUpdateDeeplyNested(): void {
    const emp = new Employee()

    JSON.parseUpdate<Employee>(
        '{"name":"Alice","age":30,"company":{"name":"Acme","address":{"city":"Berlin","zip":"10115"}}}', emp
    )

    arktest.assertEQ(emp.name, "Alice")
    arktest.assertEQ(emp.age, 30)
    arktest.assertEQ(emp.company.name, "Acme")
    arktest.assertEQ(emp.company.address.city, "Berlin")
    arktest.assertEQ(emp.company.address.zip, "10115")
}

// ---------------------------------------------------------------------------
// Test: deeply nested — merge preserves absent inner fields
// ---------------------------------------------------------------------------

function testParseUpdateDeeplyNestedMerge(): void {
    const emp = new Employee()
    emp.name = "Bob"
    emp.age = 40
    emp.company.name = "OldCorp"
    emp.company.address.city = "Munich"
    emp.company.address.zip = "80331"

    JSON.parseUpdate<Employee>('{"company":{"name":"NewCorp"}}', emp)

    arktest.assertEQ(emp.name, "Bob")
    arktest.assertEQ(emp.age, 40)
    arktest.assertEQ(emp.company.name, "NewCorp")
    arktest.assertEQ(emp.company.address.city, "Munich")
    arktest.assertEQ(emp.company.address.zip, "80331")
}

// ---------------------------------------------------------------------------
// Test: deeply nested — roundtrip via stringify + parseUpdate
// ---------------------------------------------------------------------------

function testParseUpdateDeeplyNestedRoundtrip(): void {
    const src = new Employee()
    src.name = "Carol"
    src.age = 28
    src.company.name = "TechCo"
    src.company.address.city = "Tokyo"
    src.company.address.zip = "100-0001"

    const json = JSON.stringify(src)
    const target = new Employee()
    JSON.parseUpdate<Employee>(json, target)

    arktest.assertEQ(target.name, "Carol")
    arktest.assertEQ(target.age, 28)
    arktest.assertEQ(target.company.name, "TechCo")
    arktest.assertEQ(target.company.address.city, "Tokyo")
    arktest.assertEQ(target.company.address.zip, "100-0001")
}

// ---------------------------------------------------------------------------
// Test: deeply nested — identity preserved across all levels
// ---------------------------------------------------------------------------

function testParseUpdateDeeplyNestedIdentity(): void {
    const emp = new Employee()
    const savedCompany = emp.company
    const savedAddress = emp.company.address

    JSON.parseUpdate<Employee>(
        '{"name":"Dan","company":{"name":"X","address":{"city":"Oslo"}}}', emp
    )

    arktest.assertTrue(emp.company === savedCompany)
    arktest.assertTrue(emp.company.address === savedAddress)
    arktest.assertEQ(emp.company.address.city, "Oslo")
}

// ---------------------------------------------------------------------------
// Test: nested object with FixedArray field
// ---------------------------------------------------------------------------

function testParseUpdateNestedWithFixedArray(): void {
    const order = new Order()

    JSON.parseUpdate<Order>(
        '{"id":42,"product":{"label":"Widget","dims":{"width":5.5,"height":3.2},"tags":["sale","new"]},"customer":"Eve"}',
        order
    )

    arktest.assertEQ(order.id, 42)
    arktest.assertEQ(order.customer, "Eve")
    arktest.assertEQ(order.product.label, "Widget")
    arktest.assertEQ(order.product.dims.width, 5.5)
    arktest.assertEQ(order.product.dims.height, 3.2)
    arktest.assertEQ(order.product.tags.length, 2)
    arktest.assertEQ(order.product.tags[0], "sale")
    arktest.assertEQ(order.product.tags[1], "new")
}

// ---------------------------------------------------------------------------
// Test: nested object with FixedArray — roundtrip
// ---------------------------------------------------------------------------

function testParseUpdateNestedWithFixedArrayRoundtrip(): void {
    const src = new Order()
    src.id = 7
    src.customer = "Frank"
    src.product.label = "Gadget"
    src.product.dims.width = 10.0
    src.product.dims.height = 20.0
    src.product.tags = ["featured", "premium", "limited"]

    const json = JSON.stringify(src)
    const target = new Order()
    JSON.parseUpdate<Order>(json, target)

    arktest.assertEQ(target.id, 7)
    arktest.assertEQ(target.customer, "Frank")
    arktest.assertEQ(target.product.label, "Gadget")
    arktest.assertEQ(target.product.dims.width, 10.0)
    arktest.assertEQ(target.product.dims.height, 20.0)
    arktest.assertEQ(target.product.tags.length, 3)
    arktest.assertEQ(target.product.tags[0], "featured")
    arktest.assertEQ(target.product.tags[1], "premium")
    arktest.assertEQ(target.product.tags[2], "limited")
}

// ---------------------------------------------------------------------------
// Test: nested FixedArray of objects within nested object
// ---------------------------------------------------------------------------

class Team {
    name: string = ""
    members: FixedArray<Named> = []
}

class Department {
    title: string = ""
    team: Team = new Team()
}

function testParseUpdateNestedFixedArrayOfObjects(): void {
    const dept = new Department()

    JSON.parseUpdate<Department>(
        '{"title":"Engineering","team":{"name":"Core","members":[{"name":"A","score":10},{"name":"B","score":20}]}}',
        dept
    )

    arktest.assertEQ(dept.title, "Engineering")
    arktest.assertEQ(dept.team.name, "Core")
    arktest.assertEQ(dept.team.members.length, 2)
    arktest.assertEQ(dept.team.members[0].name, "A")
    arktest.assertEQ(dept.team.members[0].score, 10)
    arktest.assertEQ(dept.team.members[1].name, "B")
    arktest.assertEQ(dept.team.members[1].score, 20)
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.parseUpdate')
    suite.addTest('full population', testParseUpdateFullPopulation)
    suite.addTest('merge semantics', testParseUpdateMergeSemantics)
    suite.addTest('no default constructor', testParseUpdateNoDefaultCtor)
    suite.addTest('reviver transforms field', testParseUpdateWithReviver)
    suite.addTest('reviver undefined suppresses field', testParseUpdateReviverUndefined)
    suite.addTest('JSONParseIgnore annotation', testParseUpdateParseIgnore)
    suite.addTest('nested object', testParseUpdateNested)
    suite.addTest('bigIntMode option', testParseUpdateWithBigIntModeOption)
    suite.addTest('nested no-default-ctor roundtrip', testParseUpdateNestedNoDefaultCtor)
    suite.addTest('nested no-default-ctor changed', testParseUpdateNestedNoDefaultCtorChanged)
    suite.addTest('nested no-default-ctor merge', testParseUpdateNestedNoDefaultCtorMerge)
    suite.addTest('reviver handles type mismatch', testParseUpdateReviverTypeMismatch)
    suite.addTest('reviver receives raw value', testParseUpdateReviverReceivesRawValue)
    suite.addTest('reviver JSONObject mismatch throws', testParseUpdateReviverJsonObjectMismatchThrows)
    suite.addTest('reviver JSONArray mismatch throws', testParseUpdateReviverJsonArrayMismatchThrows)
    suite.addTest('reviver numeric field JSONObject throws JSONTypeError', testParseUpdateReviverNumericFieldJsonObjectThrowsJSONTypeError)
    suite.addTest('array fields throws', testParseUpdateArrayFieldsThrows)
    suite.addTest('array fields merge', testParseUpdateArrayFieldsMerge)
    suite.addTest('empty array field throws', testParseUpdateEmptyArrayFieldThrows)
    suite.addTest('fixed array of objects', testParseUpdateFixedArrayOfObjects)
    suite.addTest('fixed array of objects roundtrip', testParseUpdateFixedArrayOfObjectsRoundtrip)
    suite.addTest('fixed array of ints', testParseUpdateFixedArrayOfInts)
    suite.addTest('fixed array of boxed ints', testParseUpdateFixedArrayOfBoxedInts)
    suite.addTest('fixed array of object ints throws', testParseUpdateFixedArrayOfObjectIntsThrows)
    suite.addTest('fixed array of doubles', testParseUpdateFixedArrayOfDoubles)
    suite.addTest('fixed array of booleans', testParseUpdateFixedArrayOfBooleans)
    suite.addTest('fixed array of chars', testParseUpdateFixedArrayOfChars)
    suite.addTest('fixed char array roundtrip', testParseUpdateFixedCharArrayRoundtrip)
    suite.addTest('fixed array of strings', testParseUpdateFixedArrayOfStrings)
    suite.addTest('fixed primitive array merge', testParseUpdateFixedPrimitiveArrayMerge)
    suite.addTest('fixed primitive array roundtrip', testParseUpdateFixedPrimitiveArrayRoundtrip)
    suite.addTest('fixed primitive array empty', testParseUpdateFixedPrimitiveArrayEmpty)
    suite.addTest('array of objects throws', testParseUpdateArrayOfObjectsThrows)
    suite.addTest('reviver rescues array field', testParseUpdateReviverRescuesArrayField)
    suite.addTest('deeply nested', testParseUpdateDeeplyNested)
    suite.addTest('deeply nested merge', testParseUpdateDeeplyNestedMerge)
    suite.addTest('deeply nested roundtrip', testParseUpdateDeeplyNestedRoundtrip)
    suite.addTest('deeply nested identity', testParseUpdateDeeplyNestedIdentity)
    suite.addTest('nested with fixed array', testParseUpdateNestedWithFixedArray)
    suite.addTest('nested with fixed array roundtrip', testParseUpdateNestedWithFixedArrayRoundtrip)
    suite.addTest('nested fixed array of objects', testParseUpdateNestedFixedArrayOfObjects)
    return suite.run()
}
