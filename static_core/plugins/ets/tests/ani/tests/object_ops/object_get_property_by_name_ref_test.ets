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

class Person {
    constructor(name: String, surname: String, age: int, realAge: int) {
        this.name = name;
        this.__surname = surname;
        this.age = age;
        this.__realAge = realAge;
    }
    get surname(): String {
        return this.__surname;
    }
    get realAge(): int {
        return this.__realAge;
    }
    name: String;
    private __surname: String;
    age: int;
    private __realAge: int;
}

function newPersonObject() {
    return new Person("Max", "Pain", 34, 42);
}
