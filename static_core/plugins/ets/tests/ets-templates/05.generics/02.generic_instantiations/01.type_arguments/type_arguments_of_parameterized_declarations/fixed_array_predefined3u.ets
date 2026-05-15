/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Verify instantiation of the FixedArray with the predefined element type.
---*/

class A<T> {
    // class methods with desired return type
    foo_a(p: FixedArray<T>): FixedArray<T> {
        return p
    }
    foo_b(...p: FixedArray<T>): FixedArray<T> {
        return p
    }
    // class method which can change FixedArray element
    change(v: T, id: int, p: FixedArray<T>): T {
      p[id] = v
      return p[id]
    }
}

// declare object with union
let v_char: A<char>|undefined
let v_boolean: A<boolean>|undefined
let v_byte: A<byte>|undefined
let v_short: A<short>|undefined
let v_int: A<int>|undefined
let v_long: A<long>|undefined
let v_float: A<float>|undefined
let v_double: A<double>|undefined
let v_string: A<string>|undefined
let v_bigint: A<bigint>|undefined
let v_object: A<Object>|undefined
let v_any: A<Any>|undefined
let v_null: A<null>|undefined
let v_undefined: A<undefined>|undefined

function test_double(p: A<double>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<double>|undefined)")
    let fa: FixedArray<double> = [1.5]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<double>, "expect instanceof FixedArray<double>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof double, "expect instanceof double")
    arktest.assertEQ(res1[0], 1.5)
    let res2 = p!.foo_b(2.5)
    arktest.assertTrue(res2 instanceof FixedArray<double>, "expect instanceof FixedArray<double>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof double, "expect instanceof double")
    arktest.assertEQ(res2[0], 2.5)
    let res3 = p!.change(3.7, 0, fa)
    arktest.assertTrue(fa[0] instanceof double, "expect instanceof double")
    arktest.assertEQ(fa[0], 3.7)
    arktest.assertTrue(res3 instanceof double, "expect instanceof double")
    arktest.assertEQ(res3, 3.7)
}

function test_float(p: A<float>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<float>|undefined)")
    let fa: FixedArray<float> = [1.5f]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<float>, "expect instanceof FixedArray<float>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof float, "expect instanceof float")
    arktest.assertEQ(res1[0], 1.5f)
    let res2 = p!.foo_b(2.5f)
    arktest.assertTrue(res2 instanceof FixedArray<float>, "expect instanceof FixedArray<float>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof float, "expect instanceof float")
    arktest.assertEQ(res2[0], 2.5f)
    let res3 = p!.change(3.7f, 0, fa)
    arktest.assertTrue(fa[0] instanceof float, "expect instanceof float")
    arktest.assertEQ(fa[0], 3.7f)
    arktest.assertTrue(res3 instanceof float, "expect instanceof float")
    arktest.assertEQ(res3, 3.7f)
}

function test_long(p: A<long>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<long>|undefined)")
    let fa: FixedArray<long> = [9223372036854775807]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<long>, "expect instanceof FixedArray<long>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof long, "expect instanceof long")
    arktest.assertEQ(res1[0], 9223372036854775807)
    let res2 = p!.foo_b(123)
    arktest.assertTrue(res2 instanceof FixedArray<long>, "expect instanceof FixedArray<long>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof long, "expect instanceof long")
    arktest.assertEQ(res2[0], 123)
    let res3 = p!.change(567, 0, fa)
    arktest.assertTrue(fa[0] instanceof long, "expect instanceof long")
    arktest.assertEQ(fa[0], 567)
    arktest.assertTrue(res3 instanceof long, "expect instanceof long")
    arktest.assertEQ(res3, 567)
}

function test_int(p: A<int>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<int>|undefined)")
    let fa: FixedArray<int> = [2147483647]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<int>, "expect instanceof FixedArray<int>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof int, "expect instanceof int")
    arktest.assertEQ(res1[0], 2147483647)
    let res2 = p!.foo_b(123)
    arktest.assertTrue(res2 instanceof FixedArray<int>, "expect instanceof FixedArray<int>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof int, "expect instanceof int")
    arktest.assertEQ(res2[0], 123)
    let res3 = p!.change(567, 0, fa)
    arktest.assertTrue(fa[0] instanceof int, "expect instanceof int")
    arktest.assertEQ(fa[0], 567)
    arktest.assertTrue(res3 instanceof int, "expect instanceof int")
    arktest.assertEQ(res3, 567)
}

function test_short(p: A<short>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<short>|undefined)")
    let fa: FixedArray<short> = [32767]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<short>, "expect instanceof FixedArray<short>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof short, "expect instanceof short")
    arktest.assertEQ(res1[0], 32767)
    let res2 = p!.foo_b(123)
    arktest.assertTrue(res2 instanceof FixedArray<short>, "expect instanceof FixedArray<short>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof short, "expect instanceof short")
    arktest.assertEQ(res2[0], 123)
    let res3 = p!.change(567, 0, fa)
    arktest.assertTrue(fa[0] instanceof short, "expect instanceof short")
    arktest.assertEQ(fa[0], 567)
    arktest.assertTrue(res3 instanceof short, "expect instanceof short")
    arktest.assertEQ(res3, 567)
}

