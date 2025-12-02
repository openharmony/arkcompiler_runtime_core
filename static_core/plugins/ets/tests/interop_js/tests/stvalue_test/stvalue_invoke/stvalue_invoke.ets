/**
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

export function runFullGC(): void {
    GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
}

export function createWeakRef(dyDate: Date): WeakRef<Date> {
    return new WeakRef<Date>(dyDate);
}

export class Student {
    private static id: int = 999;
    private age: int = 0;
    private name: string = '';

    constructor() { }
    constructor(age: int) { this.age = age; }
    constructor(age: int, name: string) {
        this.age = age;
        this.name = name;
    }

    getStudentAge(): int {
        return this.age;
    }

    setStudentAge(newAge: int): void {
        this.age = newAge;
    }

    setStudentAge(newAge: double): void {
        console.log("ets log, setDouble value, newAge:", newAge)
        this.age = newAge.toInt();
    }

    getStudentName(): string {
        return this.name;
    }

    static getStudentId(): int {
        return Student.id;
    }

    static setStudentId(newId: int): void {
        Student.id = newId;
    }

}

export class SubStudent extends Student {
    uName: string = '';
    uAge: int = 0;
    static uID: int = 0;
    constructor() { super() }
    constructor(age: int) { super(age) }
    constructor(age: int, name: string) {
        super(age, name);
        this.uAge = age;
        this.uName = this.uName;
    }

    static setUId(newId: int): void {
        SubStudent.uID = newId;
    }
    static setUId(newId: double): void {
        SubStudent.uID = newId.toInt();
    }
    static getUId(): int {
        return SubStudent.uID;
    }
}



// boolean type
export function BooleanInvoke(b1: Boolean, b2: Boolean): Boolean {
    return b1 && b2;
}

export function BooleanEmptyInvoke(): Boolean {
    return true;
}

// char type
export function CharInvoke(c: char): char {
    return c;
}

export function CharEmptyInvoke(): char {
    return c'c';
}

// byte type
export function ByteInvoke(b: byte): byte {
    return b;
}

export function ByteEmptyInvoke(): byte {
    return 112;
}

// short type
export function ShortInvoke(s: short): short {
    return s;
}

export function ShortEmptyInvoke(): short {
    return 3456;
}

// int type
export function IntInvoke(a: int, b: int): int {
    return a + b;
}

export function IntEmptyInvoke(): int {
    return 468;
}

// long type
export function LongInvoke(l1: long, l2: long): long {
    return l1 + l2;
}

export function LongEmptyInvoke(): long {
    return 468;
}

// float type
export function FloatInvoke(f1: float, f2: float): float {
    return f1 + f2;
}

export function FloatEmptyInvoke(): float {
    return 1.23f;
}

// double type
export function DoubleInvoke(d1: double, d2: double): double {
    return d1 + d2;
}

export function DoubleEmptyInvoke(): double {
    return 1.23;
}

// number type
export function NumberInvoke(n1: number, n2: number): number {
    return n1 + n2;
}

export function NumberEmptyInvoke(): number {
    return 1;
}

// reference string type
export function StringInvoke(str1: String, str2: String): String {
    return str1 + str2;
}

export function StringEmptyInvoke(): String {
    return 'string';
}

// reference BigInt type
export function BigIntInvoke(b1: BigInt, b2: BigInt): BigInt {
    return b1 + b2;
}

export function BigIntEmptyInvoke(): BigInt {
    return BigInt('1234');
}

// void
export function VoidInvoke(str1: String, str2: String) {
    console.log(str1 + str2);
}

export function VoidEmptyInvoke() {
    console.log('string');
}

export namespace Invoke {

    export let getNumberFn = () => { return 123; }

    export let getSumFn = (n1: number, n2: number) => { return n1 + n2; }

    // boolean type
    export function BooleanInvoke(b1: Boolean, b2: Boolean): Boolean {
        return b1 && b2;
    }

    export function BooleanEmptyInvoke(): Boolean {
        return true;
    }

    // char type
    export function CharInvoke(c: char): char {
        return c;
    }

    export function CharEmptyInvoke(): char {
        return c'c';
    }

    // byte type
    export function ByteInvoke(b: byte): byte {
        return b;
    }

    export function ByteEmptyInvoke(): byte {
        return 112;
    }

    // short type
    export function ShortInvoke(s: short): short {
        return s;
    }

    export function ShortEmptyInvoke(): short {
        return 3456;
    }

    // int type
    export function IntInvoke(a: int, b: int): int {
        return a + b;
    }

    export function IntEmptyInvoke(): int {
        return 468;
    }

    // long type
    export function LongInvoke(l1: long, l2: long): long {
        return l1 + l2;
    }

    export function LongEmptyInvoke(): long {
        return 468;
    }

    // float type
    export function FloatInvoke(f1: float, f2: float): float {
        return f1 + f2;
    }

    export function FloatEmptyInvoke(): float {
        return 1.23f;
    }

    // double type
    export function DoubleInvoke(d1: double, d2: double): double {
        return d1 + d2;
    }

    export function DoubleEmptyInvoke(): double {
        return 1.23;
    }

    // number type
    export function NumberInvoke(n1: number, n2: number): number {
        return n1 + n2;
    }

    export function NumberEmptyInvoke(): number {
        return 1;
    }

    // reference string type
    export function StringInvoke(str1: String, str2: String): String {
        return str1 + str2;
    }

    export function StringEmptyInvoke(): String {
        return 'string';
    }

    // reference BigInt type
    export function BigIntInvoke(b1: BigInt, b2: BigInt): BigInt {
        return b1 + b2;
    }

    export function BigIntEmptyInvoke(): BigInt {
        return new BigInt('1234');
    }

    // void
    export function VoidInvoke(str1: String, str2: String) {
        console.log(str1 + str2);
    }

    export function VoidEmptyInvoke() {
        console.log('string');
    }
}  // namespace Invoke