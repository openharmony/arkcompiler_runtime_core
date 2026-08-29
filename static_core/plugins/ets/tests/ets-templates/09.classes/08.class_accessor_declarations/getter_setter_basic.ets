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
desc: "TEST-UNIT-09-0014 GAP-09-0014 GRC-09-0014 CI-09-08-001-01: Getter accessor provides read access"
tags: [compile-only]
---*/

class Temperature {
    private celsius: number

    constructor(celsius: number) {
        this.celsius = celsius
    }

    get fahrenheit(): number {
        return this.celsius * 9 / 5 + 32
    }

    set fahrenheit(value: number) {
        this.celsius = (value - 32) * 5 / 9
    }
}

function main(): void {
    let temp = new Temperature(25)
    console.log(temp.fahrenheit)
    temp.fahrenheit = 77
    console.log(temp.fahrenheit)
}