function test_byte(p: A<byte>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<byte>|undefined)")
    let fa: FixedArray<byte> = [127]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<byte>, "expect instanceof FixedArray<byte>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof byte, "expect instanceof byte")
    arktest.assertEQ(res1[0], 127)
    let res2 = p!.foo_b(123)
    arktest.assertTrue(res2 instanceof FixedArray<byte>, "expect instanceof FixedArray<byte>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof byte, "expect instanceof byte")
    arktest.assertEQ(res2[0], 123)
    let res3 = p!.change(15, 0, fa)
    arktest.assertTrue(fa[0] instanceof byte, "expect instanceof byte")
    arktest.assertEQ(fa[0], 15)
    arktest.assertTrue(res3 instanceof byte, "expect instanceof byte")
    arktest.assertEQ(res3, 15)
}

function test_boolean(p: A<boolean>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<boolean>|undefined)")
    let fa: FixedArray<boolean> = [true]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<boolean>, "expect instanceof FixedArray<boolean>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof boolean, "expect instanceof boolean")
    arktest.assertEQ(res1[0], true)
    let res2 = p!.foo_b(false)
    arktest.assertTrue(res2 instanceof FixedArray<boolean>, "expect instanceof FixedArray<boolean>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof boolean, "expect instanceof boolean")
    arktest.assertEQ(res2[0], false)
    let res3 = p!.change(false, 0, fa)
    arktest.assertTrue(fa[0] instanceof boolean, "expect instanceof boolean")
    arktest.assertEQ(fa[0], false)
    arktest.assertTrue(res3 instanceof boolean, "expect instanceof boolean")
    arktest.assertEQ(res3, false)
}

function test_char(p: A<char>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<char>|undefined)")
    let fa: FixedArray<char> = [c'x']
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<char>, "expect instanceof FixedArray<char>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof char, "expect instanceof char")
    arktest.assertEQ(res1[0], c'x')
    let res2 = p!.foo_b(c'y')
    arktest.assertTrue(res2 instanceof FixedArray<char>, "expect instanceof FixedArray<char>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof char, "expect instanceof char")
    arktest.assertEQ(res2[0], c'y')
    let res3 = p!.change(c'z', 0, fa)
    arktest.assertTrue(fa[0] instanceof char, "expect instanceof char")
    arktest.assertEQ(fa[0], c'z')
    arktest.assertTrue(res3 instanceof char, "expect instanceof char")
    arktest.assertEQ(res3, c'z')
}

function test_string(p: A<string>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<string>|undefined)")
    let fa: FixedArray<string> = ["abc"]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<string>, "expect instanceof FixedArray<string>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof string, "expect instanceof string")
    arktest.assertEQ(res1[0], "abc")
    let res2 = p!.foo_b("efg")
    arktest.assertTrue(res2 instanceof FixedArray<string>, "expect instanceof FixedArray<string>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof string, "expect instanceof string")
    arktest.assertEQ(res2[0], "efg")
    let res3 = p!.change("123", 0, fa)
    arktest.assertTrue(fa[0] instanceof string, "expect instanceof string")
    arktest.assertEQ(fa[0], "123")
    arktest.assertTrue(res3 instanceof string, "expect instanceof string")
    arktest.assertEQ(res3, "123")
}

function test_bigint(p: A<bigint>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<bigint>|undefined)")
    let fa: FixedArray<bigint> = [5n]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<bigint>, "expect instanceof FixedArray<bigint>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof bigint, "expect instanceof bigint")
    arktest.assertEQ(res1[0], 5n)
    let res2 = p!.foo_b(123n)
    arktest.assertTrue(res2 instanceof FixedArray<bigint>, "expect instanceof FixedArray<bigint>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof bigint, "expect instanceof bigint")
    arktest.assertEQ(res2[0], 123n)
    let res3 = p!.change(567n, 0, fa)
    arktest.assertTrue(fa[0] instanceof bigint, "expect instanceof bigint")
    arktest.assertEQ(fa[0], 567n)
    arktest.assertTrue(res3 instanceof bigint, "expect instanceof bigint")
    arktest.assertEQ(res3, 567n)
}

function test_object(p: A<Object>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<Object>|undefined)")
    let fa: FixedArray<Object> = ["abc"]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<Object>, "expect instanceof FixedArray<Object>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof Object, "expect instanceof Object")
    arktest.assertEQ(res1[0], "abc")
    let res2 = p!.foo_b(5n)
    arktest.assertTrue(res2 instanceof FixedArray<Object>, "expect instanceof FixedArray<Object>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof Object, "expect instanceof Object")
    arktest.assertEQ(res2[0], 5n)
    let res3 = p!.change(c'x', 0, fa)
    arktest.assertTrue(fa[0] instanceof Object, "expect instanceof Object")
    arktest.assertEQ(fa[0], c'x')
    arktest.assertTrue(res3 instanceof Object, "expect instanceof Object")
    arktest.assertEQ(res3, c'x')
}

