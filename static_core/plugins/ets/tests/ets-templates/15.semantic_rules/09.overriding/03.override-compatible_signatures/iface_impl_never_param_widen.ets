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
    A class implementing an interface may widen a 'never' parameter to 'undefined' in
    its method definition (contravariant parameter rule).
tags: []
---*/

interface I {
    foo(p: never): int
}

class A implements I {
    foo(p: undefined): int { return 42 }
}

function main(): void {
    let a: A = new A()
    arktest.assertEQ(a.foo(undefined), 42)
}
