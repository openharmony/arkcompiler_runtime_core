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
desc: Pass incomplete object literal to Required<Readonly> type with multiple public fields, expect compile error
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0226: Class property 'p' needs to be initialized for Required<A>.
    Semantic error ESE0226: Class property 'q' needs to be initialized for Required<A>.
---*/

class A {
    public o = 45;
    public p = 46;
    private q = 47;
}

function test(arg: Required<Readonly<A>>) {
}

function main(): void {
    test({ o: 48 });
}