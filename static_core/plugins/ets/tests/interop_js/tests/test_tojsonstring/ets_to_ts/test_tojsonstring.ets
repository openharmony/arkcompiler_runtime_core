/**
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

// Test class with primitive fields
class SimpleClass {
    public name: string = 'test';
    public age: number = 25;
    public active: boolean = true;
}

// Serialize SimpleClass using JSON.stringify for verification
function serializeSimpleClass(obj: SimpleClass): string {
    return JSON.stringify(obj);
}

// Test class with nested object
class Address {
    public city: string = '';
    public street: string = '';
    public zipCode: number = 0;
}

// Test class with array field
class ClassWithArray {
    public id: number = 0;
    public items: Array<number> = [];
    public tags: Array<string> = [];
}

// Serialize ClassWithArray using JSON.stringify for verification
function serializeClassWithArray(obj: ClassWithArray): string {
    return JSON.stringify(obj);
}

// Test class with null/undefined fields
class ClassWithNullFields {
    public name: string | null = null;
    public value: number | null = null;
    public flag: boolean = true;
}

// Serialize ClassWithNullFields using JSON.stringify for verification
function serializeClassWithNullFields(obj: ClassWithNullFields): string {
    return JSON.stringify(obj);
}

// Test empty class
class EmptyClass {
}

// Serialize EmptyClass using JSON.stringify for verification
function serializeEmptyClass(obj: EmptyClass): string {
    return JSON.stringify(obj);
}

// Test class with special characters in string
class ClassWithSpecialChars {
    public quote: string = '';
    public newLine: string = '';
    public unicode: string = '';
}

// Serialize ClassWithSpecialChars using JSON.stringify for verification
function serializeClassWithSpecialChars(obj: ClassWithSpecialChars): string {
    return JSON.stringify(obj);
}

// Test class with number edge cases
class ClassWithNumberEdgeCases {
    public zero: number = 0;
    public negative: number = -1;
    public num: number = 3.14;
    public large: number = 999999;
}

// Serialize ClassWithNumberEdgeCases using JSON.stringify for verification
function serializeClassWithNumberEdgeCases(obj: ClassWithNumberEdgeCases): string {
    return JSON.stringify(obj);
}

// Test class with complex nested structure
class Task {
    public id: number = 0;
    public title: string = '';
    public status: string = '';
    public hours: number = 0;
}

class Project {
    public name: string = '';
    public budget: number = 0;
    public active: boolean = true;
    public tasks: Array<Task> = [];
}

// Serialize Project using JSON.stringify for verification
function serializeProject(obj: Project): string {
    return JSON.stringify(obj);
}
