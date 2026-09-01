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
desc: "TEST-UNIT-09-0024 GAP-09-0024 GRC-09-0024 CI-09-07-008-01: Methods can return this for method chaining"
tags: [compile-only]
---*/

class Builder {
    private value: number = 0

    setValue(val: number): this {
        this.value = val
        return this
    }

    add(val: number): this {
        this.value += val
        return this
    }

    multiply(val: number): this {
        this.value *= val
        return this
    }

    getValue(): number {
        return this.value
    }
}

function main(): void {
    let result = new Builder()
        .setValue(10)
        .add(5)
        .multiply(2)
        .getValue()
    console.log(result)  // 30
}
