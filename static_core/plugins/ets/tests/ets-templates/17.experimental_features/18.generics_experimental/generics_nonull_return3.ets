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
desc: Non-mandatory return of a non-empty generic instance, Using an inherited union type object | null.
      Using object as generic type.
---*/

class A {
    value: string;
    constructor(value: string) {
        this.value = value;
    }
}

class B<T extends A | null> {
    foo(p: T): T {
        return p;
    }
}

function main(): void {
    let b = new B<A>();
    let result = b.foo(new A("test"));
    assertEQ(result !== null, true);
}