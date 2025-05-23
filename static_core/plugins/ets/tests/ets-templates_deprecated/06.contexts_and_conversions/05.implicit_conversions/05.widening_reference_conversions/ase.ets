/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    This array assignment can cause ArrayStoreError at runtime if an object
    of incorrect type is included in the array.
---*/

class Base {}
class Derived extends Base {}
class AnotherDerived extends Base {}

function bar(): Base {
    return new AnotherDerived()
}

function foo(da: Base[]): int {
    try {
        da[0] = bar()
    } catch (e: ArrayStoreError) {
        return 0
    }
    return 1
}

function main(): int {
    let da: Derived[] = [new Derived(), new Derived(), new Derived()]
    return foo(da)
}
