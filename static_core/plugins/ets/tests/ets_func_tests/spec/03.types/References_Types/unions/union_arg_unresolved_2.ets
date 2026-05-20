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
    03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_arg_unresolved_2
expected_error:
    verifier: 'E/verifier: Cannot link class: Cannot find class Lunion_arg_unresolved_2/NoImpl;'
    compile.verifier: 'E/verifier: Cannot link class: Cannot find class Lunion_arg_unresolved_2/NoImpl;'
    runtime.ark: 'E/verifier: Cannot link class: Cannot find class Lunion_arg_unresolved_2/NoImpl;'
    ark-with-jit.ark: 'E/verifier: Cannot link class: Cannot find class Lunion_arg_unresolved_2/NoImpl;'
    ark-after-aot.ark: 'E/verifier: Cannot link class: Cannot find class Lunion_arg_unresolved_2/NoImpl;'
    ark-with-jit-repeats.ark: 'E/verifier: Cannot link class: Cannot find class Lunion_arg_unresolved_2/NoImpl;'
    hybrid-ark.ark: 'E/verifier: Cannot link class: Cannot find class Lunion_arg_unresolved_2/NoImpl;'
---*/

declare class NoImpl {}

class classWithNoImplField {
  public static bar(a: NoImpl | int) {
    return 1;
  }
}

function main() {
  const klass = new classWithNoImplField();
}
