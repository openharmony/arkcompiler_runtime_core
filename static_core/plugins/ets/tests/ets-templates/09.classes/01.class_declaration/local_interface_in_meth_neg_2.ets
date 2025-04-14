/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Local classes and interfaces are prohibited in ArkTS
tags: [compile-only, negative]
---*/

class A {
    meth(): int {
        interface LocalInterface {
            fld: int
        }

        class LocalClass implements LocalInterface {
            fld: int
        }

        let v: LocalClass = { fld: 0 }
        v.fld = 42
        return v.fld
    }
}

function main() {
    assertEQ(new A().meth(), 42)
}
