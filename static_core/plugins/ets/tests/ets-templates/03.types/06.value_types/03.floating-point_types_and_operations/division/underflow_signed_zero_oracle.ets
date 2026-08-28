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
desc: Underflow preserves the sign of an operation-produced signed zero.
test_unit: TEST-UNIT-03-0027
assertion: GAP-03-0038 / GRC-03-0059 / CI-03-06-03-027-02
---*/
function main(): void {
    let result: double = -Double.MIN_VALUE / 2.0;
    let reciprocal: double = 1.0 / result;
    arktest.assertEQ(result, 0.0, 'underflow operation must produce zero at this boundary');
    arktest.assertEQ(reciprocal, Double.NEGATIVE_INFINITY, 'underflow-produced zero must retain its negative sign');
}
