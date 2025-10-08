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
        console.log('  ets log, Student Ctor--age:', this.age, '  name:', this.name);
        this.age = age;
        this.name = name;
    }

    checkFixPrimitiveArray(arr: FixedArray<boolean>): boolean {
        if (arr.length != 5) {
            return false;
        }
        for (let i = 0; i < 5; i++) {
            if (arr[i] != false) {
                return false;
            }
        }
        return true;
    }
    checkFixReferenceArray(arr: FixedArray<Int>): boolean {
        console.log('checkFixReferenceArray:', arr)
        if (arr.length != 3) {
            return false;
        }
        for (let i = 0; i < 3; i++) {
            console.log('log res:', arr[i] == 5, (arr[i] instanceof Int), arr[i], arr[i].unboxed(), arr)
            if (!(arr[i] instanceof Int)) {
                return false;
            }
        }
        return true;
    }
}

export class SubStudent extends Student {
    constructor() { super() }
    constructor(age: int) { super(age) }
    constructor(age: int, name: string) { super(age, name) }
}
