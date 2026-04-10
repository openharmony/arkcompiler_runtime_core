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
    A method whose parent declares 'never' as its return type cannot be overridden with
    a non-'never' return type ('never' is the bottom type and cannot be widened back).
tags: [negative, compile-only]
expected_out: |-
    Semantic error ESE0141: Class member foo with type String in B cannot override foo with type never field from class A because the overriding return type is not compatible with the other return type.
---*/

class A {
    foo(): never { throw new Error() }
}

class B extends A {
    override foo(): string { return "b" }
}
