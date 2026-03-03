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
desc: Pass incomplete object literal to Required<Readonly> type with array type public field, expect compile error
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0226: Class property 'a' needs to be initialized for Required<A>.
    Semantic error ESE0226: Class property 'b' needs to be initialized for Required<A>.
---*/

class A {
    public a: string[] = ["one", "two"];
    private b: boolean[] = [true];
}

function test(arg: Required<Readonly<A>>) {
}

function main(): void {
    test({});
}