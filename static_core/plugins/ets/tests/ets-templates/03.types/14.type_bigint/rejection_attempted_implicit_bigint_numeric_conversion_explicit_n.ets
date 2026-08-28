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
desc: An implicit bigint-to-numeric conversion is rejected while explicit conversion remains distinct.
test_unit: TEST-UNIT-03-0046
tags: [compile-only, negative]
assertion: GAP-03-0074 / GRC-03-0100 / CI-03-14-003-01
expected_out: |-
    Semantic error ESE0318: Type 'BigInt' cannot be assigned to type 'Int'
---*/

function main(): void {
    let source: bigint = 42n
    let rejected: int = source
}
