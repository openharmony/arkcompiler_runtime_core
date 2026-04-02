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
desc: NonNullable with nested nullable classes - inner field cannot be null
---*/

class Inner {
    v: number
    constructor(v: number) {
        this.v = v
    }
}

class Outer {
    inner: NonNullable<Inner|null>
    constructor(inner: NonNullable<Inner|null>) {
        this.inner = inner
    }
}

function main() {

    let o = new Outer(new Inner(20))

    // nested type is Inner
    let i: Inner = o.inner

    // Nested access is safe since inner is NonNullable
    arktest.assertEQ(o.inner.v, 20)
}

