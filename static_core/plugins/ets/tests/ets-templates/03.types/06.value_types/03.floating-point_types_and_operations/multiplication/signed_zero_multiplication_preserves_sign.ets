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
desc: IEEE-754 signed-zero multiplication is accepted for a non-remainder operator.
test_unit: TEST-UNIT-03-0023
tags: []
assertion: GAP-03-0034 / GRC-03-0055 / CI-03-06-03-020-01
---*/
function main(): void {
    let left: double = -0.0;
    let right: double = 2.0;
    let result: double = left * right;
    arktest.assertDoubleEQ(result, -0.0, 0.0, 'signed-zero multiplication');
    arktest.assertLT(1.0 / result, 0.0, 'signed-zero sign must be preserved')
}
