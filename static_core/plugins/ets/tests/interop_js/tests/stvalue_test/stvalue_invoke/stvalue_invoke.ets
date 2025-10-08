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

export class Student {
    private static id: int = 999;
    private age: int = 0;
    private name: string = '';

    constructor() { }
    constructor(age: int) { this.age = age; age = age + 2; }
    constructor(age: int, name: string) {
        console.log('  ets log, Student Ctor--age:', this.age, '  name:', this.name);
        this.age = age;
        this.name = name;
    }

    getStudentAge(): int {
        console.log('  ets log, age:', this.age);
        return this.age;
    }

    setStudentAge(newAge: int): void {
        this.age = newAge;
        console.log('  ets log, age:', this.age);
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
    constructor() { super() }
    constructor(age: int) { super(age) }
    constructor(age: int, name: string) { super(age, name) }
}

export let getNumberFn = () => { return 123; }

// boolean type
export function BooleanInvoke(b1 : Boolean, b2 : Boolean) : Boolean
{
    return b1 & b2;
}

// char type
export function CharInvoke(c : char) : char
{
    return c;
}

// byte type
export function ByteInvoke(b : byte) : byte
{
    return b;
}

// short type
export function ShortInvoke(s : short) : short
{
    return s;
}

// int type
export function IntInvoke(a : int, b : int) : int
{
    return a + b;
}

// long type
export function LongInvoke(l1 : long, l2 : long) : long
{
    return l1 + l2;
}

// float type
export function FloatInvoke(f1 : float, f2 : float) : float
{
    return f1 + f2;
}

// double type
export function DoubleInvoke(d1 : double, d2 : double) : double
{
    return d1 + d2;
}

// number type
export function NumberInvoke(n1 : number, n2 : number) : number
{
    return n1 + n2;
}

// reference string type
export function StringInvoke(str1 : String, str2 : String) : String
{
    return str1 + str2;
}

// reference BigInt type
export function BigIntInvoke(b1 : BigInt, b2 : BigInt) : BigInt
{
    return b1 + b2;
}

// void
export function VoidInvoke(str1 : String, str2 : String)
{
    console.log(str1 + str2);
}

export namespace Invoke {

// boolean type
export function BooleanInvoke(b1 : Boolean, b2 : Boolean) : Boolean
{
    return b1 & b2;
}

// char type
export function CharInvoke(c : char) : char
{
    return c;
}

// byte type
export function ByteInvoke(b : byte) : byte
{
    return b;
}

// short type
export function ShortInvoke(s : short) : short
{
    return s;
}

// int type
export function IntInvoke(a : int, b : int) : int
{
    return a + b;
}

// long type
export function LongInvoke(l1 : long, l2 : long) : long
{
    return l1 + l2;
}

// float type
export function FloatInvoke(f1 : float, f2 : float) : float
{
    return f1 + f2;
}

// double type
export function DoubleInvoke(d1 : double, d2 : double) : double
{
    return d1 + d2;
}

// number type
export function NumberInvoke(n1 : number, n2 : number) : number
{
    return n1 + n2;
}

// reference string type
export function StringInvoke(str1 : String, str2 : String) : String
{
    return str1 + str2;
}

// reference BigInt type
export function BigIntInvoke(b1 : BigInt, b2 : BigInt) : BigInt
{
    return b1 + b2;
}

// void
export function VoidInvoke(str1 : String, str2 : String)
{
    console.log(str1 + str2);
}
}  // namespace Invoke