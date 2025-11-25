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
desc: Generic named type
---*/

class A<T> {}
class C<T> {
    field1: A<T>
    field2: A<number>
    foo(p: T): T { return p }
    constructor(p1: A<T>, p2: A<number>) {
        this.field1 = p1
        this.field2 = p2
    }
}

let x: C<int> = new C<int>(new A<int>(), new A<number>())
arktest.assertTrue(x instanceof C<int>)
arktest.assertTrue(x.field1 instanceof A<int>)
arktest.assertTrue(x.field2 instanceof A<number>)
arktest.assertTrue(x.foo(1) instanceof int)
