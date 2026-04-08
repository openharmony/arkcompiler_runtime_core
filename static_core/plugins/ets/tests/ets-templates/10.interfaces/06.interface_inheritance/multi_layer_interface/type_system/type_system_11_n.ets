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
desc: Use generic interfaces and class implementations to verify the correctness of the type system.
tags: [compile-only, negative]
expected_out: >-
    Semantic error ESE985118: Class member value with type Int in MyStringBBox cannot implement value with type String property from interface BBox<T>
    because the implementing return type is not compatible with the interface return type.
---*/

interface BBox<T> {
    value: T;
}

interface StringBBox extends BBox<string> {}

class MyStringBBox implements StringBBox {
    value = 123;
}
