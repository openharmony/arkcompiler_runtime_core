/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Verify the correctness of the type system through complex generic interfaces and class implementations.
tags: [compile-only, negative]
expected_out: >-
    Semantic error ESE0141: first(): Int in StringNumberPair cannot override first():
    String in Pair<T,U> because overriding return type is not compatible with the other return type.
---*/

interface Pair<T, U> {
    first: T;
    second: U;
}

class StringNumberPair implements Pair<string, number> {
    first = 42;
    second = "Hello";
}
