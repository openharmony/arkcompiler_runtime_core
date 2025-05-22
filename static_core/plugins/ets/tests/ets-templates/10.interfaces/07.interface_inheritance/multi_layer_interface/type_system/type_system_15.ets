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
desc: Verify the correctness of the type system through complex generic interfaces and class implementations.
---*/

interface A<T, U> {
    a: T;
    b: U;
}

class B implements A<string, Boolean> {
    a: string = "B";
    b: Boolean = true;
}

class C extends B {
    a: string = "C";
    b: Boolean = false;
}

function main(): void {
    const b: A<string, Boolean> = new B();
    assertEQ(b.a, "B");
    assertEQ(b.b, true);

    const c: B = new C();
    assertEQ(c.a, "C");
    assertEQ(c.b, false);
}