function test_any(p: A<Any>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<Any>|undefined)")
    let fa: FixedArray<Any> = ["abc"]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<Any>, "expect instanceof FixedArray<Any>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof Any, "expect instanceof Any")
    arktest.assertEQ(res1[0], "abc")
    let res2 = p!.foo_b(5n)
    arktest.assertTrue(res2 instanceof FixedArray<Any>, "expect instanceof FixedArray<Any>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof Any, "expect instanceof Any")
    arktest.assertEQ(res2[0], 5n)
    let res3 = p!.change(null, 0, fa)
    arktest.assertTrue(fa[0] instanceof Any, "expect instanceof Any")
    arktest.assertEQ(fa[0], null)
    arktest.assertTrue(res3 instanceof Any, "expect instanceof Any")
    arktest.assertEQ(res3, null)
}

function test_null(p: A<null>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<null>|undefined)")
    let fa: FixedArray<null> = [null]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<null>, "expect instanceof FixedArray<null>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof null, "expect instanceof null")
    arktest.assertEQ(res1[0], null)
    let res2 = p!.foo_b(null)
    arktest.assertTrue(res2 instanceof FixedArray<null>, "expect instanceof FixedArray<null>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof null, "expect instanceof null")
    arktest.assertEQ(res2[0], null)
    let res3 = p!.change(null, 0, fa)
    arktest.assertTrue(fa[0] instanceof null, "expect instanceof null")
    arktest.assertEQ(fa[0], null)
    arktest.assertTrue(res3 instanceof null, "expect instanceof null")
    arktest.assertEQ(res3, null)
}

function test_undefined(p: A<undefined>|undefined) {
    arktest.assertTrue(p instanceof A|undefined, "expect instanceof A|undefined (A<undefined>|undefined)")
    let fa: FixedArray<undefined> = [undefined]
    let res1 = p!.foo_a(fa)
    arktest.assertTrue(res1 instanceof FixedArray<undefined>, "expect instanceof FixedArray<undefined>")
    arktest.assertEQ(res1.length, 1)
    arktest.assertTrue(res1[0] instanceof undefined, "expect instanceof undefined")
    arktest.assertEQ(res1[0], undefined)
    let res2 = p!.foo_b(undefined)
    arktest.assertTrue(res2 instanceof FixedArray<undefined>, "expect instanceof FixedArray<undefined>")
    arktest.assertEQ(res2.length, 1)
    arktest.assertTrue(res2[0] instanceof undefined, "expect instanceof undefined")
    arktest.assertEQ(res2[0], undefined)
    let res3 = p!.change(undefined, 0, fa)
    arktest.assertTrue(fa[0] instanceof undefined, "expect instanceof undefined")
    arktest.assertEQ(fa[0], undefined)
    arktest.assertTrue(res3 instanceof undefined, "expect instanceof undefined")
    arktest.assertEQ(res3, undefined)
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('fixed_array_predefined3u');
    testSuite.addTest('test_double', () => {
        v_double = new A<double>
        test_double(v_double)
    });
    testSuite.addTest('test_float', () => {
        v_float = new A<float>
        test_float(v_float)
    });
    testSuite.addTest('test_long', () => {
        v_long = new A<long>
        test_long(v_long)
    });
    testSuite.addTest('test_int', () => {
        v_int = new A<int>
        test_int(v_int)
    });
    testSuite.addTest('test_short', () => {
        v_short = new A<short>
        test_short(v_short)
    });
    testSuite.addTest('test_byte', () => {
        v_byte = new A<byte>
        test_byte(v_byte)
    });
    testSuite.addTest('test_boolean', () => {
        v_boolean = new A<boolean>
        test_boolean(v_boolean)
    });
    testSuite.addTest('test_char', () => {
        v_char = new A<char>
        test_char(v_char)
    });
    testSuite.addTest('test_string', () => {
        v_string = new A<string>
        test_string(v_string)
    });
    testSuite.addTest('test_bigint', () => {
        v_bigint = new A<bigint>
        test_bigint(v_bigint)
    });
    testSuite.addTest('test_object', () => {
        v_object = new A<Object>
        test_object(v_object)
    });
    testSuite.addTest('test_any', () => {
        v_any = new A<Any>
        test_any(v_any)
    });
    testSuite.addTest('test_null', () => {
        v_null = new A<null>
        test_null(v_null)
    });
    testSuite.addTest('test_undefined', () => {
        v_undefined = new A<undefined>
        test_undefined(v_undefined)
    });
    return testSuite.run();
}
