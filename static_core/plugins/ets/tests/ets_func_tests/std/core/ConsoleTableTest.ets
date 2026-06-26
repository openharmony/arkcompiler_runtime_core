/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Matrix type definitions remain as types
type MatrixRow = [string, string, string];
type Matrix = MatrixRow[];

// Basic user class
class User {
    id: number;
    name: string;
    age: number;

    constructor(id: number, name: string, age: number) {
        this.id = id;
        this.name = name;
        this.age = age;
    }
}

// Product class
class Product {
    id: number;
    name: string;
    price: number;
    stock: number;

    constructor(id: number, name: string, price: number, stock: number) {
        this.id = id;
        this.name = name;
        this.price = price;
        this.stock = stock;
    }
}

// Mixed data class
class MixedData {
    id: number;
    value: string | number | boolean;

    constructor(id: number, value: string | number | boolean) {
        this.id = id;
        this.value = value;
    }
}

// Nested data classes
class Contact {
    email: string;

    constructor(email: string) {
        this.email = email;
    }
}

class Info {
    name: string;
    contact: Contact;

    constructor(name: string, contact: Contact) {
        this.name = name;
        this.contact = contact;
    }
}

class NestedData {
    id: number;
    info: Info;

    constructor(id: number, info: Info) {
        this.id = id;
        this.info = info;
    }
}

// Object with string/number/boolean scalar fields.
class Item {
    name: string;
    price: number;
    inStock: boolean;

    constructor(name: string, price: number, inStock: boolean) {
        this.name = name;
        this.price = price;
        this.inStock = inStock;
    }
}

// Nested classes to exercise sub-column expansion of a single object
class Address {
    city: string;
    zip: number;

    constructor(city: string, zip: number) {
        this.city = city;
        this.zip = zip;
    }
}

class Person {
    id: number;
    addr: Address;

    constructor(id: number, addr: Address) {
        this.id = id;
        this.addr = addr;
    }
}

// Numeric enum for testing
enum Priority {
    Low,
    Medium,
    High
}

// String enum for testing
enum Direction {
    North = 'N',
    South = 'S',
    East = 'E',
    West = 'W'
}

// Class with many fields for wide table testing
class WideRecord {
    id: number;
    name: string;
    age: number;
    city: string;
    active: boolean;
    score: number;
    level: number;

    constructor(id: number, name: string, age: number, city: string, active: boolean, score: number, level: number) {
        this.id = id;
        this.name = name;
        this.age = age;
        this.city = city;
        this.active = active;
        this.score = score;
        this.level = level;
    }
}

// Data display utility class
class DataDisplay {
    static displayUsers(): void {
        const users : Array<Any> = [
            new User(1, 'John', 30),
            new User(2, 'Alice', 25),
            new User(3, 'Bob', 35)
        ];
        console.table(users);
    }

    static displayMatrix(): void {
        const matrix: Matrix = [
            ['A1', 'B1', 'C1'],
            ['A2', 'B2', 'C2'],
            ['A3', 'B3', 'C3']
        ];
        console.table(matrix as Object as Array<Any>);
    }

    static displayProducts(): void {
        const products : Array<Any> = [
            new Product(1, 'Phone', 699, 50),
            new Product(2, 'Laptop', 999, 30),
            new Product(3, 'Tablet', 399, 75)
        ];
        console.table(products);
    }

    static displayMixedData(): void {
        const mixedData : Array<Any> = [
            new MixedData(1, 'string'),
            new MixedData(2, 42),
            new MixedData(3, true)
        ];
        console.table(mixedData);
    }

    // Single object carrying string/number/boolean scalar fields.
    static displaySingleObject(): void {
        console.table(new Item('Pen', 12, true));
    }

    // Array of mixed primitive element types.
    static displayMixedPrimitiveArray(): void {
        const arr: Array<Any> = [true, 1, 'text'];
        console.table(arr);
    }

    // Array containing null/undefined elements.
    static displayArrayWithNulls(): void {
        const arr: Array<Any> = [1, null, 3];
        console.table(arr);
    }

    // Array containing both undefined and null elements.
    static displayArrayWithUndefinedAndNull(): void {
        const arr: Array<Any> = [1, 2, undefined, null];
        console.table(arr);
    }

    // Single object with a nested class field (sub-column expansion + Values column).
    static displayNestedObject(): void {
        console.table(new Person(1, new Address('Shanghai', 200000)));
    }

    // Nested arrays (2D array of arrays)
    static displayNestedArray(): void {
        const nested: Array<Any> = [[1, 2], [3, 4], [5, 6]];
        console.table(nested);
    }

