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
    A compile-time error occurs if one or more unexported entities in an exported overload.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0456: The overload function must be exported along because the overload alias is exported."
---*/

function foo(): void {};
export function bar(): void {};

// compile-time error, `foo` not exported
export overload baz { foo, bar }
