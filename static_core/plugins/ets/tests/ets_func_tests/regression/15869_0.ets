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
desc: Issue 15869. Optional arguments in interface methods
tags: []
---*/

type TF = (value?: int) => void

type ui = int | undefined

interface IFoo {
    foo(bar: (value?: int) => void): void {
    }
    boo(bar: (value: string|undefined) => void) {
    }
    baz(bar: (value: ui) => void): void {
    }
    ttf(bar: TF): void {
    }
}

class MyClass implements IFoo {
}

function main() {
    let mc: MyClass = new MyClass();
    mc.foo(() => {})
    mc.boo(() => {})
    mc.baz(() => {})
    mc.ttf(() => {})
}
