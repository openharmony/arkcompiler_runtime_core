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
  TEST-UNIT-09-0030 GAP-09-0030 GRC-09-0030 CI-09-05-002-01: Protected
  members accessible in subclass but not outside
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0293: Property value is not visible here.
---*/

class Base {
    protected value: number = 100
}

class Derived extends Base {
    getValue(): number {
        return this.value  // OK: accessible in subclass
    }
}

function main(): void {
    let d = new Derived()
    console.log(d.getValue())  // OK
    console.log(d.value)  // Should fail: protected member not accessible outside
}
