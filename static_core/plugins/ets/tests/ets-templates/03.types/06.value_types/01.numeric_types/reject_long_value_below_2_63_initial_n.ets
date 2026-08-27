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
desc: An integer literal below the signed long lower bound is rejected.
test_unit: TEST-UNIT-03-0011
tags: [compile-only, negative]
expected_out: 'Syntax error ESY0249: Invalid number.'
assertion: GAP-03-0020 / GRC-03-0037 / CI-03-06-02-004-04
---*/
// TEST-UNIT-03-0011
function main(): void {
    let belowLowerBound: long = -9223372036854775809;
}
