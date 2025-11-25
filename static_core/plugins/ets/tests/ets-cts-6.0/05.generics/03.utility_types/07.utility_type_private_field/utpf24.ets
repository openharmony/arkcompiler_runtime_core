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
desc: Pass partial object literal with private field to Partial<Readonly> type with different public field types, expect compile error
tags:
- negative
- compile-only
---*/

class A {
    public d: string = "g";
    public e: number = 60;
    private f: boolean = false;
}

function test(arg: Partial<Readonly<A>>) {
}

function main(): void {
    test({ d: "h", f: true });
}