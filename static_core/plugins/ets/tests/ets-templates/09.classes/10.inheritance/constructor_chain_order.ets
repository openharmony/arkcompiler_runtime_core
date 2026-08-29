/*
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

/*---
desc: >-
  TEST-UNIT-09-0027 GAP-09-0027 GRC-09-0027 CI-09-10-005-01: Constructor chain
  execution order - parent before child
tags: [compile-only]
---*/

class Base {
    constructor() {
        console.log('Base constructor')
    }
}

class Middle extends Base {
    constructor() {
        super()
        console.log('Middle constructor')
    }
}

class Derived extends Middle {
    constructor() {
        super()
        console.log('Derived constructor')
    }
}

function main(): void {
    let d = new Derived()
    // Output should be: Base constructor, Middle constructor, Derived constructor
}
