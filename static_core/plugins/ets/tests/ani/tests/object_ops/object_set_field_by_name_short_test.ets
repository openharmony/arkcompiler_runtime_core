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

class Animal {
    constructor(name: String, value: short) {
        this.name = name;
        this.value = value;
    }
    name: String;
    value: short;
}

function newAnimalObject() {
    let value:short = 32767;
    return new Animal("Cat", value);
}

function checkObjectField(animal: Animal,  value: short): boolean {
    return animal.value == value;
}
