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
desc: Test assigning a Readonly<A> object to another Readonly<A> variable
tags: [compile-only, positive]
---*/

function foo(): string {
    return "private field value";
}

class A {
    public_field = 444;
    private private_field = foo();
}

function main(): void {
    let obj1: Readonly<A> = new A();
    let obj2: Readonly<A> = obj1;
    console.log(obj2);
}