    // Deeply nested arrays (3 levels)
    static displayDeeplyNestedArray(): void {
        const deep: Array<Any> = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]];
        console.table(deep);
    }

    // Numeric enum values in array
    static displayNumericEnumArray(): void {
        const arr: Array<Any> = [Priority.Low, Priority.Medium, Priority.High];
        console.table(arr);
    }

    // String enum values in array
    static displayStringEnumArray(): void {
        const arr: Array<Any> = [Direction.North, Direction.South, Direction.East, Direction.West];
        console.table(arr);
    }

    // Single numeric enum value
    static displaySingleEnum(): void {
        console.table(Priority.High);
    }

    // Empty array
    static displayEmptyArray(): void {
        const arr: Array<Any> = [];
        console.table(arr);
    }

    // Boolean array
    static displayBooleanArray(): void {
        const arr: Array<Any> = [true, false, true, false];
        console.table(arr);
    }

    // String array
    static displayStringArray(): void {
        const arr: Array<Any> = ['apple', 'banana', 'cherry'];
        console.table(arr);
    }

    // Special numeric values: Infinity, -Infinity, NaN, 0, -0
    static displaySpecialNumbers(): void {
        const arr: Array<Any> = [Infinity, -Infinity, NaN, 0, -0];
        console.table(arr);
    }

    // Array of objects with nested objects (3 levels deep)
    static displayArrayOfNestedObjects(): void {
        const arr: Array<Any> = [
            new NestedData(1, new Info('Alice', new Contact('alice@test.com'))),
            new NestedData(2, new Info('Bob', new Contact('bob@test.com')))
        ];
        console.table(arr);
    }

    // Single wide object (many fields)
    static displayWideObject(): void {
        console.table(new WideRecord(1, 'Test', 30, 'Shanghai', true, 95.5, 5));
    }

    // Array of wide objects
    static displayWideObjectArray(): void {
        const arr: Array<Any> = [
            new WideRecord(1, 'Alice', 25, 'Beijing', true, 90, 3),
            new WideRecord(2, 'Bob', 30, 'Shanghai', false, 85, 2)
        ];
        console.table(arr);
    }

    // Jagged array (rows of different lengths)
    static displayJaggedArray(): void {
        const jagged: Array<Any> = [[1, 2, 3], [4, 5], [6]];
        console.table(jagged);
    }

    // Single element array
    static displaySingleElementArray(): void {
        const arr: Array<Any> = [42];
        console.table(arr);
    }

    // Large array (stress test)
    static displayLargeArray(): void {
        const arr: Array<Any> = [];
        for (let i = 0; i < 15; i++) {
            arr.push(i * 10);
        }
        console.table(arr);
    }

    // Mixed content array (primitives, null, undefined)
    static displayMixedContentArray(): void {
        const arr: Array<Any> = [1, 'text', true, null, undefined, 3.14];
        console.table(arr);
    }
}

interface A {
    a: number[],
    b: number,
    c: B,
}
interface B {
    e: number,
}

function main() {
    // Display simple array of objects
    DataDisplay.displayUsers();

    // Display array of arrays
    DataDisplay.displayMatrix();

    // Display array with specific columns
    DataDisplay.displayProducts();

    // Display mixed data types
    DataDisplay.displayMixedData();

    console.table(42)
    console.table('str')
    console.table('count %d')

    let a: A = {
        a: [1, 2, 3, 4, 5], b: 5,
        c: {
        e: 5
        }
    }
    console.table(a);
        
    let number1: number = 1;
    let str = 'str';
    console.table(number1);
    console.table(str);
    console.table('count: %d');
    console.table();
    console.table([1, 2, 3]);

    console.table(null);
    console.table(undefined);

    // Single object with string/number/boolean scalar fields.
    DataDisplay.displaySingleObject();

    // Array of mixed primitive element types.
    DataDisplay.displayMixedPrimitiveArray();

    // Array containing a null element.
    DataDisplay.displayArrayWithNulls();

    // Array containing both undefined and null elements.
    DataDisplay.displayArrayWithUndefinedAndNull();

    // Single object with a nested class field.
    DataDisplay.displayNestedObject();

    // Nested arrays (2D)
    DataDisplay.displayNestedArray();

    // Deeply nested arrays (3 levels)
    DataDisplay.displayDeeplyNestedArray();

    // Numeric enum values in array
    DataDisplay.displayNumericEnumArray();

    // String enum values in array
    DataDisplay.displayStringEnumArray();

    // Single enum value
    DataDisplay.displaySingleEnum();

    // Empty array
    DataDisplay.displayEmptyArray();

    // Boolean array
    DataDisplay.displayBooleanArray();

    // String array
    DataDisplay.displayStringArray();

    // Special numeric values
    DataDisplay.displaySpecialNumbers();

    // Array of objects with nested objects
    DataDisplay.displayArrayOfNestedObjects();

    // Single wide object (many fields)
    DataDisplay.displayWideObject();

    // Array of wide objects
    DataDisplay.displayWideObjectArray();

    // Jagged array
    DataDisplay.displayJaggedArray();

    // Single element array
    DataDisplay.displaySingleElementArray();

    // Large array
    DataDisplay.displayLargeArray();

    // Mixed content array
    DataDisplay.displayMixedContentArray();
}
