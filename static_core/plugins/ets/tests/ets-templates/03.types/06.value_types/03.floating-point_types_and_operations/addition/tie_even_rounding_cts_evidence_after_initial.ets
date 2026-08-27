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
desc: Ties round to the even representable result for both positive and negative half-way values.
test_unit: TEST-UNIT-03-0026
assertion: GAP-03-0037 / GRC-03-0058 / CI-03-06-03-024-01
---*/
// TEST-UNIT-03-0026
function main(): int {
    let suite = new arktest.ArkTestsuite('tie to even rounding');
    suite.addTest('positive_tie_even', () => {
        let rounded: float = 1.0f + 0.000000059604644775390625f;
        arktest.assertDoubleEQ(rounded, 1.0f, 0.0, 'positive float operation tie rounds to even');
    });
    suite.addTest('negative_tie_even', () => {
        let rounded: float = -1.0f - 0.000000059604644775390625f;
        arktest.assertDoubleEQ(rounded, -1.0f, 0.0, 'negative float operation tie rounds to even');
    });
    return suite.run();
}
