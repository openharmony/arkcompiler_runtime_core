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

class Box {
    #val;
    static #count = 0;

    constructor(v) {
        this.#val = v;
        Box.#count++;
    }

    get value() {
        return this.#val;
    }

    set value(v) {
        this.#val = v;
    }

    add(n) {
        this.#val += n;
        return this.#val;
    }

    static size() {
        return Box.#count;
    }
}

let a = new Box(1);
let b = new Box(2);
a.value = 4;
console.log(a.add(3), b.value, Box.size());
