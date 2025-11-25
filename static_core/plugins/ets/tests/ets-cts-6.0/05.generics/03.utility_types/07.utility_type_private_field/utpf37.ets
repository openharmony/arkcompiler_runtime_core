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
desc: Pass incomplete object literal to Required<Readonly> type with class method, expect compile error
tags:
- negative
- compile-only
---*/

class Waiter {
    public name = "Alice";
    private experienceYears = 3;
    public serve() {
        return `${this.name} is serving you.`;
    }
}

function hireWaiter(w: Required<Readonly<Waiter>>) {
    console.log(w.serve());
}

function main(): void {
    hireWaiter({ name: "Bob" });
}