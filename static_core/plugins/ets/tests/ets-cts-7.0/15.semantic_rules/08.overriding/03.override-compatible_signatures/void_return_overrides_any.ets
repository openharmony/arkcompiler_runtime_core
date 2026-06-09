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
    An overriding method may return void when the overridden method returns Any,
    because void is an alias for undefined and Any is a supertype of undefined.
    A call through the base type dispatches to the overriding method and returns undefined.
tags: []
---*/

class A {
    m(): Any {
        throw new Error("base method should not be called")
    }
}

class B extends A {
    m(): void {}
}

function callM(a: A): Any {
    return a.m()
}

function main(): void {
    arktest.assertEQ(callM(new B()), undefined)
}
