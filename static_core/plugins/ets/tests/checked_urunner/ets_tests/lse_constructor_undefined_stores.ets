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

//! CHECKER      Check LSE eliminates redundant undefined stores in constructor
//! RUN          force_jit: true, options: "--compiler-regex='.*MyClass.*'", entry: "lse_constructor_undefined_stores.ETSGLOBAL::main"
//! METHOD       "lse_constructor_undefined_stores.MyClass::<ctor>"
//! PASS_BEFORE  "LSE"
//! INST_COUNT   /StoreObject.*a/, 6
//! INST_COUNT   /StoreObject.*b/, 6
//! INST_COUNT   /StoreObject.*c/, 6
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*a/, 3
//! INST_COUNT   /StoreObject.*b/, 3
//! INST_COUNT   /StoreObject.*c/, 3

//! CHECKER      Check LSE eliminates redundant undefined stores in constructor (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*MyClass.*'"
//! METHOD       "lse_constructor_undefined_stores.MyClass::<ctor>"
//! PASS_BEFORE  "LSE"
//! INST_COUNT   /StoreObject.*a/, 6
//! INST_COUNT   /StoreObject.*b/, 6
//! INST_COUNT   /StoreObject.*c/, 6
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*a/, 3
//! INST_COUNT   /StoreObject.*b/, 3
//! INST_COUNT   /StoreObject.*c/, 3
//! RUN          entry: "lse_constructor_undefined_stores.ETSGLOBAL::main"

//! CHECKER      Check LSE eliminates redundant undefined stores in constructor (AOT-PGO)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "lse_constructor_undefined_stores.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex='.*MyClass.*'"
//! METHOD       "lse_constructor_undefined_stores.MyClass::<ctor>"
//! PASS_BEFORE  "LSE"
//! INST_COUNT   /StoreObject.*a/, 6
//! INST_COUNT   /StoreObject.*b/, 6
//! INST_COUNT   /StoreObject.*c/, 6
//! PASS_AFTER   "LSE"
//! INST_COUNT   /StoreObject.*a/, 3
//! INST_COUNT   /StoreObject.*b/, 3
//! INST_COUNT   /StoreObject.*c/, 3
//! RUN          entry: "lse_constructor_undefined_stores.ETSGLOBAL::main"

class A {
    value: int

    constructor(value: int) {
        this.value = value
    }
}

class MyClass {
    a: A
    b: A
    c: A

    constructor(aVal: A, bVal: A, cVal: A) {
        this.a = aVal
        this.b = bVal
        this.c = cVal
    }

    sum(): int {
        return this.a.value + this.b.value + this.c.value
    }
}

function main(): void {
    let obj = new MyClass(new A(1), new A(2), new A(3))
    arktest.assertEQ(obj.a.value, 1)
    arktest.assertEQ(obj.b.value, 2)
    arktest.assertEQ(obj.c.value, 3)
    arktest.assertEQ(obj.sum(), 6)
}
