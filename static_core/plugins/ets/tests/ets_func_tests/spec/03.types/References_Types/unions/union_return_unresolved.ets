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
name: 03.types/08.reference_types/03.08.11.union_types/union_return_unresolved
tags: [negative]
negative_steps: [compile.verifier, verifier]
expected_error:
    verifier: 'E/verifier: Verifier error 2010: Cannot resolve method return type'
    verifier: 'E/verifier: Error: method {Ustd.core.Int,union_return_unresolved.NoImpl}
union_return_unresolved.classWithNoImplField::bar() failed to verify'
    compile.verifier: 'E/verifier: Error: method {Ustd.core.Int,union_return_unresolved.NoImpl}
union_return_unresolved.classWithNoImplField::bar() failed to verify'
---*/

declare class NoImpl {}

class classWithNoImplField {
  public static bar(): NoImpl | int {
    return 1;
  }
}

function main() {}
