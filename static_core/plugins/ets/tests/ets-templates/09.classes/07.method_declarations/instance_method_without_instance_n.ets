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
desc: "TEST-UNIT-09-0013 GAP-09-0013 GRC-09-0013 CI-09-07-002-01: Instance methods require object instance"
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0208: 'increment' is an instance property of 'Counter'
---*/

class Counter {
    private count: number = 0

    increment(): void {
        this.count++
    }

    getCount(): number {
        return this.count
    }
}

function main(): void {
    Counter.increment()  // Should fail: instance method called without instance
}
