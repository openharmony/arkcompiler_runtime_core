/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class User {
    public name: string;
    public age: number;
    public email: string;

    constructor(name: string, age: number, email: string) {
        this.name = name;
        this.age = age;
        this.email = email;
    }
}

class Employee {
    public id: number;
    public name: string;

    constructor(id: number, name: string) {
        this.id = id;
        this.name = name;
    }
}

class Address {
    public street: string;
    public city: string;
    public country: string;

    constructor(street: string, city: string, country: string) {
        this.street = street;
        this.city = city;
        this.country = country;
    }
}

class Company {
    public name: string;
    public employees: Employee[];
    public address: Address;

    constructor(name: string, employees: Employee[], address: Address) {
        this.name = name;
        this.employees = employees;
        this.address = address;
    }
}

class Calculator {
    add(a: number, b: number): number {
        return a + b;
    }

    subtract(a: number, b: number): number {
        return a - b;
    }

    multiply(a: number, b: number): number {
        return a * b;
    }
}

function main() {
    // Simple object
    const user = new User('John', 30, 'john@example.com');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.dir(user);

    // Nested object
    const company = new Company(
        'Tech Corp',
        [
            new Employee(1, 'Alice'),
            new Employee(2, 'Bob')
        ],
        new Address('123 Main St', 'Tech City', 'Techland')
    );
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.dir(company);

    // Object with methods
    const calculator = new Calculator();
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.dir(calculator);

    // Array with mixed types
    const mixedArray: Any[] = [
        1,
        'string',
        [1, 2, 3],
    ];
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.dir(mixedArray);

    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.dir()
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.dir(1);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.dir('str');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.dir('count: %d')
}
