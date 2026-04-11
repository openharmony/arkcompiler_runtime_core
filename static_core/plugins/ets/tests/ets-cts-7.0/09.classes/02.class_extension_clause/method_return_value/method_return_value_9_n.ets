/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    The return value of the parent class method is number, and the return value of the subclass method is number [].
    An error is reported when the subclass recompiles the parent class method.
tags: [negative, compile-only]
expected_out: >-
    Semantic error ESE0141: Class member getValue with type Array<Double> in Child cannot override getValue with type Double field from class Parent
    because the overriding return type is not compatible with the other return type.
---*/

class Parent {
    public getValue(): number {
        return 100;
    }
}

class Child extends Parent {
    public getValue(): number[] {
        let a :number[] = [1, 2, 3];
        return a;
    }
}