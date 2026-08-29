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
  TEST-UNIT-09-0021 GAP-09-0021 GRC-09-0021 CI-09-06-001-01: Static fields
  belong to class, instance fields belong to object
tags: [compile-only]
---*/

class Counter {
    public static totalCount: number = 0
    public instanceCount: number = 0

    constructor() {
        Counter.totalCount++
        this.instanceCount++
    }
}

function main(): void {
    let c1 = new Counter()
    let c2 = new Counter()
    console.log(Counter.totalCount)  // 2
    console.log(c1.instanceCount)    // 1
    console.log(c2.instanceCount)    // 1
}
