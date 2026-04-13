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
desc: For a given class, the overload set for constructors is formed from implicitly 
      overloaded constructors and from constructors listed in Explicit Constructor Overload.
---*/

class C {
    private str: string = ""
    public getStr() { return this.str }

    constructor() { this.str = "default constructor" }
    constructor(s: string) { this.str = "constructor from string" }
    constructor fromNumber(n: number) { this.str = "constructor from number" }
    overload constructor { fromNumber }
}

function main() {
    let c1 = new C();
    let c2: C = new C("from string");
    let c3 = new C(1);

    arktest.assertEQ(c1.getStr(), "default constructor");
    arktest.assertEQ(c2.getStr(), "constructor from string");
    arktest.assertEQ(c3.getStr(), "constructor from number");
}
