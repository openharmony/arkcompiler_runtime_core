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
desc: Issue 17498. Non-void functions should have return statement on all control-flow paths
tags: [compile-only, negative]
---*/

class C<T> {
    public field: T;
}

function baz() {}
function boo() {}

function foo() {
    let ci = new C<undefined>()
    if(false) return ci.field;
    boo()
    baz()
    let flag: boolean = false

    if(flag) return ci.field;
}
