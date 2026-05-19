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
desc: |-
    Value array type is the built-in type written as ValueArray<T> and characterized by the fact:
    Any instance of array type contains elements of type T. T must be a value type
    Test assertions:
    - Check override with wider array stored type
tags: [compile-only, negative]
expected_out: "Semantic error ESE0136: Method foo(p: ValueArray<Long>): ValueArray<Long> in A not overriding any method"
---*/

class B {
    foo(p: ValueArray<int>): ValueArray<long> {
        return [333]
    }
}

class A extends B {
    override foo(p: ValueArray<long>): ValueArray<long> {
        return [333]
    }

    bar(p: ValueArray<char>): int {
        return 0
    }
}
