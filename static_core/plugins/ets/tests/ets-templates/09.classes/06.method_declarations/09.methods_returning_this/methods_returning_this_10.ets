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
desc: several methods returning this, overload, call several times, positive.
tags: []
---*/
class C {
    foo(): this {
        return this
    }
    foo2(a:int, b:object): this {
        return this
    }
}

function main(): void {
    let x: C = new C()
    x=x.foo().foo2(1, new object).foo().foo2(2, new object).foo().foo2(3, new object)
}
