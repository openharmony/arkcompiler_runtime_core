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
desc: "TEST-UNIT-09-0025 GAP-09-0025 GRC-09-0025 CI-09-08-002-01: Setter accessor must have exactly one parameter"
tags: [compile-only, negative]
expected_out: |-
    Syntax error ESY0063: Setter must have exactly one formal parameter.
---*/

class Data {
    private _value: number = 0

    get value(): number {
        return this._value
    }

    set value(v: number, extra: number) {  // Should fail: setter must have exactly one parameter
        this._value = v + extra
    }
}

function main(): void {
    let d = new Data()
}
