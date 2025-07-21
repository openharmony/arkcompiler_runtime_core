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

function istrue(v: Any) { return !!v }

function erase<T>(v: T): Any {
    return v;
}

class Klass {
    public f:number;

    constructor(f:number) {
        this.f= f;
    }
}

function foo(n: number): Klass | undefined {
    if(n == 0){
        return undefined;
    }
    return new Klass(n);
}

enum Color {
    Red,
    Green,
    Blue = 2147483647,
    Yellow = -2147483648,
}

enum Color64 {
    Red,
    Green,
    Blue = 9223372036854775807,
    Yellow = -9223372036854775808,
}

enum ColorString {
    Red = "",
    Green = "Green"
}

function testNormal() {
    assertTrue(!istrue(new Byte(0 as byte)));
    assertTrue(istrue(new Byte(1 as byte)));
    assertTrue(!istrue(new Short(0 as short)));
    assertTrue(istrue(new Short(1 as short)));
    assertTrue(!istrue(new Int(0)));
    assertTrue(istrue(new Int(1)));
    assertTrue(!istrue(new Long(0)));
    assertTrue(istrue(new Long(1)));
    assertTrue(!istrue(new Float(0.0)));
    assertTrue(istrue(new Float(0.5)));
    assertTrue(!istrue(new Double(0.0)));
    assertTrue(istrue(new Double(0.5)));
    assertTrue(!istrue(new Char(0 as char)));
    assertTrue(istrue(new Char(c'0')));
    assertTrue(!istrue(""));
    assertTrue(istrue("Hello"));
    assertTrue(!istrue(false));
    assertTrue(istrue(true));
    assertTrue(!istrue(null));
    assertTrue(!istrue(undefined));
    assertTrue(!istrue(NaN));
    assertTrue(!istrue(new bigint(0)));
    assertTrue(istrue(new bigint(1)));
    assertTrue(!istrue(foo(0)));
    assertTrue(istrue(foo(1)));
    assertTrue(!istrue(Color.Red));
    assertTrue(istrue(Color.Green));
    assertTrue(istrue(Color.Blue));
    assertTrue(istrue(Color.Yellow));
    assertTrue(!istrue(Color64.Red));
    assertTrue(istrue(Color64.Green));
    assertTrue(istrue(Color64.Blue));
    assertTrue(istrue(Color64.Yellow));
    assertTrue(!istrue(ColorString.Red));
    assertTrue(istrue(ColorString.Green));
    let a: bigint | null = 0n;
    assertTrue(istrue(!a));
}

function testErase() {
    assertTrue(!istrue(erase(new Byte(0 as byte))));
    assertTrue(istrue(erase(new Byte(1 as byte))));
    assertTrue(!istrue(erase(new Short(0 as short))));
    assertTrue(istrue(erase(new Short(1 as short))));
    assertTrue(!istrue(erase(new Int(0))));
    assertTrue(istrue(erase(new Int(1))));
    assertTrue(!istrue(erase(new Long(0))));
    assertTrue(istrue(erase(new Long(1))));
    assertTrue(!istrue(erase(new Float(0.0))));
    assertTrue(istrue(erase(new Float(0.5))));
    assertTrue(!istrue(erase(new Double(0.0))));
    assertTrue(istrue(erase(new Double(0.5))));
    assertTrue(!istrue(erase(new Char(0 as char))));
    assertTrue(istrue(erase(new Char(c'0'))));
    assertTrue(!istrue(erase("")));
    assertTrue(istrue(erase("Hello")));
    assertTrue(!istrue(erase(false)));
    assertTrue(istrue(erase(true)));
    assertTrue(!istrue(erase(null)));
    assertTrue(!istrue(erase(undefined)));
    assertTrue(!istrue(erase(NaN)));
    assertTrue(!istrue(erase(new bigint(0))));
    assertTrue(istrue(erase(new bigint(1))));
    assertTrue(!istrue(erase(foo(0))));
    assertTrue(istrue(erase(foo(1))));
    assertTrue(!istrue(erase(Color.Red)));
    assertTrue(istrue(erase(Color.Green)));
    assertTrue(istrue(erase(Color.Blue)));
    assertTrue(istrue(erase(Color.Yellow)));
    assertTrue(!istrue(erase(Color64.Red)));
    assertTrue(istrue(erase(Color64.Green)));
    assertTrue(istrue(erase(Color64.Blue)));
    assertTrue(istrue(erase(Color64.Yellow)));
    assertTrue(!istrue(erase(ColorString.Red)));
    assertTrue(istrue(erase(ColorString.Green)));
    let a: bigint | null = 0n;
    assertTrue(istrue(erase(!a)));
}

function testConditions() {
    if (istrue(0)) {
        assertTrue(false);
    } else {
        assertTrue(true);
    }

    if (istrue(1)) {
        assertTrue(true);
    } else {
        assertTrue(false);
    }
}


function testFunction() {
    if (istrue((() => { let x: number = 0; return x })())) {
        assertTrue(false);
    } else {
        assertTrue(true);
    }
}

function testMemoryAllocationsWithIsTrue() {
    const totalAllocations = 10000;
    let allocations: Array<Object> = new Array<Object>(totalAllocations);
    let count = 0;

    for (let i = 0; i < totalAllocations; i++) {
        allocations[i] = new ArrayBuffer(1024);
        if (istrue(erase(new Int(1)))) {
            count++
        }
    }
    assertEQ(count, totalAllocations);
}

function main() {
    testNormal();
    testErase();
    testConditions();
    testFunction();
    testMemoryAllocationsWithIsTrue();
}
