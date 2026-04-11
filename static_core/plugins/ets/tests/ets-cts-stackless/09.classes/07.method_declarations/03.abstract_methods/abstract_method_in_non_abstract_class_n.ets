/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    Abstract method defined in non-abstract class.
    A compile-time error must be thrown when a class contains an abstract method but is not marked abstract.
tags: [negative, compile-only]
expected_out: "Semantic error ESE0019: Non abstract class has abstract method."
---*/

class B {
    foo(): void {};
}

class A extends B {
    abstract foo(): void;
}
