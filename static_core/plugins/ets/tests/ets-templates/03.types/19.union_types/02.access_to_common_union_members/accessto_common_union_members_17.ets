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
desc: Accessing common members with inheritance.
---*/

class Base {
    tmp = 1;
    foo() {}
}

class Derived1 extends Base {
    tmp1 = "aa";
}

class Derived2 extends Base {
    tmp1 = 3.14;
}

function main(): void {
    let account: Derived1 | Derived2 = new Derived2();
    let num = account.tmp;
    assertTrue(num === 1);
}