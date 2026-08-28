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
desc: A declared local of a non-defaultable reference type receives no supplied default value.
test_unit: TEST-UNIT-03-0092
tags: [compile-only, negative]
assertion: GAP-03-0124 / GRC-03-0161 / CI-03-22-002-01
expected_out: 'Semantic error ESE0375: Variable ''value'' is used before being assigned.'
---*/

function main(): void {
    let value: Object
    value.toString()
}
