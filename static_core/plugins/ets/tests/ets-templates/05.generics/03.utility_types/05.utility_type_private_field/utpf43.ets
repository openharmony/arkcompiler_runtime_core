/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Pass object literal with private field to Required<Readonly> type with optional public fields, expect compile error
tags:
- negative
- compile-only
---*/

class A {
    public a?: boolean;
    public b: number = 6;
    private c: number = 7;
}

function test(arg: Required<Readonly<A>>) {
}

function main(): void {
    test({ b: 8, c: 9, a: true });
}