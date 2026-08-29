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
desc: "TEST-UNIT-09-0020 GAP-09-0020 GRC-09-0020 CI-09-04-006-01: Static initializer executes when class is loaded"
tags: [compile-only]
---*/

class Config {
    public static initialized: boolean = false
    public static value: number

    static {
        console.log('Static initializer executing')
        Config.value = 42
        Config.initialized = true
    }
}

function main(): void {
    console.log(Config.initialized)
    console.log(Config.value)
}
