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
    Test for binary compatibility. Adding new public method to base class
    may break the binary compatibility when a derived class has overloaded
    methods with the same name.
files: ['./add_method_to_base_class_with_conflict_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs: |-
        TODO: Expected `Multiple override` error message that multiple methods in C2 override a method in C1
    runtime-with-new-libs.ark: |-
        TODO: Expected `Multiple override` error message that multiple methods in C2 override a method in C1
---*/

import { A, B, C1 } from './add_method_to_base_class_with_conflict_n.libr.ets'

class C2 extends C1 {
    m(p: B): string {
        return "C2.B"
    }

    m(p: A): string {
        return "C2.A"
    }
}

((): C1 => new C2)().m(new B)
