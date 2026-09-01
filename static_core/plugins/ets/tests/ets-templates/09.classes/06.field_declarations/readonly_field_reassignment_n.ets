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
  TEST-UNIT-09-0011 GAP-09-0011 GRC-09-0011 CI-09-06-002-01: Readonly field
  cannot be reassigned in an instance method
tags: [compile-only, negative]
expected_out: "Semantic error ESE4002: Cannot assign to a readonly field maxValue"
---*/

class Config {
    public readonly maxValue: number = 100

    updateMax(value: number): void {
        this.maxValue = value  // Should fail: readonly field cannot be reassigned
    }
}

function main(): void {
    let config = new Config()
    config.updateMax(300)
}
