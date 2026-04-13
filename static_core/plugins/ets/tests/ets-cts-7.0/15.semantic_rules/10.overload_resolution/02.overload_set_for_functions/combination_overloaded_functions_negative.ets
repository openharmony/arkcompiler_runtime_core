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
desc: If an overload set is a combination of implicitly and explicitly overloaded entities,
      then an explicit overload list must contain the name of an implicitly overloaded entity.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0397: Method with the same name as overload declaration 'foo', overloadlist must list this medhod."
---*/

function fooN(n: number): string { return 'fooN' }

function foo(): string { return 'foo1' }

function fooX(x?: number): string { return 'fooX' }

overload foo { fooN, fooX }
