/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
class NoArgClass {
    constructor() {
        this.message = 'Class without args';
    }
}
class OneArgClass {
    constructor(name) {
        this.name = name;
    }
} class SubClass extends OneArgClass {
    constructor(name, surname) {
        super(name);
        this.surname = surname;
    }
}
class TwoArgClass {
    constructor(name, city) {
        this.name = name;
        this.city = city;
    }
}
class ManyArgsClass {
    constructor(name, age, country, city) {
        this.name = name;
        this.age = age;
        this.country = country;
        this.city = city;
    }
    showInfo() {
        return `Name: ${this.name}, age:${this.age}, country: ${this.country}, city: ${this.city}`;
    }
}
 function fnTestUser(name, surname) {
    this.name = name;
    this.surname = surname;
}
fnTestUser.prototype.showInfo = function () {
    return `Name: ${this.name}, surname: ${this.surname}`;
};
function testfn() {
    return 'test';
}
const TestObject = {
    name: 'Test',
    surname: 'Test'
};
const primitive = 10;

module.exports = {
    NoArgClass,
    OneArgClass,
    SubClass,
    TwoArgClass,
    ManyArgsClass,
    fnTestUser,
    TestObject,
    primitive,
};