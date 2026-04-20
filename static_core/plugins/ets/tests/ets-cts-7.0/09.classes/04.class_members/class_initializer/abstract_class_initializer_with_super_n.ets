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
desc: >-
      In addition, a compile-time error occurs if a class initializer contains the following:
      - Keyword super (see Method Call Expression and Field Access Expression);
      Abstract class case.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0202: 'super' cannot be referenced from a static context"
---*/
<<<<<<<< HEAD:static_core/plugins/ets/tests/ets-cts-7.0/09.classes/04.class_members/class_initializer/abstract_class_initializer_with_super_n.ets
========

>>>>>>>> 4c9c00f9f0 (Description: Fix u-runner verifier files chain):static_core/plugins/ets/tests/ets-cts-7.0/23.binary_compatibility/01.classification_of_source_code_changes/class_and_interface_declaration/ambient_decl_not_loadable/interface_declarations.ets

abstract class A {
  public foo(): int {
    return 5;
  }
}

abstract class B extends A {
  public static x: int;

  static {
    x = 5 + super.foo();
  }
}
<<<<<<<< HEAD:static_core/plugins/ets/tests/ets-cts-7.0/09.classes/04.class_members/class_initializer/abstract_class_initializer_with_super_n.ets
========

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
>>>>>>>> 4c9c00f9f0 (Description: Fix u-runner verifier files chain):static_core/plugins/ets/tests/ets-cts-7.0/23.binary_compatibility/01.classification_of_source_code_changes/class_and_interface_declaration/ambient_decl_not_loadable/interface_declarations.ets
