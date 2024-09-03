/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: 10 Interfaces
name: 10.interfaces/generic_interface1
tags: []
---*/

interface Interface1 {
    method1(): boolean
}

interface Interface2<T> {
    method1(): boolean
}

class Interface1Impl implements Interface1 {
    inner_method<T>(): Interface2Impl<T> {
        return new Interface2Impl<T>;
    }

    override method1(): boolean {
        const m = this.inner_method<int>();
        return m.method1();
    }
}

class Interface2Impl<T> implements Interface2<T> {
    override method1(): boolean {
        return true;
    }
}

function main(): void {
    let a = new Interface1Impl();
    assert a.method1() == true;
}
