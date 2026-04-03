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
    If initializers for all members are omitted, then the first member is assigned the value zero.
    The other member is assigned the value of the immediately preceding member plus one.
    If an initializer is not a constant expression (see Constant Expressions),
    all subsequent members must be explicitly initialized.
    Otherwise, a compile-time error occurs.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0342: Type of enumeration constants must be any integer or string type or explicitly defined and they must be of the same type."
---*/

const c = 10

enum TestEnum { One = c + 10, Two}
