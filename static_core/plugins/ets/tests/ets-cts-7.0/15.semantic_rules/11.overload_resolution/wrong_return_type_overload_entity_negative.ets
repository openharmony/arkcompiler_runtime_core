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
desc: Return types of overload entities are not considered by overload resolution.
tags: [negative, compile-only]
expected_out: "Semantic error ESE0318: Type 'String' cannot be assigned to type 'Double'"
---*/

function foo(n: number): number { return n }
function foo(s: string): string { return s }

let x: number = foo("1")
