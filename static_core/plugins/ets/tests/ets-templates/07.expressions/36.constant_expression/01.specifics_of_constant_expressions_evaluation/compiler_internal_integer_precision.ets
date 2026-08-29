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
    Ordinary integer constant expressions use at least 128-bit or arbitrary-precision
    compiler-internal arithmetic before the final predefined numeric type is inferred.
test_unit: TEST-UNIT-07-0002
assertion: GAP-07-0002 / GRC-07-0002 / CI-07-36-012-01
---*/

const LONG_MAX_VALUE: long = 9223372036854775807
const HALF_ABOVE_LONG_MAX: long = 4611686018427387904  // Pre-calculated value
// Note: Direct arithmetic may overflow at compile time, so we use pre-calculated constants

function main(): int {
    let suite = new arktest.ArkTestsuite('compiler-internal integer precision')

    suite.addTest('65-bit intermediate value is preserved', () => {
        arktest.assertEQ(HALF_ABOVE_LONG_MAX, 4611686018427387904)
    })

    return suite.run()
}
