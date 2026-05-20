/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: 05.Generics
name: 05.generics/genericTypeArgumentWithDefaultParameterValue_03
tags: []
---*/

class Helper {}

class A<T> {
    data: T
    isData: boolean
    constructor(data: T, isData: boolean) {
        this.data = data
        this.isData = isData
    }
}

class B<T> {
    data: T
    constructor(data: T) {
        this.data = data
    }
    foo<T>(data: T, isData: boolean): A<T> {
        return new A<T>(data, isData)
    }
}

function main(): void {
    let b = new B<String>("hello")
    let a = b.foo<Helper>(new Helper(), true);
    arktest.assertTrue( a instanceof A)
    arktest.assertEQ( a.isData, true)
    arktest.assertTrue( a.data instanceof Helper)
}

