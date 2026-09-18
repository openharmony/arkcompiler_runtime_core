// Copyright (c) 2026 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

class Animal {
    constructor(name) {
        this.name = name;
    }

    hello() {
        return 'hi ' + this.name;
    }
}

class Dog extends Animal {
    constructor(name, age) {
        super(name);
        this.age = age;
    }

    hello() {
        return super.hello() + ' ' + this.age;
    }

    bark() {
        return this.name + '!';
    }
}

class Cat extends Animal {
    constructor(name) {
        super(name);
    }

    hello() {
        return super.hello() + '?';
    }
}

function greet(a) {
    return a.hello();
}

let d = new Dog('n', 3);
let c = new Cat('m');
console.log(greet(d), greet(c), d.bark(), d instanceof Animal);
