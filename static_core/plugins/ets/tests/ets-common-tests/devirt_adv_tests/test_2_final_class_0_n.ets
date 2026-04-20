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
  files: [./ext_module_final_class/ext_module.ets]
  tags: [negative]
  negative_steps: [verifier, runtime.ark, ark-with-jit.ark, ark-after-aot.ark, hybrid-ark.ark]
  expected_error:
      verifier: |-
        E/verifier: Verifier error 31: Bad call incompatible parameter
      runtime.ark: |-
        E/verifier: Verifier error 31: Bad call incompatible parameter
      ark-with-jit.ark: |-
        E/verifier: Verifier error 31: Bad call incompatible parameter
      ark-after-aot.ark: |-
        E/verifier: Verifier error 31: Bad call incompatible parameter
      hybrid-ark.ark: |-
        E/verifier: Verifier error 31: Bad call incompatible parameter
---*/

import { A, B } from './imported_hierarchy/ext_module.ets'

function test(a: A) { return a.foo(); }

function main(): int {
    arktest.assertEQ(test(new B()), 3);
    return 0;
}
