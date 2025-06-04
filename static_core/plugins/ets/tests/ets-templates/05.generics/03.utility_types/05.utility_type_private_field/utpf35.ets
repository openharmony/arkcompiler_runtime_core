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
desc: Pass partial object literal without private field to Partial<Readonly> type with class method
tags: [compile-only, positive]
---*/

class A {
    public a() { return 101; }
    public b = 102;
    private c = 103;
}

function test(arg: Partial<Readonly<A>>) {
}

function main(): void {
    test({ b: 104 });
}