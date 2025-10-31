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
    public age: int = 0;
    public name: string = '';

    constructor() { }
    constructor(age: int) { this.age = age; }
    constructor(age: int, name: string) {
        this.age = age;
        this.name = name;
    }
}

export class SubStudent extends Student {
    constructor() { super() }
    constructor(age: int) { super(age) }
    constructor(age: int, name: string) { super(age, name) }
}

function checkPrimitiveArray<T>(arr: FixedArray<T>, value: T): boolean {
    if (arr.length != 5) {
        return false;
    }
    for (let i = 0; i < 5; i++) {
        if (arr[i] != value) {
            return false;
        }
    }
    return true;
}

export function checkFixPrimitiveBooleanArray(arr: FixedArray<boolean>): boolean {
    if (arr.length != 5) {
        return false;
    }
    for (let i = 0; i < 5; i++) {
        console.log(arr[i])
        if (arr[i] != false) {
            return false;
        }
    }
    return true;
}

export function checkFixPrimitiveByteArray(arr: FixedArray<byte>): boolean {
    if (arr.length != 5) {
        return false;
    }
    for (let i = 0; i < 5; i++) {
        console.log(arr[i])
        if (arr[i] != 0) {
            return false;
        }
    }
    return true;
}

export function checkFixPrimitiveCharArray(arr: FixedArray<char>): boolean {
    if (arr.length != 5) {
        return false;
    }
    console.log('----------------')
    for (let i = 0; i < 5; i++) {
        if (arr[i] != c'\0') {
            return false;
        }
    }
    return true;
}

export function checkFixPrimitiveShortArray(arr: FixedArray<short>): boolean {
    if (arr.length != 5) {
        return false;
    }
    for (let i = 0; i < 5; i++) {
        console.log(arr[i])
        if (arr[i] != 0) {
            return false;
        }
    }
    return true;
}

export function checkFixPrimitiveIntArray(arr: FixedArray<int>): boolean {
    if (arr.length != 5) {
        return false;
    }
    for (let i = 0; i < 5; i++) {
        console.log(arr[i])
        if (arr[i] != 0) {
            return false;
        }
    }
    return true;
}

export function checkFixPrimitiveLongArray(arr: FixedArray<long>): boolean {
    if (arr.length != 5) {
        return false;
    }
    for (let i = 0; i < 5; i++) {
        console.log(arr[i])
        if (arr[i] != 0) {
            return false;
        }
    }
    return true;
}

export function checkFixPrimitiveFloatArray(arr: FixedArray<float>): boolean {
    if (arr.length != 5) {
        return false;
    }
    for (let i = 0; i < 5; i++) {
        console.log(arr[i])
        if (arr[i] != 0.0) {
            return false;
        }
    }
    return true;
}

export function checkFixPrimitiveDoubleArray(arr: FixedArray<double>): boolean {
    if (arr.length != 5) {
        return false;
    }
    for (let i = 0; i < 5; i++) {
        console.log(arr[i])
        if (arr[i] != 0.0) {
            return false;
        }
    }
    return true;
}


function checkFixReferenceObjectArray(arr: FixedArray<Object>): boolean {
    if (arr.length != 3) {
        return false;
    }
    for (let i = 0; i < 3; i++) {
        if (!(arr[i] instanceof Int)) {
               return false;
        }
    }
    return true;
}

export function checkFixReferenceStringArray(arr: FixedArray<String>): boolean {
    console.log('checkFixReferenceBooleanArray:', arr)
    if (arr.length != 3) {
        return false;
    }
    for (let i = 0; i < 3; i++) {
        console.log('log res:', (arr[i] instanceof String), arr[i], arr);

        if (!(arr[i] instanceof String)) {
            return false;
        }
    }
    return true;
}

export function checkFixReferenceUndefinedArray(arr: FixedArray<Object>): boolean {
    if (arr.length != 3) {
        return false;
    }
    return true;
}