/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

function erase<T>(v: T): Any {
    return v;
}

function foo(v: int) {
    return v
};

function foo2(v: int | object) {
    return v
};

function testNumerics() {
    assertEQ(typeof (() => { let v: byte = 1; return v })(), "number");
    assertEQ(typeof (() => { let v: short = 1; return v })(), "number");
    assertEQ(typeof (() => { let v: int = 1; return v })(), "number");
    assertEQ(typeof (() => { let v: long = 123456789123; return v })(), "number");
    assertEQ(typeof (() => { let v: float = 1.1f; return v })(), "number");
    assertEQ(typeof (() => { let v: double = 1.1; return v })(), "number");
    assertEQ(typeof (() => { let v: char = c'a'; return v })(), "number");
}

function testNumerics2() {
    let a1: byte = 1;
    let a2: short = 1;
    let a3: int = 1;
    let a4: long = 123456789123;
    let a5: float = 1.1f;
    let a6: double = 1.1;
    let a7: char = c'a';
    assertEQ(typeof erase(a1), "number");
    assertEQ(typeof erase(a2), "number");
    assertEQ(typeof erase(a3), "number");
    assertEQ(typeof erase(a4), "number");
    assertEQ(typeof erase(a5), "number");
    assertEQ(typeof erase(a6), "number");
    assertEQ(typeof erase(a7), "number");
}

function testString() {
    assertEQ(typeof (() => { let v: string = "Hello World"; return v })(), "string");
    assertEQ(typeof (() => { let v: String = "Hello World"; return v })(), "string");
    assertEQ(typeof erase(new string("Hello World")), "string");
    assertEQ(typeof erase(new String("Hello World")), "string");
}

function testBoolean() {
    assertEQ(typeof (() => { let v: boolean = true; return v })(), "boolean");
    assertEQ(typeof (() => { let v: Boolean = false; return v })(), "boolean");
    assertEQ(typeof erase(true), "boolean");
    assertEQ(typeof erase(false), "boolean");
}

function testBigInt() {
    assertEQ(typeof (() => { let v: BigInt = new BigInt(10000000000000); return v })(), "bigint");
    assertEQ(typeof (() => { let v: bigint = new bigint(99999999999999); return v })(), "bigint");
    assertEQ(typeof erase(new BigInt(10000000000000)), "bigint");
    assertEQ(typeof erase(new bigint(99999999999999)), "bigint");
}

function testObject() {
    let obj1: Object = {};
    let obj2: Object = [1, 2, 3];
    let obj3: Object = () => { console.log("Hello World") };
    let obj4: Object = new Date();
    let obj5: Object = 123;

    assertEQ(typeof obj1, "object");
    assertEQ(typeof obj2, "object");
    assertEQ(typeof obj3, "function");
    assertEQ(typeof obj4, "object");
    assertEQ(typeof obj5, "number");

    assertEQ(typeof erase(obj1), "object");
    assertEQ(typeof erase(obj2), "object");
    assertEQ(typeof erase(obj3), "function");
    assertEQ(typeof erase(obj4), "object");
    assertEQ(typeof erase(obj5), "number");
}

let f1: () => void = () => {
    console.log("Hello World");
}

function normalFuction() {
    console.log("Hello World");
}

function testFunction() {
    let f2 = normalFuction;
    assertEQ(typeof f1, "function");
    assertEQ(typeof f2, "function");

    assertEQ(typeof erase(f1), "function");
    assertEQ(typeof erase(f2), "function");
}

function testUndefined() {
    let v = undefined;
    assertEQ(typeof v, "undefined");
    assertEQ(typeof erase(v), "undefined");
}

function testNull() {
    let v = null;
    assertEQ(typeof v, "object");
    assertEQ(typeof erase(v), "object");
}

enum Direction {
    Up = 1,
    Down,
    left,
    Right
}

enum Status {
    Active = "ACTIVE",
    Inactive = "INACTIVE",
    Pending = "PENDING"
}

function testEnum() {
    let dir: Direction = Direction.Up;
    let status: Status = Status.Active;
    assertEQ(typeof dir, "number");
    assertEQ(typeof status, "string");
    // The enum is wrapped as an object type when calling erase
    assertEQ(typeof erase(dir), "number");
    assertEQ(typeof erase(status), "string");
    assertEQ(typeof foo(dir), "number");
    // The union type will convert all its members into object
    assertEQ(typeof foo2(dir), "number");
}

let v1: string | number | boolean = "abc";
let v2: string | number | null | undefined = 123;

function testUnion() {
    assertEQ(typeof v1, "string");
    assertEQ(typeof v2, "number");
    assertEQ(typeof erase(v1), "string");
    assertEQ(typeof erase(v2), "number");
}

class A implements Function {
    unsafeCall(...r: FixedArray<NullishType>): NullishType { return null }
    get name() {
        return "A";
    }
}
class B extends Numeric {
    public toByte(): byte { return 1 };
    public toInt(): int { return 1 };
    public toShort(): short { return 1 };
    public toLong(): long { return 1 };
    public toFloat(): float { return 1.1 };
    public toDouble(): double { return 1.1 };
}

function testClass() {
    let a = new A();
    let b = new B();
    assertEQ(typeof a, "function");
    assertEQ(typeof b, "object");
    assertEQ(typeof erase(a), "function");
    assertEQ(typeof erase(b), "object");
}

interface User {
    id: number;
}

function testInterface() {
    let u: User = {
        id: 1
    }
    assertEQ(typeof u, "object");
    assertEQ(typeof erase(u), "object");
}

function main() {
    testNumerics();
    testNumerics2();
    testString();
    testBoolean();
    testBigInt();
    testObject();
    testFunction();
    testUndefined();
    testNull();
    testEnum();
    testUnion();
    testClass();
    testInterface();
}
