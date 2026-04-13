/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Supertype relation among array types
tags: [compile-only, negative]
---*/
<<<<<<<< HEAD:static_core/plugins/ets/tests/ets-cts-7.0/15.semantic_rules/02.subtyping/subtyping_among_array_types/NotASubtype0_n.ets
========

>>>>>>>> 4c9c00f9f0 (Description: Fix u-runner verifier files chain):static_core/plugins/ets/tests/ets-templates/23.binary_compatibility/01.classification_of_source_code_changes/class_and_interface_declaration/ambient_decl_not_loadable/interface_declarations.ets


class Base {}
class Derived extends Base {}

function bad(ab: Array<Base>, ad: Array<Derived>) {
    ab = ad  // TypeError: Type 'Array<Derived>' cannot be assigned to type 'Array<Base>'
}
<<<<<<<< HEAD:static_core/plugins/ets/tests/ets-cts-7.0/15.semantic_rules/02.subtyping/subtyping_among_array_types/NotASubtype0_n.ets
========

export interface C {
    foo(x: number, y: string): string
}

export interface D<T1> {
    a: T1
}

export interface E<U, R> {
    foo(x: U): R
}

export interface F {
    f: number
}

export interface M extends F {
}

export interface K {
    a: string
    get foo(): string
    set foo(v: string)
}

export interface J {}
>>>>>>>> 4c9c00f9f0 (Description: Fix u-runner verifier files chain):static_core/plugins/ets/tests/ets-templates/23.binary_compatibility/01.classification_of_source_code_changes/class_and_interface_declaration/ambient_decl_not_loadable/interface_declarations.ets
