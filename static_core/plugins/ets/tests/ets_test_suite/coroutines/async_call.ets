/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

// Test async bridge
// Check all parameters are passed correctly for different architectures

let called: boolean = false;

function assert_eq(value1: int, value2: int): void {
    if (value1 == value2) {
        return;
    }
    console.println("Values of type int are not equal: " + value1 + " != " + value2);
    throw new Error();
}

function assert_eq(value1: double, value2: double): void {
    if (value1 == value2) {
        return;
    }
    console.println("Values of type double are not equal: " + value1 + " != " + value2);
    throw new Error();
}

function prepare(): void {
    called = false;
}

// check no-args static function call
async function foo(): Promise<Object | null> {
    called = true;
    return null;
}

// check function call when all arguments are in registers on x86-64
async function amd64_small(i1: int, i2: int, i3: int, i4: int, i5: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    return null;
}

// check function call when some arguments are passed via the stack on x86-64
async function amd64_large(i1: int, i2: int, i3: int, i4: int, i5: int, i6: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double, f9: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    assert_eq(i6, 6);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    assert_eq(f9, 9.0);
    return null;
}

// check function call when all arguments are in registers on arm32
async function arm32_small(i1: int, i2: int, i3: int): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    return null;
}

// check function call when some arguments are passed via the stack on arm32
async function arm32_large(i1: int, i2: int, i3: int, i4: int, i5: int): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    return null;
}

// check function call when all arguments are in registers on arm32hf
async function arm32hf_small(i1: int, i2: int, i3: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    return null;
}

// check function call when some arguments are passed via the stack on arm32hf
async function arm32hf_large(i1: int, i2: int, i3: int, i4: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double, f9: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    assert_eq(f9, 9.0);
    return null;
}

// check function call when all arguments are in registers on arm64
async function arm64_small(i1: int, i2: int, i3: int, i4: int, i5: int, i6: int, i7: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    assert_eq(i6, 6);
    assert_eq(i7, 7);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    return null;
}

// check function call when some arguments are passed via the stack on arm64
async function arm64_large(i1: int, i2: int, i3: int, i4: int, i5: int, i6: int, i7: int, i8: int, f1: double, f2: double, f3: double, f4: double, f5: double, f6: double, f7: double, f8: double, f9: double): Promise<Object | null> {
    called = true;
    assert_eq(i1, 1);
    assert_eq(i2, 2);
    assert_eq(i3, 3);
    assert_eq(i4, 4);
    assert_eq(i5, 5);
    assert_eq(i6, 6);
    assert_eq(i7, 7);
    assert_eq(i8, 8);
    assert_eq(f1, 1.0);
    assert_eq(f2, 2.0);
    assert_eq(f3, 3.0);
    assert_eq(f4, 4.0);
    assert_eq(f5, 5.0);
    assert_eq(f6, 6.0);
    assert_eq(f7, 7.0);
    assert_eq(f8, 8.0);
    assert_eq(f9, 9.0);
    return null;
}

async function crasher(): Promise<Int> {
    return await Promise.resolve(1);
}

// check async/await in global context
try {
    let x = crasher();
} catch (e) {
    assertTrue(e instanceof InvalidCoroutineOperationError);
}

function main(): void {
    prepare();
    let p = foo();
    assertTrue(called);
    await p;

    prepare();
    p = amd64_small(1, 2, 3, 4, 5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    assertTrue(called);
    await p;

    prepare();
    p = amd64_large(1, 2, 3, 4, 5, 6, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    assertTrue(called);
    await p;

    prepare();
    p = arm32_small(1, 2, 3);
    assertTrue(called);
    await p;

    prepare();
    p = arm32_large(1, 2, 3, 4, 5);
    assertTrue(called);
    await p;

    prepare();
    p = arm32hf_small(1, 2, 3, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    assertTrue(called);
    await p;

    prepare();
    p = arm32hf_large(1, 2, 3, 4, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    assertTrue(called);
    await p;

    prepare();
    p = arm64_small(1, 2, 3, 4, 5, 6, 7, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    assertTrue(called);
    await p;

    prepare();
    p = arm64_large(1, 2, 3, 4, 5, 6, 7, 8, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    assertTrue(called);
    await p;
}
