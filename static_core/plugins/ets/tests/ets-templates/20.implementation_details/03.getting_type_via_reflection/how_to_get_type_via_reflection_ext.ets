/*
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

{% for c in cases %}

/*---
desc: {{c.desc}}
---*/

class User {
    name: string;
    constructor(name: string) {
        this.name = name;
    }
}

class ExtendedUser extends User {
    age: int;
    constructor(name: string, age: int) {
        super(name);
        this.age = age;
    }
}

enum Color {
    red = 0,
    green = 1,
    blue = 2
}

interface SomeInterface {
    getAge(): string;
}

class ExtendedUserImplementedInterface extends User implements SomeInterface {
    age: int;
    constructor(name: string, age: int) {
        super(name);
        this.age = age;
    }
    getAge() {
        return this.age + ' years';
    }
}

type NullableUser = User | null;

type NullableObject = Object | null | undefined;

type TypeAlias = (x: number, y: number) => number;

type UType = 'One' | 'Two';

function main(): void {
  {{c.logic}}
}

{% endfor %}
