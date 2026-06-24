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
    Some of the changes that can make some compiled code not loadable:
    Changing Effective Type of a public or protected method or field.
files: ['./effective_type_class_method_protected_n.libr.ets']
tags: [negative]
negative_steps: [verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    verify-new-libs:
        - "E/verifier: Cannot link class: Cannot find method 'm
            (Leffective_type_class_method_protected_n/libr/A;)Leffective_type_class_method_protected_n/libr/A;'
            in class 'effective_type_class_method_protected_n.libr.A'"
        - "E/verifier: Cannot link class: Cannot find method 'm
            (Leffective_type_class_method_protected_n/libr/A;)Leffective_type_class_method_protected_n/libr/A;'
            in class 'effective_type_class_method_protected_n.libr.A'"
        - "E/verifier: Verifier error 54: Cache miss for method at offset"
        - "E/verifier: Verifier error 62: Cannot resolve method id"
        - "E/verifier: Error: method void
            effective_type_class_method_protected_n.C::check(effective_type_class_method_protected_n.C)
            failed to verify"
    runtime-with-new-libs.ark:
        - "E/verifier: Cannot link class: Cannot find method 'm
            (Leffective_type_class_method_protected_n/libr/A;)Leffective_type_class_method_protected_n/libr/A;'
            in class 'effective_type_class_method_protected_n.libr.A'"
        - "E/verifier: Cannot link class: Cannot find method 'm
            (Leffective_type_class_method_protected_n/libr/A;)Leffective_type_class_method_protected_n/libr/A;'
            in class 'effective_type_class_method_protected_n.libr.A'"
        - "E/verifier: Verifier error 54: Cache miss for method at offset"
        - "E/verifier: Verifier error 62: Cannot resolve method id"
        - "E/verifier: Method verification failed: void
            effective_type_class_method_protected_n.C::check(effective_type_class_method_protected_n.C)"
        - "E/verifier: Class verification failed: 'effective_type_class_method_protected_n.C' method:
            void effective_type_class_method_protected_n.C::check(effective_type_class_method_protected_n.C)"
        - "Unhandled exception: std.core.LinkerVerificationError"
        - "LinkerVerificationError: Leffective_type_class_method_protected_n/C;"
        - "  at std.core.LinkerError.<ctor> (RuntimeLinkerErrors.ets:20:0)"
        - "  at std.core.LinkerVerificationError.<ctor> (RuntimeLinkerErrors.ets:30:0)"
        - "  at effective_type_class_method_protected_n.ETSGLOBAL.main
            (effective_type_class_method_protected_n.ets:65:0)"
---*/

import { B } from './effective_type_class_method_protected_n.libr.ets'

class C extends B {
    check() {
        arktest.assertEQ(this.m(this), this)
        arktest.assertNE(this.m(new B), this)
    }
}

function main(): void {
    new C().check()
}
